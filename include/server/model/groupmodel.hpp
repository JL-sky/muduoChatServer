#ifndef GROUPMODEL_H
#define GROUPMODEL_H

#include"group.hpp"
#include"groupuser.hpp"

class GroupModel
{
public:
    //创建群组
    bool createGroup(Group& group);
    //加入群组
    bool addGroup(int userId,int groupId,string role);
    //根据用户id查询用户所在群组信息
    vector<Group> queryGroups(int userId);
    //根据指定的groupid查询群组用户id列表，用于群组聊天业务，给群组中其他成员发消息
    vector<int> queryGroupUsers(int userId,int groupId);

private:
};

#endif