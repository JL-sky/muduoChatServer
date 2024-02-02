#include "chatserver.hpp"
#include "json.hpp"
#include "chatservice.hpp"
#include<functional>

using namespace std;
using namespace placeholders;
using json=nlohmann::json;

ChatServer::ChatServer(EventLoop* loop,
        const InetAddress& listenAddr,
        const string& nameArg)
        :_server(loop,listenAddr,nameArg),_loop(loop)
{
    //发生链接建立和断开事件时对应的回调
    _server.setConnectionCallback(std::bind(&ChatServer::onConnectionCallback,this,_1));
    //发生消息读写事件时对应的回调
    _server.setMessageCallback(std::bind(&ChatServer::onMessageCallback,this,_1,_2,_3));

    //设置线程数量
    _server.setThreadNum(4);
}

//启动函数
void ChatServer::start()
{
    _server.start();
}

//处理链接建立和断开时的回调函数
void ChatServer::onConnectionCallback(const TcpConnectionPtr& conn)
{
    if(!conn->connected())
    {
        //客户端连接断开处理
        ChatService::getInstance()->clientCloseException(conn);
        conn->shutdown();
    }
}

//处理读写事件时的回调函数
void ChatServer::onMessageCallback(const TcpConnectionPtr& conn,
                            Buffer* buffer,
                            Timestamp time)
{
    string buf=buffer->retrieveAllAsString();
    //json数据的反序列化
    json js=json::parse(buf);
    /*
        获取收到的消息id对应的事件处理器
        get<int>()将json数据转换为对应的int类型
    */
    auto msgHandler=ChatService::getInstance()->getHandler(js["msgid"].get<int>());
    //处理消息id对应的消息事件
    msgHandler(conn,js,time);
}

