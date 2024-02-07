#include"friendmodel.hpp"
#include"db.hpp"
#include"usermessage.hpp"
// #include"muduo/base/Logging.h"
#include "public.hpp"

//向数据库中添加好友
bool FriendModel::insert(int userId,int friendId)
{
    char sql[1024]={0};
    sprintf(sql,
            "insert into friend values(%d,%d),(%d,%d)",
            userId,friendId,friendId,userId);
    MySql mysql;
    if(mysql.connection())
    {
        if(mysql.update(sql))
        {
            LOG("好友添加成功！");
            return true;
        }
    }
    return false;
}
//查询好友信息并返回
/*
从friend表（只有userid和friendid字段）中查好友的详细信息，
其中用户的详细信息存储在user表中
*/
vector<User> FriendModel::query(int userId)
{
    char sql[1024]={0};
    sprintf(sql,
            "select a.id,a.name,a.state from user a inner join friend b on b.friendid=a.id where b.userid=%d",
            userId);
    vector<User> friendMsgVec;
    MySql mysql;
    if(mysql.connection())
    {
        User friendMsg;
        MYSQL_RES* res=mysql.query(sql);
        MYSQL_ROW row;
        while((row=mysql_fetch_row(res)))
        {
            friendMsg.setId(atoi(row[0]));
            friendMsg.setName(row[1]);
            friendMsg.setState(row[2]);
            friendMsgVec.push_back(friendMsg);
        }
        mysql_free_result(res);//释放mysql链接
    }

    return friendMsgVec;
}