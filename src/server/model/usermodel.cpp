#include "usermodel.hpp"
#include "connectionpool.hpp"
#include<memory>

bool UserModel::insert(User &user)
{
    // 组装sql语句
    char sql[1024]={0};
    sprintf(sql,
    "insert into user(name,password,state) values ('%s','%s','%s')",
    user.getName().c_str(),user.getPasswd().c_str(),user.getState().c_str());

    //声明自定义的mysql类
    // MySql mysql;
    ConnectionPool* cp=ConnectionPool::getConnectionPool();
    shared_ptr<Connection> sp=cp->getConnection();
    if(sp)
    {
        if(sp->update(sql))
        {
            //获取插入成功的用户数据成功生成的主键id
            user.setId(mysql_insert_id(sp->getConnection()));
            return true;
        }
    }
    return false;
}

//根据数据库id查询消息
User UserModel::queryById(int id)
{
    //组装查询sql
    char sql[1024]={0};
    sprintf(sql,
            "select * from user where id=%d",
            id);

    //连接sql
    // MySql mysql;
    ConnectionPool* cp=ConnectionPool::getConnectionPool();
    shared_ptr<Connection> sp=cp->getConnection();
    if(sp)
    {
        //查询数据库
        MYSQL_RES* res=sp->query(sql);
        if(res)
        {
            User user;
            // 取出一行数据
            MYSQL_ROW row=mysql_fetch_row(res);
            if(row)
            {
                //根据数据库数据组成user对象
                user.setId(atoi(row[0]));
                user.setName(row[1]);
                user.setPasswd(row[2]);
                user.setState(row[3]);
                mysql_free_result(res);
            }
            return user;
        }
    }
}

bool UserModel::updateState(User& user)
{
    char sql[1024]={0};
    sprintf(sql,
            "update user set state='%s' where id=%d",
            user.getState().c_str(),user.getId());

    // MySql mysql;
    ConnectionPool* cp=ConnectionPool::getConnectionPool();
    shared_ptr<Connection> sp=cp->getConnection();
    if(sp)
    {
        return sp->update(sql);
    }
    return false;
}

void UserModel::resetState()
{
    char sql[1024]="update user set state='offline' where state='online'";
    // MySql mysql;
    ConnectionPool* cp=ConnectionPool::getConnectionPool();
    shared_ptr<Connection> sp=cp->getConnection();
    if(sp)
    {
        sp->update(sql);
    }
}