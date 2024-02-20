#ifndef CONNECTIONPOOL_H
#define CONNECTIONPOOL_H
#include<mutex>
#include<queue>
#include<atomic>
#include<memory>
#include<functional>
#include<thread>
#include<condition_variable>
// #include"connection.hpp"  
#include"db.hpp"  

class ConnectionPool
{
public:
    static ConnectionPool* getConnectionPool();//获取单例连接池对象
    //从连接池中获取一个可用的连接，使用智能指针进行管理，当连接不使用时放回连接池不进行释放
    shared_ptr<Connection> getConnection();

private:
    //创建初始化数目的连接
    ConnectionPool();
    //加载配置项
    bool loadConfigFile();
    //生产者线程，用于生成链接
    void produceConnectionTask();
    //定时监督线程，用于监督队列中的空闲连接
    void scannerConnectionTask();


    //数据库连接配置
    string _ip;
    unsigned short _port;
    string _user;
    string _passwd;
    string _dbname;

    //连接池的初始连接量
    int _initSize;
    //连接池的最大连接量
    int _maxSize;
    //连接池的最大空闲时间
    int _maxIdleTime;
    //连接池获取连接的超时时间
    int _connectionTimeout;

    //存储mysql连接的队列
    queue<Connection*> _connectionQue;
    //维护连接队列的线程安全互斥锁
    mutex _queueMutex;
    //记录所创建连接的总数量
    atomic_int _connectionCnt;
    //设置条件变量，供生产者消费者模型通信使用
    condition_variable cv;
};
#endif