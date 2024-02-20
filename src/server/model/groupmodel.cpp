#include"groupmodel.hpp"
#include"connectionpool.hpp"
#include"public.hpp"
#include<vector>
using namespace std;

//创建群组
bool GroupModel::createGroup(Group& group)
{
    char sql[1024]={0};
    sprintf(sql,
            "insert into allgroup(groupname,groupdesc) values('%s','%s')",
            group.getGroupName().c_str(),group.getGroupDesc().c_str());
    // MySql mysql;
    ConnectionPool* cp=ConnectionPool::getConnectionPool();
    shared_ptr<Connection> sp=cp->getConnection();
    if(sp)
    {
        if(sp->update(sql))
        {
            //获取插入成功的生成的主键id
            int gid=mysql_insert_id(sp->getConnection());
            group.setId(gid);
            LOG("群组 "+group.getGroupName()+" 创建成功！gid="+to_string(gid));
            return true;
        }
    }
    LOG("群组 "+group.getGroupName()+" 创建失败！");
    return false;
}
//加入群组
bool GroupModel::addGroup(int userId,int groupId,string role)
{
    char sql[1024]={0};
    sprintf(sql,
            "insert into groupuser values(%d,%d,'%s')",
            groupId,userId,role.c_str());
    // MySql mysql;
    ConnectionPool* cp=ConnectionPool::getConnectionPool();
    shared_ptr<Connection> sp=cp->getConnection();
    if(sp)
    {
        if(sp->update(sql))
        {
            LOG("用户 "+to_string(userId)+" 加入群组 "+to_string(groupId)+" 成功！");
            return true;
        }
    }
    LOG("用户 "+to_string(userId)+" 加入群组 "+to_string(groupId)+" 失败！");
    return false;
}

//根据用户id查询用户所在群组信息,包括该群组中其他用户的信息
/*
1.先根据userid联合groupuser和allgroup表查询用户所在群组信息
2.再根据查询到的群组信息，查询用户所在群组的其他用户成员信息
*/
vector<Group> GroupModel::queryGroups(int userId)
{
    //查询该用户所在群组信息(id,groupname,groupdesc)
    char sql[1024]={0};
    sprintf(sql,
            "select a.id,a.groupname,a.groupdesc from allgroup a \
            inner join groupuser b \
            on b.groupid=a.id \
            where b.userid=%d",
            userId);
    vector<Group> groupMsgVec;
    // MySql mysql;
    ConnectionPool* cp=ConnectionPool::getConnectionPool();
    shared_ptr<Connection> sp=cp->getConnection();
    if(sp)
    {
        MYSQL_RES* res=sp->query(sql);
        if(res)
        {
            MYSQL_ROW row;
            //该用户可能不只在一个组群
            while((row=mysql_fetch_row(res)))
            {
                Group group;
                group.setId(atoi(row[0]));
                group.setGroupName(row[1]);
                group.setGroupDesc(row[2]);
                groupMsgVec.push_back(group);
            }
            mysql_free_result(res);
        }
    }


    //遍历该用户所在的所有群组，并查询其所在群组中其余用户信息
    /*
    此处的group必须为引用，
    只有使用引用才能将查询到的群组成员信息存到上述已经取到群组信息的group对象里
    */
    for(auto& group:groupMsgVec)
    {
        char sql[1024]={0};
        /*
        1.首先过滤groupid（where b.groupid=%d）
        2.联合groupuser表的userid字段和user表的id字段查询user表中对应用户信息
        */
        sprintf(sql,
                "select a.id,a.name,a.state,b.grouprole from user a \
                inner join groupuser b \
                on a.id=b.userid \
                where b.groupid=%d",
                group.getId()
                );
        MYSQL_RES* res=sp->query(sql);
        if(res)
        {
            MYSQL_ROW row;
            while((row=mysql_fetch_row(res)))
            {
                GroupUser guser;
                guser.setId(atoi(row[0]));
                guser.setName(row[1]);
                guser.setState(row[2]);
                guser.setRole(row[3]);
                group.getGroupUsers().push_back(guser);
            }
            mysql_free_result(res);
        }
    }
    
    return groupMsgVec;
}

//根据指定的groupid查询群组用户id列表，用于群组聊天业务，给群组中其他成员发消息
vector<int> GroupModel::queryGroupUsers(int userId,int groupId)
{
    char sql[1024]={0};
    sprintf(sql,
            "select userid from groupuser where userid!=%d and groupid=%d",
            userId,groupId);
    // MySql mysql;
    vector<int> userIdVec;
    ConnectionPool* cp=ConnectionPool::getConnectionPool();
    shared_ptr<Connection> sp=cp->getConnection();
    if(sp)
    {
        MYSQL_RES * res=sp->query(sql);
        if(res)
        {
            MYSQL_ROW row;
            while((row=mysql_fetch_row(res)))
            {
                userIdVec.push_back(atoi(row[0]));
            }
        }
        mysql_free_result(res);
    }
    return userIdVec;
}