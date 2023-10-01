
#ifndef SERVER_FILES_H
#define SERVER_FILES_H

#include "../../simple_chat/web_protocol.h"
#include "../../simple_chat/swap_files.h"
#include "inter_server.h"
#include "sqlite_op.h"
#include "make_tools.h"

#include <functional>
#include <string>
#include <map>
#include <set>
#include <mutex>
#include <memory>

using namespace protocol;
using namespace std::placeholders;
using std::function;
using std::string;
using std::map;
using std::set;
using std::mutex;

class server_files
{
public:
    struct fs_data
    {
        int64 time;
        int64 length_max;
        string filename;
    };
public:

    server_files();

//    void init_data(sqlite_account *account,sqlite_friends *friends,sqlite_info *info,sqlite_cache *cache);

    function<void(const sp_channel&, const sp_http&)> fn_open;
    function<void(const sp_channel&, const string&)> fn_message;
    function<void(const sp_channel&)> fn_close;

protected:
    string _swap_path;
    swap_files _swap_fs;
    mutex _mut_swap_file;
    map<int64,fs_data> _map_swap_file;
    map<string,function<void(const sp_channel&,const string &)>> _map_task;

//    sqlite_account  *_db_account;
//    sqlite_friends  *_db_friends;
//    sqlite_info     *_db_info;
//    sqlite_cache    *_db_cache;


    void open(const sp_channel &channel, const sp_http& http);
    void message(const sp_channel &channel, const string& msg);
    void close(const sp_channel &channel);

    void send_msg(const sp_channel &channel,const string &sjson);
    void send_data(const sp_channel &channel,int64 id, const string &msg);

    int64 add_map_swap_th(int64 id,fs_data data);
    int64 add_data_to_map(int64 id,const fs_data &data);
    void move_map_swap_th(int64 id);


    bool check_sjson_head(string flg);

    void transmit_msg(const sp_channel &channel,const string &msg);

    void task_recv_binary_data(int64 id,const string &data);

    void task_files_create_upload(const sp_channel &channel,const string &sjson);
    void task_files_finish_upload(const sp_channel &channel,const string &sjson);
    void task_files_create_download(const sp_channel &channel,const string &sjson);
    void task_files_begin_download(const sp_channel &channel,const string &sjson);
    void task_files_cancel_download(const sp_channel &channel,const string &sjson);

};


#endif //SERVER_FILES_H
