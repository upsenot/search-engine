#include"EchoServer.h"
#include <sw/redis++/utils.h>
#include <iostream>
#include <iterator>
#include "../lib/nlohmann/json.hpp"
#include "../lib/nlohmann/json_fwd.hpp"
#include <optional>
#include <string>
using namespace nlohmann;
using std::cout;
using std::endl;

#define KEYRECOMMAND 0
#define WEBPAGESEARCH 1

string MyTask::serializationForNone() const {
    json myJ;
    myJ["msgID"] = 404;
    myJ["msg"] = "未找到关键词";
    return myJ.dump(4);
}
void MyTask::process() {
    cout << "thread_name: " << threadIdx << endl;
    string response;

    json msgJ = json::parse(_msg);
        for(auto ite = msgJ.begin(); ite != msgJ.end(); ++ite)
        {
            cout << ite.key() << ":" << ite.value() << endl;
        }
    int msgID = msgJ["msgID"];
    string msg = msgJ["msg"];
    cout << "msgID:" << msgID << ",msg:" << msg << endl;

    vector<OptionalString> cache;
    // vector<optional<string>> cache;
    cache.clear();

    if (KEYRECOMMAND == msgID) {
        cout << "KeyRecommand start--->" << endl;
        _redis.hmget(msg, {to_string(msgID)}, std::back_inserter(cache));
        /* cout << "test" << endl; */
        // cout << cache[0].value() << endl;
        if (cache[0].has_value()) {
            cout << "缓存命中！" << endl;
            response = cache[0].value();
        } else {
            cout << "缓存未命中" << endl;
            response = _keyRecommander.doQuery(msg);
            /* _redis.hset(msg, std::make_pair(to_string(msgID), response)); */
            _redis.hset(msg, to_string(msgID), response);
        }
        cout << "KeyRecommand completed!" << endl;
    } else if (WEBPAGESEARCH == msgID) {
        cout << "WebPageSearch start--->" << endl;
        _redis.hmget(msg, {to_string(msgID)}, std::back_inserter(cache));
        if (cache[0].has_value()) {
            cout << "缓存命中！" << endl;
            response = cache[0].value();
        } else {
            response = _webPageSearcher.doQuery(msg);
            /* _redis.hset(msg, std::make_pair(to_string(msgID), response)); */
            _redis.hset(msg, to_string(msgID), response);
        }
        cout << "WebPageSearch completed!" << endl;
    } else {
        response = serializationForNone();
    }
    cout << "sendInloop:" << response << endl;
    _con->sendInLoop(response);
}
