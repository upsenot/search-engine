#ifndef __Configuration_H__
#define __Configuration_H__

#include <string.h>
#include <unistd.h>
#include <fstream>
#include <sstream>
#include <string>
#include <unordered_map>
//宏定义 之后只要GetPath就会得到相应的配置文件
#define GetPath(query) Configuration::getInstance()->getPath(query)

using std::ifstream;
using std::istringstream;
using std::string;
using std::unordered_map;

//单例模式
//1，将构造函数私有化
//2，在类中定义一个静态的指向本类型的指针变量
//3，定义一个返回值为指针类型的静态成员函数
class Configuration
{
public:
    //只有第一次调用会创建实例
    static Configuration* getInstance()
    {
        if(_pInstance == nullptr)
        {
            _pInstance = new Configuration();
        }
        return _pInstance;
    }
    void initConfig();
    void showConfig();
    
    //删除唯一的实例化对象
    static void destroy()
    {
        if(_pInstance)
        {
            delete _pInstance;
            _pInstance = nullptr;
        }
    }

    static void print()
    {
        _pInstance->showConfig();
    }

    //获取路径
    string getPath(const string& item)
    {
        return _configs[item];
    }
    //返回_config
    unordered_map<string ,string>& getConfigMap();
private:
    Configuration(const string& filePath = "../conf/conf2.conf");
    ~Configuration(){};
private:
    static Configuration* _pInstance;//实例
    string _configFilePath;//配置文件根目录
    unordered_map<string, string> _configs;//用键值对存储配置文件

};
#endif
