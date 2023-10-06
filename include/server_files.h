
#ifndef SERVER_FILES_H
#define SERVER_FILES_H

#include "../../simple_chat/web_protocol.h"
//#include "../../simple_chat/swap_files.h"
#include "../../simple_chat/files_transfer.h"
#include "inter_server.h"
#include "sqlite_op.h"
#include "make_tools.h"

#include <functional>
#include <string>
#include <map>
#include <set>
#include <mutex>
#include <queue>
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
//    enum swap_type{ e_send,e_recv };
//    struct fs_data
//    {
//        fs_data(swap_type n1,int64 n2) : type(n1),id(n2) {}
//        swap_type type;
//        int64 id;
//    };
//    struct fs_swap_id
//    {
//        fs_swap_id(set<int64> n1,set<int64> n2) : _set_recv(n1),_set_send(n2) {}
//        set<int64> _set_recv;
//        set<int64> _set_send;
//    };

public:

    server_files();

    function<void(const sp_channel&, const sp_http&)> fn_open;
    function<void(const sp_channel&, const string&)> fn_message;
    function<void(const sp_channel&)> fn_close;

protected:
//    mutex _mut_send_map;
//    mutex _mut_send_que;
    string _path_temp_save;
    files_channel _fs_swap;
    map<string,function<void(const sp_channel&,const string &)>> _map_task;
//    map<int64,sp_channel> _map_send;
//    queue<int64> _que_send;


    bool add_send_map(int64 id,const sp_channel &channel);
    void remove_send_map(int64 id);
    sp_channel find_channel_send_map(int64 id);

    void push_send_que(int64 id);
    bool pop_send_que(int64 &id);

//    void close_file_send(const sp_channel &channel,int64 id);
//    void close_file_recv(const sp_channel &channel,int64 id);
//    void close_file_channel(const sp_channel &channel);


    bool check_sjson_head(string flg);
    int64 make_fs_id(); //创建不重复的文件ID


    bool send_msg(const sp_channel &channel,const string &sjson);
    bool send_data(const sp_channel &channel,int64 id, const string &msg);

    void open(const sp_channel &channel, const sp_http& http);
    void message(const sp_channel &channel, const string& msg);
    void close(const sp_channel &channel);


    void transmit_msg(const sp_channel &channel,const string &msg);
    void task_recv_binary_data(int64 id,const string &data);

    void task_files_create_upload(const sp_channel &channel,const string &sjson);
    void task_files_finish_upload(const sp_channel &channel,const string &sjson);

    void task_files_create_download(const sp_channel &channel,const string &sjson);
    void task_files_begin_download(const sp_channel &channel,const string &sjson);

    void task_files_cancel_download(const sp_channel &channel,const string &sjson);
};


#endif //SERVER_FILES_H
