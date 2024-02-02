#include "chatserver.hpp"//网络模块
#include "chatservice.hpp"//业务模块
#include<signal.h>
#include<iostream>
#include"public.hpp"

using namespace std;

//服务器异常退出处理
void resetHandler(int)
{
    ChatService::getInstance()->reset();
    LOG("服务器异常退出！");
    exit(0);
}

int main(int argc,char **argv)
{
    if(argc<3)
    {
        cerr<<"command invalid! example: ./ChatServer 127.0.0.1 6000"<<endl;
        exit(-1);
    }

    //通过解析命令行传递ip和port
    char *ip=argv[1];
    // unsigned short port=atoi(argv[2]);
    uint16_t port=atoi(argv[2]);

    signal(SIGINT,resetHandler);//服务器ctrl+c信号将触发数据重置

    EventLoop loop;
    // InetAddress addr("127.0.0.1",6000);
    InetAddress addr(ip,port);
    ChatServer server(&loop,addr,"chatserver");
    server.start();
    loop.loop();
}