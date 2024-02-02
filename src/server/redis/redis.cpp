#include "redis.hpp"
#include<iostream>

Redis::Redis():_publish_context(nullptr),_subscribe_context(nullptr){}
Redis::~Redis()
{
    if(_publish_context)
    {
        redisFree(_publish_context);
    }

    if(_subscribe_context)
    {
        redisFree(_subscribe_context);
    }
}

//连接redis服务器
bool Redis::connect()
{
    _subscribe_context=redisConnect("127.0.0.1",6379);
    if(_subscribe_context==nullptr)
    {
        cerr<<"redis connect faile"<<endl;
        return false;
    }

    _publish_context=redisConnect("127.0.0.1",6379);
    if(_publish_context==nullptr)
    {
        cerr<<"redis connect faile"<<endl;
        return false;
    }

    //在单独的线程中，监听通道上的事件，如果有消息就上报给服务器
    //重新开启一个新线程的原因是为了防止当前订阅通道的线程被阻塞
    thread t(
        [&]()
        {
            observe_channel_message();
        }
    );
    cout<<"connect redis-server success!"<<endl;
    return true;
}
//向指定的通道发布消息
bool Redis::publish(int channel,string message)
{
    redisReply *reply=(redisReply*)redisCommand(_publish_context,"PUBLISH %d %s",channel,message.c_str());
    if(!reply)
    {
        cerr<<"publish command failed!"<<endl;
        return false;
    }
    freeReplyObject(reply);
    return true;    
}
//向指定的通道订阅消息
bool Redis::subscribe(int channel)
{
    /*
    hiredis的redisCommand实际上相当于调用了这三个函数：
        1.redisAppendCommand 把命令写入本地发送缓冲区
        2.redisBufferWrite 把本地缓冲区的命令通过网络发送给redis-server
        3.redisGetReply 阻塞等待redis server响应消息
    */

   //muduo库的ThreadPool中单独开辟了一个线程池，接收this->_context上下文的响应消息，
   //因此subcribe订阅消息只做消息发送，不做消息接收就可以了
    if(REDIS_ERR==redisAppendCommand(this->_subscribe_context,"SUBSCRIBE %d",channel))
    {
        cerr<<"subscribe command failed!"<<endl;
        return false;
    }

    int done=0;
    while(!done)
    {
        if(REDIS_ERR==redisBufferWrite(_subscribe_context,&done))
        {
            cerr<<"subscribe command failed!"<<endl;
            return false;
        }
    }
    return true;
}
//向指定的通道取消订阅消息
bool Redis::unsubscribe(int channel)
{
    if(REDIS_ERR==redisAppendCommand(_subscribe_context,"UNSUBSCRIBE %d",channel))
    {
        cerr<<"unsubscribe error!"<<endl;
        return false;
    }
     int done=0;
     while(!done)
     {
        if(REDIS_ERR==redisBufferWrite(_subscribe_context,&done))
        {
            cerr<<"unsubscribe error!"<<endl;
            return false;
        }
     }
     return true;
}
//向独立的线程中接收订阅通道中的消息
void Redis::observe_channel_message()
{
    redisReply *reply=nullptr;
    while(REDIS_OK==redisGetReply(_subscribe_context,(void**)&reply))
    {
        //订阅收到的消息是一个带三个元素的数组
        /*
        reply->element[1]表示通道号
        reply->element[2]表示消息
        */
        if(!reply && !reply->element[2] && !reply->element[2]->str)
        {
            //给业务层上报通道上的消息
            _notify_message_handler(atoi(reply->element[1]->str),reply->element[2]->str);
        }
        freeReplyObject(reply);
    }
    cerr<<"---------------------observer_channel_message quit---------------------"<<endl;

}
//初始化向业务层上报通道消息的回调对象
void Redis::init_notify_handler(function<void(int,string)> fn)
{
    this->_notify_message_handler=fn;
}