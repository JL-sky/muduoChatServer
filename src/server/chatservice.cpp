#include"chatservice.hpp"
#include "public.hpp"
#include"group.hpp"
#include<muduo/base/Logging.h>
using namespace muduo;
using namespace std;

//初始化，将消息id与对应的回调函数进行绑定
ChatService::ChatService()
{
    //登录业务触发
    _msgHandlerMap.insert({LOGIN_MSG,std::bind(&ChatService::login,this,_1,_2,_3)});
    //注册业务触发
    _msgHandlerMap.insert({REG_MSG,std::bind(&ChatService::reg,this,_1,_2,_3)});
    //用户注销业务触发
    _msgHandlerMap.insert({LOGIN_OUT_MSG,std::bind(&ChatService::loginout,this,_1,_2,_3)});


    //一对一聊天业务触发
    _msgHandlerMap.insert({ONE_CHAT_MSG,std::bind(&ChatService::oneChat,this,_1,_2,_3)});
    //添加好友业务触发
    _msgHandlerMap.insert({ADD_FRIEND_MSG,std::bind(&ChatService::addFriend,this,_1,_2,_3)});

    //创建群组业务触发
    _msgHandlerMap.insert({CREATE_GROUP_MSG,std::bind(&ChatService::createGroup,this,_1,_2,_3)});
    //加入群组业务触发
    _msgHandlerMap.insert({ADD_GROUP_MSG,std::bind(&ChatService::addGroup,this,_1,_2,_3)});
    //群组聊天业务触发
    _msgHandlerMap.insert({GROUP_CHAT_MSG,std::bind(&ChatService::groupChat,this,_1,_2,_3)});

    //连接redis服务器
    if(_redis.connect())
    {
        //设置上报消息的回调
        _redis.init_notify_handler(std::bind(&ChatService::handlerRedisSubscribeMessage,this,_1,_2));
    }
}

//获取单例对象的调用接口
ChatService* ChatService::getInstance()
{
    static ChatService service;
    return &service;
}

//获取消息id对应的处理器
MsgHandler ChatService::getHandler(int msgid){
    auto it=_msgHandlerMap.find(msgid);

    //记录错误日志，消息id没有对应的事件回调handler
    if(it==_msgHandlerMap.end())
    {
        return [=](const TcpConnectionPtr& conn,json& js,Timestamp time){
            // LOG_ERROR<<"msgid:"<<msgid<<"找不到对应的handler!";
            LOG("找不到对应的handler!");
        };
    }else{
        return _msgHandlerMap[msgid];
    }
}

//处理登录业务
void ChatService::login(const TcpConnectionPtr& conn,json& js,Timestamp time)
{
    // LOG_INFO<<"do login service!!!";
    int id=js["id"].get<int>();
    string passwd=js["password"];

    //通过id号在数据库中查询用户信息
    User user=_userModel.queryById(id);
    if(user.getId()==-1)
    {
        json response;
        response["msgid"]=LOGIN_MSG_ACK;
        response["errno"]=1;
        response["errmsg"]="该用户不存在或者用户id错误";
        conn->send(response.dump());
    }else{
        if(user.getPasswd()!=passwd)
        {
            json response;
            response["msgid"]=LOGIN_MSG_ACK;
            response["errno"]=2;
            response["errmsg"]="密码错误！";
            conn->send(response.dump());
        }else{
            if(user.getState()=="online"){
                json response;
                response["msgid"]=LOGIN_MSG_ACK;
                response["errno"]=3;
                response["errmsg"]="该用户已经在线！";
                conn->send(response.dump());
            }else{
                // LOG("用户 " + user.getName() + " 登录成功!\n");
                {
                    //保存该用户对应的tcp连接信息;互斥锁，保护多线程对_connMap的访问
                    lock_guard<mutex> lock(_userConnMutex);
                    //_connMap用户保存每个用户对应的tcp连接，以便聊天通信使用
                    _userConnMap.insert({id,conn});
                }
                //id用户登录成功后，向redis订阅channel（id）
                _redis.subscribe(id);

                //设置用户状态
                user.setState("online");
                _userModel.updateState(user);

                json response;//响应消息
                response["msgid"]=LOGIN_MSG_ACK;
                response["errno"]=0;
                response["id"]=user.getId();
                response["name"]=user.getName();
                response["state"]=user.getState();

                //判断是否有离线消息
                vector<string> offlineMsg=_offlineMsgModel.query(id);
                if(!offlineMsg.empty())
                {
                    //拉取该用户的离线消息
                    response["offlineMsg"]=offlineMsg;
                    //拉取用户的离线消息后就将其从数据库中删除
                    _offlineMsgModel.remove(id);

                }

                //拉取好友信息
                vector<User> friendMsgVec=_friendModel.query(id);
                if(!friendMsgVec.empty())
                {
                    vector<string> ftemp;
                    for(auto friendMsg:friendMsgVec)
                    {
                        json temp;
                        temp["id"]=friendMsg.getId();
                        temp["name"]=friendMsg.getName();
                        temp["state"]=friendMsg.getState();
                        ftemp.push_back(temp.dump());
                    }
                    response["friends"]=ftemp;
                }

                //查询用户的群组信息
                vector<Group> groupMsgVec=_groupModel.queryGroups(id);
                if(!groupMsgVec.empty()){
                    vector<string> groupv;
                    for(Group &group:groupMsgVec)
                    {
                        json groupjs;
                        //用户所在群组信息
                        groupjs["gid"]=group.getId();
                        groupjs["gname"]=group.getGroupName();
                        groupjs["gdesc"]=group.getGroupDesc();
                        
                        //用户所在群组的其他成员信息
                        vector<string> guserV;
                        for(GroupUser& guser:group.getGroupUsers())
                        {
                            json guserJs;
                            guserJs["uid"]=guser.getId();
                            guserJs["uname"]=guser.getName();
                            guserJs["state"]=guser.getState();
                            guserJs["role"]=guser.getRole();
                            guserV.push_back(guserJs.dump());
                        }
                        groupjs["gusers"]=guserV;

                        groupv.push_back(groupjs.dump());
                    }

                    response["groups"]=groupv;
                }

                conn->send(response.dump());
            }
        }
    }
}

//处理注册业务
void ChatService::reg(const TcpConnectionPtr& conn,json& js,Timestamp time)
{
    // LOG_INFO<<"do reg service!!!";
    //获取从客户端收到的注册信息
    string name=js["name"];
    string passwd=js["password"];

    //封装用户的注册信息
    User user;
    user.setName(name);
    user.setPasswd(passwd);

    bool state=_userModel.insert(user);
    if(state)//服务端向客户端返回注册响应信息
    {
        //注册成功
        // LOG("用户 "+ name+" 注册成功!\n");
        json response;
        response["msgid"]=REG_MSG_ACK;
        response["errno"]=0;
        response["id"]=user.getId();

        //向客户端发送响应数据
        conn->send(response.dump());
        
    }else{
        //注册失败
        // LOG("用户 "+ name+ " 注册失败!\n");
        json response;
        response["msgid"]=REG_MSG_ACK;
        response["errno"]=1;
        response["errmsg"]="注册失败！";
        conn->send(response.dump());
    }
}

//处理注销业务
void ChatService::loginout(const TcpConnectionPtr& conn,json& js,Timestamp time)
{
    int id=js["id"].get<int>();
    {
        //删除该用户的连接
        lock_guard<mutex> lock(_userConnMutex);
        auto it=_userConnMap.find(id);
        if(it!=_userConnMap.end())
        {
            _userConnMap.erase(it);
        }
    }
    //重置该用户的在线状态
    User user(id,"","","offline");
    _userModel.updateState(user);

    //用户注销后，取消对消息队列中的订阅状态
    _redis.unsubscribe(id);
}

//一对一聊天业务处理
void ChatService::oneChat(const TcpConnectionPtr& conn,json& js,Timestamp time)
{
    int toid=js["to"].get<int>();
    {
        lock_guard<mutex> lock(_userConnMutex);
        auto it=_userConnMap.find(toid);//找到对端连接
        if(it!=_userConnMap.end())//toid当前服务器上，服务器将消息推送给toid
        {
            it->second->send(js.dump());
            return;
        }
    }

    //查询toid是否在其他服务器上在线
    User user=_userModel.queryById(toid);
    if(user.getState()=="online")//如果用户在其他服务器上在线，就通过消息队列转发消息
    {
        _redis.publish(toid,js.dump());
        return;
    }

    //离线消息
    //对方不在线，将发生给对方的离线消息存储到数据库，等对方上线时再进行拉取
    _offlineMsgModel.insert(toid,js.dump());
}

//添加好友业务
void ChatService::addFriend(const TcpConnectionPtr& conn,json& js,Timestamp time)
{
    int userId=js["id"].get<int>();
    int friendId=js["friendid"].get<int>();
    //添加好友
    _friendModel.insert(userId,friendId);
}

//创建群组业务
void ChatService::createGroup(const TcpConnectionPtr& conn,json& js,Timestamp time)
{
    int userid=js["id"].get<int>();
    string gname=js["groupname"];
    string gdesc=js["groupdesc"];
    Group group(-1,gname,gdesc);
    bool state=_groupModel.createGroup(group);
    if(state)
    {
        //存储创建人的信息
        _groupModel.addGroup(userid,group.getId(),"creator");//角色为群主
        json response;
        response["msgid"]=CREATE_GROUP_MSG_ACK;
        response["errno"]=0;
        response["groupid"]=group.getId();
        response["groupname"]=gname;
        conn->send(response.dump());
    }else{
        json response;
        response["msgid"]=CREATE_GROUP_MSG_ACK;
        response["errno"]=1;
        response["errmsg"]="创建群组失败！";
        conn->send(response.dump());
    }
}

//加入群组业务
void ChatService::addGroup(const TcpConnectionPtr& conn,json& js,Timestamp time)
{
    int userid=js["id"].get<int>();
    int gid=js["groupid"].get<int>();
    bool state=_groupModel.addGroup(userid,gid,"normal");
    if(state)
    {
        json response;
        response["msgid"]=ADD_GROUP_MSG_ACK;
        response["errno"]=0;
        response["uid"]=userid;
        response["groupid"]=gid;
        conn->send(response.dump());
    }else{
        json response;
        response["msgid"]=ADD_GROUP_MSG_ACK;
        response["errno"]=1;
        response["errmsg"]="加入群组失败！";
        conn->send(response.dump());
    }
}

//群组聊天业务
void ChatService::groupChat(const TcpConnectionPtr& conn,json& js,Timestamp time)
{
    int userid=js["id"].get<int>();
    int gid=js["groupid"].get<int>();
    vector<int> groupUsers=_groupModel.queryGroupUsers(userid,gid);
    lock_guard<mutex> lock(_userConnMutex);
    for(auto uid:groupUsers)//当前群组的其余用户
    {
        auto it=_userConnMap.find(uid);
        if(it!=_userConnMap.end())
        {
            //在当前服务器上转发群消息
            it->second->send(js.dump());
        }else{
            //查询用户是否在其他服务器上登录
            User user=_userModel.queryById(uid);
            if(user.getState()=="online")
            {
                _redis.publish(uid,js.dump());
            }
            else{
                //存储离线群信息
                _offlineMsgModel.insert(uid,js.dump());
            }
        }
    }
}

//客户端异常断开连接处理
void ChatService::clientCloseException(const TcpConnectionPtr& conn)
{
    User user;
    {
        //不同于mysql内部，STL不提供多线程数据共享保护机制，需要自己手动进行
        lock_guard<mutex> lock(_userConnMutex);

        //删除该用户的tcp连接
        for(auto it=_userConnMap.begin();it!=_userConnMap.end();it++)
        {
            if(it->second==conn)
            {
                // user=_userModel.queryById(it->first);
                user.setId(it->first);
                _userConnMap.erase(it);
                break;
            }
        }
    }

    //用户注销，取消消息订阅状态
    _redis.unsubscribe(user.getId());

    //将该用户的状态设置回offline
    if(user.getId()!=-1)//如果id值等于默认值-1,说明没有找到异常的用户，不需要执行sql
    {
        user.setState("offline");
        _userModel.updateState(user);
        LOG_ERROR<<"用户 "<<user.getId()<<" 已断开连接！\n";
    }
}

//服务端ctrl+c异常退出处理
void ChatService::reset()
{
    _userModel.resetState();
}

//从redis消息队列中获取订阅的消息
void ChatService::handlerRedisSubscribeMessage(int channel,string msg)
{
    lock_guard<mutex> lock(_userConnMutex);
    auto it=_userConnMap.find(channel);
    if(it!=_userConnMap.end())
    {
        it->second->send(msg);
        return;
    }
    //存储用户的离线消息
    _offlineMsgModel.insert(channel,msg);
}