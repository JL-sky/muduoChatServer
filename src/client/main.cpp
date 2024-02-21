#include<iostream>
#include<cstring>
#include<chrono>
using namespace std;
#include<unistd.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<arpa/inet.h>
#include<netinet/in.h>
#include "clientservice.hpp"
#include "json.hpp"
using json=nlohmann::json;
#include "public.hpp"

int main(int argc,char **argv)
{
    if(argc<3)
    {   
        cout<<"command incalid! example: ./ChatClient 127.0.0.1 6000"<<endl;
    }
    //命令行解析，获取服务器的ip和端口号
    char *ip=argv[1];
    unsigned short port=atoi(argv[2]);
    
    //创建client socket
    int clientfd=socket(AF_INET,SOCK_STREAM,0);
    if(clientfd==-1)
    {
        cerr<<"client socket create error!"<<endl;
        exit(-1);
    }
    //创建socket地址
    sockaddr_in caddr;
    memset(&caddr,0,sizeof(sockaddr_in));
    caddr.sin_family=AF_INET;
    caddr.sin_port=htons(port);
    caddr.sin_addr.s_addr=inet_addr(ip);
    //连接服务器
    int ret=connect(clientfd,(sockaddr*)&caddr,sizeof(caddr));
    if(ret==-1)
    {
        cerr<<"client connect error"<<endl;
        exit(-1);
    }
    
    ClientService* cliSer=ClientService::getInstance(clientfd);
    while(1)
    {
        //显示页面菜单
        cliSer->menu();
        cout<<"choice:";
        int choice=0;
        cin>>choice;
        cin.get();//读掉cin缓冲区里残留的回车，防止后期判断失误

        switch(choice)
        {
            case 1://账号登录
            {
                cliSer->clientLogin(clientfd);
                break;
            }

            case 2://注册账号
            {
                cliSer->clientRegister(clientfd);
                // clientRegister(clientfd);
                break;
            }
            case 3:
                close(clientfd);
                exit(0);
            default:
                cerr<<"invalid input"<<endl;
                break;
        }
    }
    return 0;
}