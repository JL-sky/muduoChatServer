#ifndef FRIENDMODEL
#define FRIENDMODEL
#include"user.hpp"
#include<vector>
using namespace std;

class FriendModel
{
public:
    //向数据库中添加好友
    bool insert(int userId,int friendId);
    //查询好友信息并返回
    vector<User> query(int userId);
private:
};

#endif