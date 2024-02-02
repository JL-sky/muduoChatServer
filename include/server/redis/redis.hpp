#ifndef REDIS_H
#define REDIS_H
#include<hiredis/hiredis.h>
#include<functional>
#include<thread>
#include<string>
using namespace std;

class Redis
{
public:
    Redis();
    ~Redis();
    //连接redis服务器
    bool connect();
    //向指定的通道发布消息
    bool publish(int channel,string message);
    //向指定的通道订阅消息
    bool subscribe(int channel);
    //向指定的通道取消订阅消息
    bool unsubscribe(int channel);
    //向独立的线程中接收订阅通道中的消息
    void observe_channel_message();
    //初始化向业务层上报通道消息的回调对象
    void init_notify_handler(function<void(int,string)> fn);
private: 
    //reids同步上下文对象，负责subscribe对象
    //在redis上订阅消息后，上下文会被阻塞，以等待消息的到来
    //因此需要有另外一个publish上下文发布消息
    redisContext *_subscribe_context;

    //redis同步上下文对象，负责publish对象
    redisContext *_publish_context;

    //回调操作，收到订阅的消息，给server层上报
    function<void(int,string)> _notify_message_handler;
};
#endif