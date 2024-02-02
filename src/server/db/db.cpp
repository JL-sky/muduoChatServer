#include "db.hpp"
#include"public.hpp"

//初始化mysql连接
MySql::MySql()
{
    _conn=mysql_init(nullptr);
}

//释放链接
MySql::~MySql()
{
    if(_conn!=nullptr)
        mysql_close(_conn);
}

//连接数据库
bool MySql::connection(string ip,unsigned short port,
                        string user,string passwd,
                        string dbname)
{
    MYSQL* p=mysql_real_connect(
        _conn,
        ip.c_str(),
        user.c_str(),
        passwd.c_str(),
        dbname.c_str(),
        port,
        nullptr,0
    );

    if(!p)
    {
        LOG("数据库连接失败！\n");
        LOG("错误信息:"+string(mysql_error(_conn))+"\n");

        return false;
    }

    mysql_query(_conn,"set names gbk");
    LOG("数据库连接成功！\n");
    return true;
}

//对数据库做操作 update、delete、insert
bool MySql::update(string sql)
{
    if(mysql_query(_conn,sql.c_str()))
    {
        LOG("更新失败!\n");
        LOG("错误信息:"+string(mysql_error(_conn))+"\n");
        return false;
    }
    return true;
}

//查询操作  select
MYSQL_RES* MySql::query(string sql)
{
    if(mysql_query(_conn,sql.c_str()))
    {
        LOG("查询失败!\n");
        LOG("错误信息:"+string(mysql_error(_conn))+"\n");
        return nullptr;
    }
    return mysql_use_result(_conn);
}

//获取数据库连接
MYSQL* MySql::getConnection()
{
    return _conn;
}