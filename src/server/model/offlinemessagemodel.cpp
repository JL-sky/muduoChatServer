#include "offlinemessagemodel.hpp"
#include"db.hpp"
//存储用户的离线消息
bool OfflineLineMsgModel::insert(int userId,string msg)
{
    char sql[1024]={0};
    sprintf(sql,"insert into offlinemessage values(%d,'%s')",userId,msg.c_str());
    
    MySql mysql;
    if(mysql.connection())
    {
        return mysql.update(sql);
    }
    return false;
}
//删除用户的离线消息
bool OfflineLineMsgModel::remove(int userId)
{
    char sql[1024]={0};
    sprintf(sql,"delete from offlinemessage where userid=%d",userId);
    MySql mysql;
    if(mysql.connection())
    {
        return mysql.update(sql);
    }
    return false;
}
//查询用户的离线消息
vector<string> OfflineLineMsgModel::query(int userId)
{
    vector<string> offlineMsg;
    char sql[1024]={0};
    sprintf(sql,"select message from offlinemessage where userid=%d",userId);
    MySql mysql;
    if(mysql.connection())
    {
        MYSQL_RES* res=mysql.query(sql);
        MYSQL_ROW row;
        while((row=mysql_fetch_row(res))!=nullptr)
        {
            offlineMsg.push_back(row[0]);
            // offlineMsg.push_back(row[1]);
        }
        mysql_free_result(res);
    }
    return offlineMsg;
}