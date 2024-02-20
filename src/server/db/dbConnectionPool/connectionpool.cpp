#include"connectionpool.hpp"
#include"public.hpp"
#include<fstream>
#include<unistd.h>

ConnectionPool* ConnectionPool::getConnectionPool()
{
    static ConnectionPool  pool;
    return &pool;
}

bool ConnectionPool::loadConfigFile()
{
    char path[1024];
    if (getcwd(path, sizeof(path)) == NULL)
    {
        LOG("Failed to get current working directory!");
        return false;
    }
    //获取项目根目录
    string fullPath=string(path);
    string curPath=fullPath.substr(0,fullPath.find_last_of("/")+1);
    //加载配置文件路径
    FILE* fp=fopen((curPath+"conf/"+"mysql.cnf").c_str(),"r");
    if(!fp)
    {
        LOG("./mysql.cnf file is not exist!");
        return false;
    }

    while(!feof(fp))//feof用于检测释放读取到文件尾
    {
        char line[1024]={0};
        fgets(line,1024,fp);//读取一行数据
        string str=line;
        int idx=str.find("=");
        if(idx==-1)//无效配置项
        {
            continue;
        }
        int endidx=str.find("\n");
        string key=str.substr(0,idx);
        string value=str.substr(idx+1,endidx-idx-1);
        if(key=="ip")
        {
            _ip=value;
        }
        if(key=="port")
        {
            _port=atoi(value.c_str());
        }
        if(key=="username")
        {
            _user=value;
        }
        if(key=="passwd")
        {
            _passwd=value;
        }
        if(key=="dbname")
        {
            _dbname=value;
        }
        if(key=="initSize")
        {
            _initSize=atoi(value.c_str());
        }
        if(key=="maxSize")
        {
            _maxSize=atoi(value.c_str());
        }
        if(key=="maxIdleTime")
        {
            _maxIdleTime=atoi(value.c_str());
        }
        if(key=="maxConnectionTimeout")
        {
            _connectionTimeout=atoi(value.c_str());
        }
    }
    return true;
}


ConnectionPool::ConnectionPool()
{
    if(!loadConfigFile())//加载配置项
    {
        return;
    }
    //初始化连接池的连接
    for(int i=0;i<_initSize;i++)
    {
        Connection* connPtr=new Connection();
        connPtr->connect(_ip,_port,_user,_passwd,_dbname);
        //刷新连接空闲时间初始点
        connPtr->refreshAliveTime();
        _connectionQue.push(connPtr)    ;
        _connectionCnt++;
    }
    //连接生产线程
    thread produce(&ConnectionPool::produceConnectionTask,this);
    produce.detach();

    //监督线程，监督队列中空闲线程，如果队列中存在空闲时间过久的连接，就直接释放
    thread scanner(&ConnectionPool::scannerConnectionTask,this);
    scanner.detach();
}

//链接生产线程
void ConnectionPool::produceConnectionTask()
{
    while(1)
    {
        unique_lock<mutex> lock(_queueMutex);
        while(!_connectionQue.empty())
        {
            //连接队列不空，阻塞生产者线程，通知消费者线程进行消费
            cv.wait(lock);
        }

        //说明此时队列为空，生产连接
        if(_connectionCnt<_maxSize)
        {
            Connection* connPtr=new Connection();
            connPtr->connect(_ip,_port,_user,_passwd,_dbname);
            //刷新连接空闲时间初始点
            connPtr->refreshAliveTime();
            _connectionQue.push(connPtr);
            _connectionCnt++;
        }
        //通知所有消费者线程，队列不为空，可以进行消费
        cv.notify_all();
    }
}

shared_ptr<Connection> ConnectionPool::getConnection()
{
    unique_lock<mutex> lock(_queueMutex);
    while(_connectionQue.empty())
    {
        //当前链接队列为空，阻塞等待消费者线程个_connectionTimeout的时间
        if(cv_status::timeout==cv.wait_for(lock,std::chrono::milliseconds(_connectionTimeout)))
        {
            if(_connectionQue.empty())
            {
                LOG("获取空闲时间超时，连接失败！");
                return nullptr;
            }
        }
    }

    //从连接队列中获取一个连接对象,
    //并自定义删除器，当使用完连接后不释放连接，而是将其刚入连接池中
    shared_ptr<Connection> connPtr(
        _connectionQue.front(),
        [&](Connection* cp)
        {
            //删除器是在服务器线程中调用的，而不是这段代码调用的，
            //因此代码最开始的互斥锁在这里不起作用，需要对临界队列进行互斥使用
            // unique_lock<mutex> lock(_queueMutex);
            lock_guard<mutex> lock(_queueMutex);

            //刷新连接空闲时间初始点
            cp->refreshAliveTime();
            _connectionQue.push(cp);
        }
    );
    _connectionQue.pop();
    //通知消费者线程，如果队列为空就生成线程
    cv.notify_all();
    return connPtr;
}

void ConnectionPool::scannerConnectionTask()
{
    while(1)
    {
        //定时清理，每隔一个连接最大空闲时间单位就扫描一次队列，清理队列多余连接
        this_thread::sleep_for(std::chrono::seconds(_maxIdleTime));
        unique_lock<mutex> lock(_queueMutex);
        //扫描整个连接队列，释放对于的连接
        while(_connectionCnt>_initSize)
        {
            Connection *p=_connectionQue.front();
            if(p->getAliveTime()>=_maxIdleTime*1000)
            {
                _connectionQue.pop();
                _connectionCnt--;
                delete p;
            }
            else{
            //如果队头的连接空闲时间都没有超过设定的最大空闲时间，
            //说明队列其他的连接都没有超过空闲时间
                break;
            }
        }
    }
}


