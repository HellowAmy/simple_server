#ifndef INTER_SERVER_H
#define INTER_SERVER_H

#include <functional>
#include <string>

#include "../include/hv/WebSocketServer.h"

using hv::WebSocketServer;
using hv::WebSocketService;

using std::function;
using std::string;

typedef WebSocketChannelPtr sp_channel;
typedef HttpRequestPtr sp_http;

//!
//! 类说明： WebSocket 服务器连接
//!
class inter_server
{
public:
    inter_server(){}

    void func_bind(function<void(const sp_channel&, const sp_http&)> open,
                   function<void(const sp_channel&, const string&)> message,
                   function<void(const sp_channel&)> close)
    {
        _wsc.onopen = open;
        _wsc.onmessage = message;
        _wsc.onclose = close;
    }

    int open(int port = 4444,int th_mun = 4)
    {
        _wsv.registerWebSocketService(&_wsc);
        _wsv.setPort(port);
        _wsv.setThreadNum(th_mun);
        return _wsv.run();
    }

    WebSocketServer* get_sv() { return &_wsv; }
    WebSocketService* get_sc() { return &_wsc; }

protected:
    WebSocketServer _wsv;
    WebSocketService _wsc;
};

#endif // INTER_SERVER_H
