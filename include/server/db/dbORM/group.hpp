#ifndef GROUP_H
#define GROUP_H
#include"groupuser.hpp"
#include<vector>
#include<string>
using namespace std;

/*数据库中allgroup表的对象关系映射*/
/*
mysql> desc allgroup;
+-----------+--------------+------+-----+---------+----------------+
| Field     | Type         | Null | Key | Default | Extra          |
+-----------+--------------+------+-----+---------+----------------+
| id        | int(11)      | NO   | PRI | NULL    | auto_increment |
| groupname | varchar(50)  | NO   | UNI | NULL    |                |
| groupdesc | varchar(200) | YES  |     |         |                |
+-----------+--------------+------+-----+---------+----------------+
*/
class Group
{
public:
    Group(int groupid=-1,string groupname="",string groupdesc="",vector<GroupUser> groupusers={})
    :groupid(groupid),groupname(groupname),groupdesc(groupdesc),groupusers(groupusers)
    {}
    void setId(const int groupid){this->groupid=groupid;}
    void setGroupName(const string groupname){this->groupname=groupname;}
    void setGroupDesc(const string groupdesc){this->groupdesc=groupdesc;}

    int getId(){return this->groupid;}
    string getGroupName(){return this->groupname;}
    string getGroupDesc(){return this->groupdesc;}
    vector<GroupUser>& getGroupUsers(){return this->groupusers;}

private:
    int groupid;//群组id
    string groupname;//群组名称
    string groupdesc;//群组描述
    vector<GroupUser> groupusers;//返回该群组中的成员信息
};

#endif