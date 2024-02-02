#ifndef OFFLINEMESSAGEMODEL_H
#define OFFLINEMESSAGEMODEL_H
#include<vector>
#include<string>
using namespace std;

class OfflineLineMsgModel
{
public:
    //存储用户的离线消息
    bool insert(int userId,string msg);
    //删除用户的离线消息
    bool remove(int userId);
    //查询用户的离线消息
    vector<string> query(int userId);
};

#endif