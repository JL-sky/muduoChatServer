#ifndef DB_H
#define DB_H
#include<mysql/mysql.h>
#include<string>
#include<ctime>

using namespace std;

class Connection
{
public:
    //初始化mysql连接
    Connection();
    //释放链接
    virtual ~Connection();

    //连接数据库
    bool connect(string ip,unsigned short port,
                string user,string passwd,string dbname);

    //对数据库做操作 update、delete、insert
    bool update(string sql);

    //查询操作  select
    MYSQL_RES*  query(string sql);

    
    //刷新链接的起始空闲时间点
    void refreshAliveTime(){_aliveTime=clock();}
    //获取连接的空闲时间
    clock_t getAliveTime(){return clock()-_aliveTime;}

    //获取数据库连接
    MYSQL* getConnection();
private:
    MYSQL* _conn;
    clock_t _aliveTime;//记录每个连接空闲状态的初始时间点
};

#endif