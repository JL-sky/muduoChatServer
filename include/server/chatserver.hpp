#ifndef CHATSERVER_H
#define CHATSERVER_H

#include<muduo/net/TcpServer.h>
#include<muduo/net/EventLoop.h>

using namespace muduo;
using namespace muduo::net;

class ChatServer
{
public:
    //构造函数，初始化聊天服务器对象
    ChatServer(
        EventLoop* loop,
        const InetAddress& listenAddr,
        const string& nameArg
    );
    //启动服务
    void start();
private:
    TcpServer _server;
    EventLoop *_loop;

    //处理网络连接建立和断开的回调函数
    void onConnectionCallback(const TcpConnectionPtr&);
    //处理连接读写事件的回调函数
    void onMessageCallback(const TcpConnectionPtr&,
                            Buffer*,
                            Timestamp);

};

#endif