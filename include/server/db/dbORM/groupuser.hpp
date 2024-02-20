#ifndef GROUPUSER_H
#define GROUPUSER_H
#include "user.hpp"

//群组用户
/*数据库中groupuser表的对象关系映射*/
/*
mysql> desc groupuser;
+-----------+--------------------------+------+-----+---------+-------+
| Field     | Type                     | Null | Key | Default | Extra |
+-----------+--------------------------+------+-----+---------+-------+
| groupid   | int(11)                  | NO   | MUL | NULL    |       |
| userid    | int(11)                  | NO   |     | NULL    |       |
| grouprole | enum('creator','normal') | YES  |     | NULL    |       |
+-----------+--------------------------+------+-----+---------+-------+
*/
class GroupUser:public User//群组用户也有id，name和state，直接继承user即可
{
public:
    void setRole(string role){this->role=role;}
    string getRole(){return this->role;}
private:
    string role;//群组中该用户的角色信息
};

#endif