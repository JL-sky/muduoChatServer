#ifndef PUBLIC_H
#define PUBLIC_H
#include<iostream>
using namespace std;

// 打印源文件的文件名、所造行号和时间戳
#define LOG(str)\
    cout<<__FILE__<<":"<<__LINE__<<" "<<\
    __TIMESTAMP__<<" : "<<str<<endl;

/*
server与client的公共文件
*/
enum EnMsgType
{
    LOGIN_MSG=1,//登录消息
    LOGIN_MSG_ACK,//登录响应消息
    REG_MSG,//注册消息
    REG_MSG_ACK,//注册响应信息
    LOGIN_OUT_MSG,

    ONE_CHAT_MSG,//一对一聊天消息
    ADD_FRIEND_MSG,//添加好友消息

    CREATE_GROUP_MSG,//创建群组消息
    CREATE_GROUP_MSG_ACK,//创建群组响应消息
    ADD_GROUP_MSG,//加入群组消息
    ADD_GROUP_MSG_ACK,//加入群组消息
    GROUP_CHAT_MSG//群聊天
};

#endif