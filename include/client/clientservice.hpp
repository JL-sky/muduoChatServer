#ifndef CLIENTSERVICE_H
#define CLIENTSERVICE_H
#include<vector>
#include<string>
#include<ctime>
#include<unordered_map>
#include<functional>
#include<atomic>
#include<semaphore.h>

#include "user.hpp"
#include "group.hpp"
#include "groupuser.hpp"

#include "json.hpp"
using json=nlohmann::json;
using namespace std;

class ClientService
{
public:
    static ClientService* getInstance(const int& clientfd);
    //页面菜单
    void menu();
    //显示当前登录成功用户的基本信息
    void showCurUserData();

    //客户端登录
    void clientLogin(const int& clientfd);
    //客户端账号注册
    void clientRegister(const int& clientfd);

    //获取系统时间
    string getCurTime();
    //主聊天页面程序
    void mainchat(const int& clientfd);
    //接收消息线程
    // static void readTaskHandler(const int& clientfd);
    void readTaskHandler(const int& clientfd);
    // void readTaskHandler(const int& clientfd);

    //主聊天业务
    void help(int=-1,string="");//帮助
    void chat(int,string);//一对一聊天
    void addfriend(int,string);//加好友
    void creategroup(int,string);//创建群组
    void addgroup(int,string);//加入群组
    void groupchat(int,string);//群组聊天
    void loginout(int,string);//退出登录
private:
    ClientService();
    ClientService(const int&clientfd);

    ~ClientService();
    //控制聊天页面程序
    bool _isMainChatRunning;
    //控制接收线程,该线程只能启动一次,防止用户退出登录再次登录时开启多个接受线程浪费资源
    int s_readThreadNumber;

    //记录当前系统登录的用户信息
    User g_curUser;
    //记录当前系统登录用户的好友列表信息
    vector<User> g_curFriendList;
    //记录当前系统登录用户的群组列表信息
    vector<Group> g_curUserGroupList;        

    // 用于读写线程之间的通信
    sem_t _rwsem;
    // 记录登录状态
    atomic_bool _isLoginSuccess{false};
    //登录响应
    void doLoginResponse(const int&clientfd,json &responsejs);
    //注册响应
    void doRegResponse(const int&clientfd,json &responsejs);


    //系统支持的客户端命令列表
    unordered_map<string,string> _commandMap;
    //客户端命令处理
    unordered_map<string,function<void(int,string)>> _commandHandlerMap;                                        
};

#endif