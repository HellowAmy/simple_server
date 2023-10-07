
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

    server_files();

    function<void(const sp_channel&, const sp_http&)> fn_open;
    function<void(const sp_channel&, const string&)> fn_message;
    function<void(const sp_channel&)> fn_close;

protected:
    string _path_temp;
//    string _path_icon;
    files_channel _fs_swap;
    map<string,function<void(const sp_channel&,const string &)>> _map_task;

    bool check_sjson_head(string flg);  //检查连接
    int64 make_fs_id();                 //创建不重复的文件ID

    bool send_msg(const sp_channel &channel,const string &sjson);
    bool send_data(const sp_channel &channel,int64 id, const string &msg);

    void open(const sp_channel &channel, const sp_http& http);
    void message(const sp_channel &channel, const string& msg);
    void close(const sp_channel &channel);

    void transmit_msg(const sp_channel &channel,const string &msg);
    void task_recv_binary_data(int64 id,const string &data);

    void task_files_create_upload(const sp_channel &channel,const string &sjson);
    void task_files_begin_upload(const sp_channel &channel,const string &sjson);
    void task_files_finish_upload(const sp_channel &channel,const string &sjson);

    void task_files_create_download(const sp_channel &channel,const string &sjson);
    void task_files_begin_download(const sp_channel &channel,const string &sjson);
    void task_files_finish_download(const sp_channel &channel,const string &sjson);

//    void task_files_cancel_download(const sp_channel &channel,const string &sjson);
};


#endif //SERVER_FILES_H
