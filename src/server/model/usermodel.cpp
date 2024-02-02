#include "usermodel.hpp"
#include "db.hpp"

bool UserModel::insert(User &user)
{
    // 组装sql语句
    char sql[1024]={0};
    sprintf(sql,
    "insert into user(name,password,state) values ('%s','%s','%s')",
    user.getName().c_str(),user.getPasswd().c_str(),user.getState().c_str());

    //声明自定义的mysql类
    MySql mysql;
    //bool MySql::connection(string ip,unsigned short port,
    //            string user,string passwd,string dbname)
    if(mysql.connection())
    {
        if(mysql.update(sql))
        {
            //获取插入成功的用户数据成功生成的主键id
            user.setId(mysql_insert_id(mysql.getConnection()));
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
    MySql mysql;
    if(mysql.connection())
    {
        //查询数据库
        MYSQL_RES* res=mysql.query(sql);
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

    MySql mysql;
    if(mysql.connection())
    {
        return mysql.update(sql);
    }
    return false;
}

void UserModel::resetState()
{
    char sql[1024]="update user set state='offline' where state='online'";
    MySql mysql;
    if(mysql.connection())
    {
        mysql.update(sql);
    }
}