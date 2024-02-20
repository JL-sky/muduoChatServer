#ifndef USER_H
#define USER_H
#include<iostream>
#include<string>
using namespace std;

//user表ORM(对象关系映射)类，将数据库中的数据提取出来封住成一个类对象
/*
将对象模型与关系数据库之间进行映射，
帮助开发人员在应用程序中操作数据库而不需要编写原始的 SQL 语句
*/

/*
mysql> desc user;
+----------+--------------------------+------+-----+---------+----------------+
| Field    | Type                     | Null | Key | Default | Extra          |
+----------+--------------------------+------+-----+---------+----------------+
| id       | int(11)                  | NO   | PRI | NULL    | auto_increment |
| name     | varchar(50)              | YES  | UNI | NULL    |                |
| password | varchar(50)              | YES  |     | NULL    |                |
| state    | enum('online','offline') | YES  |     | offline |                |
+----------+--------------------------+------+-----+---------+----------------+

*/
class User
{
public:
    User(int id=-1,string name="",string passwd="",string state="offline")
    :id(id),name(name),password(passwd),state(state)
    {
    }

    void setId(int id){this->id=id;}
    void setName(string name){this->name=name;}
    void setPasswd(string passwd){this->password=passwd;}
    void setState(string state){this->state=state;}

    int getId(){return this->id;}
    string getName(){return this->name;}
    string getPasswd(){return this->password;}
    string getState(){return this->state;}

private:
    int id;
    string name;
    string password;
    string state;    
};

#endif