#ifndef DB_H
#define DB_H
#include<mysql/mysql.h>
#include<string>
#include "usermessage.hpp"

using namespace std;

class MySql
{
public:
    //初始化mysql连接
    MySql();
    //释放链接
    virtual ~MySql();

    //连接数据库
    bool connection(string ip=ip,unsigned short port=port,
                string user=userName,string passwd=passwd,string dbname=dbname);

    //对数据库做操作 update、delete、insert
    bool update(string sql);

    //查询操作  select
    MYSQL_RES*  query(string sql);

    //获取数据库连接
    MYSQL* getConnection();
private:
    MYSQL* _conn;
};

#endif