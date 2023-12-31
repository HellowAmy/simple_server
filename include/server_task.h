#ifndef SERVER_TASK_H
#define SERVER_TASK_H

#include "../../simple_chat/web_protocol.h"
#include "make_tools.h"
#include "inter_server.h"
#include "sqlite_op.h"


#include <functional>
#include <string>
#include <map>
#include <mutex>
#include <memory>

using namespace protocol;
using namespace std::placeholders;
using std::function;
using std::string;
using std::map;
using std::mutex;

class server_task
{
public:
    server_task();
    ~server_task();

    void init_data(sqlite_account *account,sqlite_friends *friends,sqlite_info *info,sqlite_cache *cache);

    function<void(const sp_channel&, const sp_http&)> fn_open;
    function<void(const sp_channel&, const string&)> fn_message;
    function<void(const sp_channel&)> fn_close;

private:
    mutex _mut_connect;
    map<int64,sp_channel> _map_connect;
    map<string,function<void(const sp_channel&,const string &)>> _map_task;

    sqlite_account  *_db_account;
    sqlite_friends  *_db_friends;
    sqlite_info     *_db_info;
    sqlite_cache    *_db_cache;


    //插入随机账号到数据库
    int64 insert_account(string passwd,int count = 100);

    //检查连接
    bool check_sjson_head(string flg);

    //== _map_connect operator ==
    void add_connect_th(int64 account, const sp_channel &channel);
    void move_connect_th(int64 account);
    sp_channel find_connect_th(int64 account);
    //== _map_connect operator ==

    bool passwd_verify(string passwd_db,string passwd);

    void open(const sp_channel &channel, const sp_http& http);
    void message(const sp_channel &channel, const string& msg);
    void close(const sp_channel &channel);
    void transmit_msg(const sp_channel &channel,const string &msg);

    void task_swap_msg(const sp_channel &channel,const string &sjson);
    void task_swap_cache(const sp_channel &channel,const string &sjson);
    void task_ac_login(const sp_channel &channel,const string &sjson);
    void task_ac_register(const sp_channel &channel,const string &sjson);
//    void task_ac_info(const sp_channel &channel,const string &sjson);
//    void task_ac_info_remarks(const sp_channel &channel,const string &sjson);
    void task_ac_info_all(const sp_channel &channel,const string &sjson);
    void task_ac_update_info(const sp_channel &channel,const string &sjson);
    void task_ac_update_remarks(const sp_channel &channel,const string &sjson);

    void task_ac_update_friends(const sp_channel &channel,const string &sjson);


//    void task_friends_list(const sp_channel &channel,const string &sjson);
//    void task_friends_status(const sp_channel &channel,const string &sjson);

};


#endif //SERVER_TASK_H
