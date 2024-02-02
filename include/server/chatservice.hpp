#ifndef CHATSERVICE_H
#define CHATSERVICE_H
#include<muduo/net/TcpConnection.h>
#include<unordered_map>
#include<functional>
#include<mutex>

#include "usermodel.hpp"
#include "offlinemessagemodel.hpp"
#include "friendmodel.hpp"
#include "groupmodel.hpp"
#include "redis.hpp"

using namespace std;
using namespace muduo;
using namespace muduo::net;

#include "json.hpp"
using json=nlohmann::json;

using MsgHandler=function<void(const TcpConnectionPtr& conn,json& js,Timestamp time)>;

class ChatService
{
public:
    //获取单例对象的调用接口
    static ChatService* getInstance();

    //处理登录业务
    void login(const TcpConnectionPtr& conn,json& js,Timestamp time);
    //处理注册业务
    void reg(const TcpConnectionPtr& conn,json& js,Timestamp time);
    //处理注销业务
    void loginout(const TcpConnectionPtr& conn,json& js,Timestamp time);
    //一对一聊天业务
    void oneChat(const TcpConnectionPtr& conn,json& js,Timestamp time);
    //添加好友业务
    void addFriend(const TcpConnectionPtr& conn,json& js,Timestamp time);
    //创建群组业务
    void createGroup(const TcpConnectionPtr& conn,json& js,Timestamp time);
    //加入群组业务
    void addGroup(const TcpConnectionPtr& conn,json& js,Timestamp time);
    //群组聊天业务
    void groupChat(const TcpConnectionPtr& conn,json& js,Timestamp time);

    //处理客户端异常退出函数
    void clientCloseException(const TcpConnectionPtr& conn);
    //获取消息id对应的handler
    MsgHandler getHandler(int msgid);
    //服务器异常退出重置函数
    void reset();
private:
    ChatService();
    //存储消息id及其对应的回调函数handler
    unordered_map<int,MsgHandler> _msgHandlerMap;
    
    //存储在线用户的通信连接，用于给对应的用户及时推送消息
    unordered_map<int,TcpConnectionPtr> _userConnMap;

    //保护多线程对_connMap的访问
    mutex _userConnMutex;
    
    /*数据操作类对象*/
    //用户数据操作处理
    UserModel _userModel;
    //离线消息数据操作处理
    OfflineLineMsgModel _offlineMsgModel;
    //好友信息数据操作处理
    FriendModel _friendModel;
    //群组信息数据操作处理
    GroupModel _groupModel;

    //使用redis作消息队列，实现跨服务器通信
    Redis _redis;
    void handlerRedisSubscribeMessage(int channel,string msg);

};

#endif