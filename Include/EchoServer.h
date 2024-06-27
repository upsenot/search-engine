#ifndef __ECHOSERVER_H__
#define __ECHOSERVER_H__

#include "TcpServer.h"
#include "ThreadPool.h"
#include"KeyRecommander.h"
#include"WebPageSearcher.h"
#include<sw/redis++/redis++.h>
#include <iostream>
#include <unistd.h>

using namespace sw::redis;

using std::cout;
using std::endl;

class MyTask
{
public:
    MyTask(const string &msg, const TcpConnectionPtr &con, KeyRecommander& keyRecommander,WebPageSearcher& webPageSearcher, Redis& redis)
    : _msg(msg)
    , _con(con)
    ,_keyRecommander(keyRecommander)
    ,_webPageSearcher(webPageSearcher)
    ,_redis(redis)
    {

    }

    //该函数在线程池中执行的
    void process();
private:
    string serializationForNone() const;
private:
    string _msg;
    TcpConnectionPtr _con;
    KeyRecommander& _keyRecommander;
    WebPageSearcher& _webPageSearcher;
    Redis& _redis;
    
};

class EchoServer
{
public:
    EchoServer(size_t threadNum, size_t queSize
               , const string &ip, unsigned short port)
    : _pool(threadNum, queSize)
    , _server(ip, port)
    ,_keyRecommander()
    ,_webPageSearcher()
    ,_redis(GetPath("REDISPATH"))
    {

    }

    void start()
    {
        _pool.start();//计算线程全部启动
        using namespace std::placeholders;
        _server.setAllCallback(bind(&EchoServer::onConnection, this, _1)
                               , bind(&EchoServer::onMessage, this, _1)
                               , bind(&EchoServer::onClose, this, _1));
        _server.start();//IO线程启动
    }

    void stop()
    {
        _pool.stop();
        _server.stop();
    }

    void onConnection(const TcpConnectionPtr &con)
    {
        cout << con->toString() << " has connected!" << endl;
    }
    
    void onMessage(const TcpConnectionPtr &con)
    {
        //回显
        string msg = con->receive();
        cout << "recv msg  " << msg << endl;
        //接收与发送之间，消息msg是没有做任何处理的
        //...
        //
        //处理msg的业务逻辑全部在此处实现的话
        //将msg这些信息打个包交给MyTask，然后将MyTask注册给线程池
        //让线程池去处理具体的业务逻辑，此时业务逻辑的处理就不在
        //EventLoop线程中
        
        MyTask task(msg, con, _keyRecommander, _webPageSearcher, _redis);
        /* gThreadPool->addTask(std::bind(&MyTask::process, &task)); */
        _pool.addTask(std::bind(&MyTask::process, task));
    
    }
    
    void onClose(const TcpConnectionPtr &con)
    {
        cout << con->toString() << " has closed!" << endl;
    }
private:
    ThreadPool _pool;
    TcpServer _server;
    KeyRecommander _keyRecommander;
    WebPageSearcher _webPageSearcher;
    Redis _redis; 
    
};

#endif
