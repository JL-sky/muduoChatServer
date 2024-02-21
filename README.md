# muduoChatServer

# 介绍

基于c++ muduo网络库实现的集群聊天服务器，使用nginx实现负载均衡，使用reids消息队列实现跨服务器通信

## 技术栈

- Json序列化和反序列化 
- muduo网络库开发 
- nginx的tcp负载均衡器配置 
- redis缓存服务器编程
- 基于发布-订阅的服务器中间件redis消息队列编程
- MySQL数据库编程 
- CMake构建编译环境
- 单例模式
- 智能指针
- 线程互斥、线程同步通信和 unique_lock、CAS原子整型
- 生产者消费者模型

# 功能

## 客户端

- 注册
- 登录
- 添加好友
- 创建群组、加入群组
- 好友单聊
- 群组聊天
- 离线消息存储

## 服务端

- tcp负载均衡
- 跨服务器通信

## 数据库

该项目提供数据库连接池，以优化服务端的访问性能，具体功能如下：

- **初始连接量**（initSize）：**连接池初始时为服务器准备的连接数量**。当应用发起MySQL访问时，不用再创建和MySQL Server新的连接，可直接从连接池中获取一个可用的连接，使用完成后，并不释放connection，而是把当前connection再归还到连接池当中。 
- **最大连接量**（maxSize）：**并发访问MySQL Server的请求增多时**，初始连接量已经不够使用，此时会根据新的请求数量去创建更多的连接给应用去使用，**maxSize就是新创建的连接数量的上限**。
- **最大空闲时间**（maxIdleTime）：当访问MySQL的并发请求多了以后，连接池里面的连接数量会动态增加，上限是maxSize个，当这些连接用完再次归还到连接池当中。如果在**指定的maxIdleTime里面， 这些新增加的连接都没有被再次使用过，那么新增加的这些连接资源就要被回收掉，只需要保持初始连 接量initSize个连接即可**。 
- **连接超时时间**（connectionTimeout）：**当MySQL的并发请求量过大，连接池中的连接数量已经到达 maxSize**，而此时没有空闲的连接可供使用，那么此时应用从连接池获取连接无法成功，**通过阻塞的方式获取连接的时间如果超过connectionTimeout时间，那么获取连接失败，无法访问数据库。**

# 环境配置

1. linux

2. boost+muduo网络库

   [c++ muduo网络库源码编译安装-CSDN博客](https://blog.csdn.net/qq_58158950/article/details/135669038?ops_request_misc=%7B%22request%5Fid%22%3A%22170850414816777224454509%22%2C%22scm%22%3A%2220140713.130102334.pc%5Fblog.%22%7D&request_id=170850414816777224454509&biz_id=0&utm_medium=distribute.pc_search_result.none-task-blog-2~blog~first_rank_ecpm_v1~rank_v31_ecpm-2-135669038-null-null.nonecase&utm_term=muduo&spm=1018.2226.3001.4450)

3. redis 4.0.9

   - ubuntu安装命令

   ```bash
    sudo apt-get install redis-server
   ```

   - 查看是否安装成功

   ```bash
   ps -ef | grep redis
   ```

4. 安装hiredis（redis对应的c++客户端编程语言）

   - 克隆源码

   ```bash
   git clone https://github.com/redis/hiredis
   ```

   - 进入源码文件并编译安装

   ```bash
   cd hiredis
   ```

   ```bash
   make
   ```

   ```bash
   sudo make install
   ```

   - 拷贝生成的动态库到/usr/local/lib目录下

   ```bash
   sudo ldconfig /usr/local/lib
   ```

   

5. mysql 5.7

   [ubuntu安装mysql5.7（图文详解）_在ubuntu安装mysql-CSDN博客](https://blog.csdn.net/qq_58158950/article/details/135667062?ops_request_misc=%7B%22request%5Fid%22%3A%22170850438216800185861806%22%2C%22scm%22%3A%2220140713.130102334.pc%5Fblog.%22%7D&request_id=170850438216800185861806&biz_id=0&utm_medium=distribute.pc_search_result.none-task-blog-2~blog~first_rank_ecpm_v1~rank_v31_ecpm-4-135667062-null-null.nonecase&utm_term=mysql&spm=1018.2226.3001.4450)

6. nginx 1.12

   安装nginx

   - 下载nginx 1.12压缩包，解压后进入解压文件夹中，并将终端切换到root用户，依次执行下述终端命令

   ```bash
   # nginx默认并没有编译tcp负载均衡模块，编写它时，需要加入--with-stream参数来激活这个模块
   ./configure --with-stream
   ```

   - 编译安装

   ```bash
    make && make install
   ```

   - 编译完成后，默认安装在了/usr/local/nginx目录

   ```bash
    cd /usr/local/nginx/ 
   ```

   其中可执行文件在sbin目录里面，配置文件在conf目录里面

   配置tcp负载均衡

   - 在conf目录里面配置nginx.conf文件，配置如下

     ```bash
     
       #nginx tcp loadbalance config;
       stream{
           #nginx loadbalance server
           upstream MyServer{
           	#nginx 负载的服务器ip和端口，以及每个
               server 127.0.0.1:6000 weight=1 max_fails=3 fail_timeout=30s;
               server 127.0.0.1:6002 weight=1 max_fails=3 fail_timeout=30s;
           }
     
           server{
               proxy_connect_timeout 1s;
               #nginx's port
               listen 8000;
               proxy_pass MyServer;
               tcp_nodelay on;
           }
       }
     
     ```

   -  重新加载配置文件并启动

     ```bash
     nginx -s reload 
     ```

7. cmake

# 编译运行

## 编译

1. 进入conf文件下修改mysql.cnf文件里的数据库配置，原始配置如下

   ```txt
   
     ip=127.0.0.1
     port=3306
     username=root#用户名
     passwd=123456#密码
     dbname=muduochat#数据库
   
     #连接池初始连接量
     initSize=10
     #连接池最大连接量
     maxSize=1024
     #最大空闲时间，默认是秒
     maxIdleTime=60
     #连接超时时间，默认是毫秒
     maxConnectionTimeout=100
   
   ```

2. 执行一键编译脚本

   ```bash
   ./autobuild.sh
   ```

## 测试运行

> 说明：项目具备集群服务器功能，并支持跨服务器通信，用户可使用单个服务器进行测试，也可打开多个服务器测试跨服务器功能。
>
> 本示例运行测试的是跨服务器功能，以打开两个服务器为例，若要打开多个服务器，可自行修改nginx配置
>
> 其中两个服务器的端口号为6000和6002，集群端口为8000

打开四个终端，进入bin目录下，两个终端作为服务器测试，两个终端作为客户端测试

### 运行服务端

- 打开第一个服务器

  ```bash
  ./ChatServer 127.0.0.1 6000
  ```

- 重新打开一个终端，打开第二个服务器

  ```bash
  ./ChatServer 127.0.0.1 6002
  ```

  

### 运行客户端

打开另外两个个终端，运行客户端

```bash
./ChatClient 127.0.0.1 8000
```

运行结果如下

![](https://s3.bmp.ovh/imgs/2024/02/20/b25656f190f2778a.png)

### 功能测试

#### 注册

- 输入选项2

- 按照提示依次输入用户需要注册的name和密码

- 服务器将返回给用户一个注册好的id（也就是用户账号，后续用户登录需要使用这个账号进行登录，因此需要记住服务器返回的账号）
  

  如下所示，连续注册两个账号：

```bash
choice:2
name:jack
passwd:123456
jack register success! id is 1,please remeber your id!
```



```bash
choice:2
name:july
passwd:666666
july register success! id is 2,please remeber your id!

```



#### 登录

分别在两个客户端下进行登录

- 输入选项3
- 使用注册好的账号（注册时服务器返回的id）和密码进行登录

```bash
choice:1
id:1
passwd:123456
 login success! welcome [jack],your id is [1]

```



```bash
choice:1
id:2
passwd:666666
 login success! welcome [july],your id is [2]

```



登录成功后，客户端会显示当前用户所有信息及各业务指令

```bash
addgroup : 添加群组     example:  addgroup:groupid
creategroup : 创建群组  example:  creategroup:groupname:groupdesc
loginout : 注销 example:  loginout
addfriend : 添加好友    example:  addfriend:friendid
groupchat : 群聊        example:  groupchat:groupid:message
chat : 一对一聊天       example:  chat:friendid:message
help : 显示所有支持的命令       example:  help
```



#### 个人与个人聊天

指令格式

```bash
chat:friendid:message
```

如下所示，1号用户（用户名为jack）给2号用户发送消息：

![](https://s3.bmp.ovh/imgs/2024/02/21/5cce5053b98343b2.png)

#### 群聊

#### 创建群组

指令格式

```bash
creategroup:groupname:groupdesc
```

如下所示，1号用户创建群组myGroup

```bash
1@jack:$ creategroup:myGroup:this is myGroup         
1@jack:$ 群组创建成功！  gid=1  gname=myGroup
```

#### 加入群组

指令格式

```bash
addgroup:groupid
```

如下所示，2号用户加入群组myGroup

```bash
2@july:$ addgroup:1
2@july:$ uid [2] 加入群组 gid[1] 成功！
```



#### 群组聊天

为模拟群组聊天，可使用上述指令再注册一个用户，并加入群组myGroup，如下所示

用户1（或者用户2）首先退出

```bash
loginout
```

之后再次注册一个新用户

```bash
choice:2
name:jj
passwd:123456
jj register success! id is 3,please remeber your id!
```

新用户登录并加入群组

```bash
choice:1
id:3
passwd:123456
```

```bash
3@jj:$ addgroup:1
3@jj:$ uid [3] 加入群组 gid[1] 成功！
```

群组聊天

指令格式

```bash
 groupchat:groupid:message
```

如下所示，用户1发送群消息。

```bash
1@jack:$ groupchat:1:wowowowowo
```

用户2和用户3，均可以收到群消息

```bash
2@july:$ 群消息[1]      time:2024-02-21 00:32:56  id:1  name:jack  message:wowowowowo
```

```bash
3@jj:$ 群消息[1]        time:2024-02-21 00:32:56  id:1  name:jack  message:wowowowowo
```


