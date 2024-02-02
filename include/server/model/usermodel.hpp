#ifndef USERMODEL_H
#define USERMODEL_H
#include"user.hpp"

//user表的数据操作类，直接对封装好的user类对象进行处理
class UserModel
{
public:
    //user表的数据增加方法
    bool insert(User &user);
    
    //根据数据库id查询消息
    User queryById(int id);

    //修改用户状态
    bool updateState(User& user);

    //用户状态重置
    void resetState();
private:
};

#endif