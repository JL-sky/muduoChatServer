#include<iostream>
#include<arpa/inet.h>
#include<thread>
#include<unistd.h>
using namespace std;

#include "json.hpp"
using json=nlohmann::json;
#include"public.hpp"
#include "clientservice.hpp"
using namespace placeholders; 

int ClientService::s_readThreadNumber=0;

//获取客户端单实例
ClientService* ClientService::getInstance()
{
    static ClientService cliService;
    return &cliService;
}

ClientService::ClientService()
{
    _commandMap={
        {"help","显示所有支持的命令\texample:  help"},
        {"chat","一对一聊天\texample:  chat:friendid:message"},
        {"addfriend","添加好友\texample:  addfriend:friendid"},
        {"creategroup","创建群组\texample:  creategroup:groupname:groupdesc"},
        {"addgroup","添加群组\texample:  addgroup:groupid"},
        {"groupchat","群聊\texample:  groupchat:groupid:message"},
        {"loginout","注销\texample:  loginout"}
    };
    _commandHandlerMap={
        {"help",std::bind(&ClientService::help,this,_1,_2)},
        {"chat",std::bind(&ClientService::chat,this,_1,_2)},
        {"addfriend",std::bind(&ClientService::addfriend,this,_1,_2)},
        {"creategroup",std::bind(&ClientService::creategroup,this,_1,_2)},
        {"addgroup",std::bind(&ClientService::addgroup,this,_1,_2)},
        {"groupchat",std::bind(&ClientService::groupchat,this,_1,_2)},
        {"loginout",std::bind(&ClientService::loginout,this,_1,_2)}
    };
}

//页面菜单
void ClientService::menu()
{
    cout<<"==================="<<endl;
    cout<<"1.login"<<endl;
    cout<<"2.register"<<endl;
    cout<<"3.quit"<<endl;
    cout<<"==================="<<endl;
}

//客户端登录
void ClientService::clientLogin(const int& clientfd)
{
    int id;
    string passwd;
    cout<<"id:";
    cin>>id;
    cin.get();
    cout<<"passwd:";
    getline(cin,passwd);

    json js;
    js["msgid"]=LOGIN_MSG;
    js["id"]=id;
    js["password"]=passwd;
    string request=js.dump();
    int len=send(clientfd,request.c_str(),strlen(request.c_str())+1,0);
    if(len==-1)
    {
        cerr<<"login data send error"<<endl;
    }else{
        char buf[1024]={0};
        int len = recv(clientfd,buf,1024,0);
        if(len==-1)
        {
            cerr<<"login response read error"<<endl;
        }else{
            json response=json::parse(buf);
            if(response["errno"].get<int>()!=0)
            {
                cerr<<response["errmsg"]<<endl;
            }else{
                //登录成功，将控制聊天程序变量设置为true
                _isMainChatRunning=true;

                //记录当前用户的id和name
                g_curUser.setId(id);
                g_curUser.setName(response["name"]);
                g_curUser.setState(response["state"]);

                //记录用户好友列表信息
                if(response.contains("friends"))
                {
                    //初始化
                    g_curFriendList.clear();
                    vector<string> vec=response["friends"];
                    for(auto& friendStr:vec)//取出每一个好友的信息
                    {
                        json friendMsg=json::parse(friendStr);
                        User user;
                        user.setId(friendMsg["id"].get<int>());
                        user.setName(friendMsg["name"]);
                        user.setState(friendMsg["state"]);
                        g_curFriendList.push_back(user);
                    }
                }

                //记录用户的群组列表信息
                if(response.contains("groups"))
                {
                    //初始化
                    g_curUserGroupList.clear();

                    vector<string> vec=response["groups"];
                    for(string& groupStr:vec)
                    {
                        json group=json::parse(groupStr);
                        Group groupMsg;
                        groupMsg.setId(group["gid"]);
                        groupMsg.setGroupName(group["gname"]);
                        groupMsg.setGroupDesc(group["gdesc"]);
                        
                        vector<string> vec2=group["gusers"];
                        for(string& guserStr:vec2)
                        {
                            json guser=json::parse(guserStr);
                            GroupUser guserMsg;
                            guserMsg.setId(guser["uid"].get<int>());
                            guserMsg.setName(guser["uname"]);
                            guserMsg.setState(guser["state"]);
                            guserMsg.setRole(guser["role"]);
                            groupMsg.getGroupUsers().push_back(guserMsg);
                        }
                        g_curUserGroupList.push_back(groupMsg);
                    }
                }

                cout<<" login success! welcome ["<<g_curUser.getName()
                    <<"],your id is ["<<id<<"]"<<endl;
                //显示登录用户的基本信息
                showCurUserData();

                //显示当前登录用户的离线消息
                if(response.contains("offlineMsg"))
                {
                    vector<string> vec=response["offlineMsg"];
                    for(string offline:vec)
                    {
                        json offlineMsg=json::parse(offline);
                        int msgtype=offlineMsg["msgid"].get<int>();
                        //显示个人离线消息
                        if(msgtype==ONE_CHAT_MSG)
                        {
                            cout<<"offline message\n\t"
                                <<" time:"<<offlineMsg["time"].get<string>()
                                <<" id:"<<offlineMsg["id"]
                                <<" name:"<<offlineMsg["name"].get<string>()
                                <<" messgae"<<offlineMsg["msg"].get<string>()
                                <<endl;
                        }
                        //显示群组离线消息
                        if(msgtype==GROUP_CHAT_MSG)
                        {
                            cout<<"群消息["<<offlineMsg["groupid"]
                                <<"]\ttime:"<<offlineMsg["time"].get<string>()
                                <<"  id:"<<offlineMsg["id"]
                                <<"  name:"<<offlineMsg["name"].get<string>()
                                <<"  message:"<<offlineMsg["msg"].get<string>()<<endl;
                        }
                    }
                    cout<<"------------------------------------------"<<endl;
                }

                cout<<endl;
                //启动接收线程，负责接受数据
                // std::thread readTask(readTaskHandler,clientfd,g_curUser);
                if(s_readThreadNumber==0)
                {
                    //该线程只能启动一次,防止用户退出登录再次登录时开启多个接受线程浪费资源
                    std::thread readTask([&]() {
                    ClientService::readTaskHandler(clientfd, g_curUser);
                    });
                    readTask.detach();
                    s_readThreadNumber++;
                }

                // std::thread readTask([&]() {
                // ClientService::readTaskHandler(clientfd, g_curUser);
                // });
                // readTask.detach();

                //进入聊天页面
                mainchat(clientfd);
            }
        }
    }
}

//客户端注册
void ClientService::clientRegister(const int& clientfd)
{
    string name,passwd;
    cout<<"name:";
    getline(cin,name);
    cout<<"passwd:";
    getline(cin,passwd);

    json js;
    js["msgid"]=REG_MSG;
    js["name"]=name;
    js["password"]=passwd;
    string request=js.dump();

    int len=send(clientfd,request.c_str(),strlen(request.c_str())+1,0);
    if(len==-1)
        cerr<<"send register data error!"<<endl;
    else{
        char buf[1024]={0};
        int len=recv(clientfd,buf,1024,0);
        if(len==-1)
            cerr<<"register response read error!"<<endl;                        
        else{
            json response=json::parse(buf);
            if(response["errno"].get<int>()!=0)
            {
                cerr<<response["errmsg"]<<endl;
            }
            else{
                cout<<name<<" register success! id is "<<response["id"]
                <<",please remeber your id!"<<endl;
            }
        }   
    }
}

//显示当前登录成功用户的基本信息
void ClientService::showCurUserData()
{
    cout<<"=============== login user Data ==============="<<endl;
    cout<<"\tid:"<<g_curUser.getId()
        <<"  name:"<<g_curUser.getName()
        <<"  state:"<<g_curUser.getState()<<endl;
        
    cout<<"----------------- Friend List -----------------"<<endl;
    if(!g_curFriendList.empty())
    {
        for(User& user:g_curFriendList)
        {
            cout<<"\tuid:"<<user.getId()
                <<"  uname:"
                <<user.getName()
                <<"  state:"<<user.getState()<<endl;
        }
    }
    
    cout<<"----------------- Group List -----------------"<<endl;
    if(!g_curUserGroupList.empty())
    {
        for(Group& group:g_curUserGroupList)
        {
            cout<<"\tgid:"<<group.getId()
                <<"  gname:"<<group.getGroupName()
                <<"  gdesc:"<<group.getGroupDesc()<<endl;
            
            for(GroupUser& guser:group.getGroupUsers())
            {
                cout<<"\t\tuid:"<<guser.getId()
                    <<"  uname:"<<guser.getName()
                    <<"  state:"<<guser.getState()
                    <<"  role:"<<guser.getRole()<<endl;
            }
        }
    }
    cout<<"------------------------------------------"<<endl;
}

//接收消息线程
void ClientService::readTaskHandler(const int& clientfd,User& g_curUser)
{
    while(1)
    {
        char buf[1024]={0};
        int len=recv(clientfd,buf,1024,0);
        if(len<=0)
        {
            close(clientfd);
            exit(-1);
        }

        json response=json::parse(buf);
        int msgtype=response["msgid"].get<int>();
        switch(msgtype)
        {
            //一对一聊天响应消息
            case ONE_CHAT_MSG:
            {
                cout<<"\ttime:"<<response["time"].get<string>()
                <<"  id:"<<response["id"]
                <<"  name:"<<response["name"].get<string>()
                <<"  message:"<<response["msg"].get<string>()<<endl;
                break;
            }
            //群聊响应消息
            case GROUP_CHAT_MSG:
            {
                cout<<"群消息["<<response["groupid"]
                <<"]\ttime:"<<response["time"].get<string>()
                <<"  id:"<<response["id"]
                <<"  name:"<<response["name"].get<string>()
                <<"  message:"<<response["msg"].get<string>()<<endl;
                break;
            }
            //创建群组响应消息
            case CREATE_GROUP_MSG_ACK:
            {
                if(response["errno"].get<int>()!=0)
                {
                    cerr<<response["errmsg"].get<string>()<<endl;
                }else{
                    cout<<"群组创建成功！"
                    <<"  gid="<<response["groupid"]
                    <<"  gname="<<response["groupname"].get<string>()<<endl;
                }
                break;
            }
            //加入群组响应消息
            case ADD_GROUP_MSG_ACK:
            {
                if(response["errno"].get<int>()!=0)
                {
                    cerr<<response["errmsg"].get<string>()<<endl;
                }else{
                    cout<<"uid ["<<response["uid"]<<"] 加入群组 "
                    <<"gid["<<response["groupid"]<<"] 成功！"<<endl;
                }
                break;
            }
        }
    }
}

//获取系统时间
string ClientService::getCurTime()
{
    auto tt = chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    struct tm *ptm = localtime(&tt);
    char date[60] = {0};
    sprintf(date, "%d-%02d-%02d %02d:%02d:%02d",
            (int)ptm->tm_year + 1900, (int)ptm->tm_mon + 1, (int)ptm->tm_mday,
            (int)ptm->tm_hour, (int)ptm->tm_min, (int)ptm->tm_sec);
    return string(date);
}

//主聊天页面程序
void ClientService::mainchat(const int& clientfd)
{
    //显示系统命令帮助
    help();
    string commandBuf;
    while(_isMainChatRunning)
    {
        cout<<g_curUser.getId()<<"@"<<g_curUser.getName()<<":$ ";
        getline(cin,commandBuf);
        //截取命令
        int idx=commandBuf.find(":");
        string command;
        if(idx==-1)//说明当前命名是help命令或者loginout命令
        {
            command=commandBuf;
        }else{
            command=commandBuf.substr(0,idx);
        }
        auto it=_commandHandlerMap.find(command);//查找用户输入的命令是否合法
        if(it==_commandHandlerMap.end())
        {
            cerr<<"invalid command!"<<endl;
            continue;//重新输入
        }
        //调用相应命令指定的函数
        string commandPara=commandBuf.substr(idx+1,commandBuf.size()-idx);
        it->second(clientfd,commandPara);
    }
}

//显示命令文档
void ClientService::help(int,string)
{
    cout<<"================ show command list ================"<<endl;
    for(auto& command:_commandMap)
    {
        cout<<command.first<<" : "<<command.second<<endl;
    }
    cout<<endl;
}

//一对一聊天
void ClientService::chat(int clientfd,string para)
{
    //提取消息
    int idx=para.find(":");
    if(idx==-1)
    {
        cerr<<"command invalid!"<<endl;
        return;
    }
    int friendid=atoi(para.substr(0,idx).c_str());
    string message=para.substr(idx,para.size());
    
    //封装消息
    json js;
    js["msgid"]=ONE_CHAT_MSG;
    js["id"]=g_curUser.getId();
    js["name"]=g_curUser.getName();
    js["to"]=friendid;
    js["msg"]=message;
    js["time"]=getCurTime();
    string request=js.dump();

    //转发消息给服务器
    int len=send(clientfd,request.c_str(),strlen(request.c_str())+1,0);
    if(len==-1)
    {
        cerr<<"oneChat message send error!"<<endl;
        return ;
    }
}

//客户端添加好友
void ClientService::addfriend(int clientfd,string para)
{
    int friendid=atoi(para.c_str());
    //封装消息
    json js;
    js["msgid"]=ADD_FRIEND_MSG;
    js["id"]=g_curUser.getId();
    js["friendid"]=friendid;
    //将json消息封装转发给服务器
    string requese=js.dump();
    int len=send(clientfd,requese.c_str(),strlen(requese.c_str())+1,0);
    if(len==-1)
        cerr<<"send addfriend msg error!"<<endl;
}

//创建群组
void ClientService::creategroup(int clientfd,string para)
{
    //封装消息
    int id=g_curUser.getId();
    int idx=para.find(":");
    if(idx==-1)
    {
        cerr<<"command invalid!"<<endl;
        return;
    }
    string groupname=para.substr(0,idx);
    string groupdesc=para.substr(idx,para.size()-idx);
    json js;
    js["msgid"]=CREATE_GROUP_MSG;
    js["id"]=id;
    js["groupname"]=groupname;
    js["groupdesc"]=groupdesc;
    string request=js.dump();

    //向客户端发送消息
    int len=send(clientfd,request.c_str(),strlen(request.c_str())+1,0);
    if(len==-1)
    {
        cerr<<"create group message send error!"<<endl;
        return;
    }
}

//加入群组
void ClientService::addgroup(int clientfd,string para)
{
    //封装消息
    int userid=g_curUser.getId();
    int groupid=atoi(para.c_str());
    json js;
    js["msgid"]=ADD_GROUP_MSG;
    js["id"]=userid;
    js["groupid"]=groupid;
    string request=js.dump();
    int len=send(clientfd,request.c_str(),strlen(request.c_str())+1,0);
    if(len==-1)
    {
        cerr<<"add group message send error!"<<endl;
        return;
    }
}

//群组聊天
void ClientService::groupchat(int clientfd,string para)
{
    int idx=para.find(":");
    if(idx==-1)
    {
        cerr<<"command invalid!"<<endl;
        return;
    }
    int id=g_curUser.getId();
    int groupid=atoi(para.substr(0,idx).c_str());
    string message=para.substr(idx+1,para.size()-idx);
    json js;
    js["msgid"]=GROUP_CHAT_MSG;
    js["id"]=id;
    js["groupid"]=groupid;
    js["name"]=g_curUser.getName();
    js["msg"]=message;
    js["time"]=getCurTime();

    string request=js.dump();
    int len=send(clientfd,request.c_str(),strlen(request.c_str())+1,0);
    if(len==-1)
    {
        cerr<<"group message send error!"<<endl;
        return;
    }
}

//退出登录
void ClientService::loginout(int clientfd,string)
{
    int id=g_curUser.getId();
    json js;
    js["msgid"]=LOGIN_OUT_MSG;
    js["id"]=id;
    string request=js.dump();
    int len=send(clientfd,request.c_str(),strlen(request.c_str())+1,0);
    if(len==-1)
    {
        cerr<<"quit error!"<<endl;
        return;
    }else
    {
        _isMainChatRunning=false;
    }
}