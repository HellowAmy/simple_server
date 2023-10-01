
#include "server_files.h"

#define ADD_TASK(str) _map_task.emplace(str,std::bind(&server_files::task_##str,this,_1,_2))

#define ERR_BACK(err)                                               \
{                                                                   \
    channel->send(set_files_json(set_error_info(err,err##_S)));     \
    vlogw($(err##_S));                                              \
}

#define ERR_BACK_S(err,sjson)                                       \
{                                                                   \
    channel->send(set_files_json(set_error_info(err,sjson)));       \
    vlogw("err sjson: "$(sjson));                                   \
}

server_files::server_files()
{
    fn_open = bind(&server_files::open,this,_1,_2);
    fn_message = bind(&server_files::message,this,_1,_2);
    fn_close = bind(&server_files::close,this,_1);

    ADD_TASK(files_create_upload);
    ADD_TASK(files_finish_upload);
    ADD_TASK(files_create_download);
    ADD_TASK(files_begin_download);
    ADD_TASK(files_cancel_download);
}

void server_files::open(const sp_channel &channel, const sp_http &http)
{
    vlogi("open:" $(http->path));
    if(check_sjson_head(http->path) == false)
    {
        channel->close();
        vlogw("check path error:" $(http->path));
    }
}

void server_files::message(const sp_channel &channel, const string &msg)
{

}

void server_files::close(const sp_channel &channel)
{

}

bool server_files::check_sjson_head(string flg)
{
    if(flg == "/"+protocol::_head_) return true;
    return false;
}

void server_files::init_data(sqlite_account *account,sqlite_friends *friends,sqlite_info *info,sqlite_cache *cache)
{
    _db_account    = account;
    _db_friends    = friends;
    _db_info       = info;
    _db_cache      = cache;
}

void server_files::transmit_msg(const sp_channel &channel, const string &msg)
{
    char c = check_files_flg(msg);
    if(c == CS_FILES_JSON)
    {
        //解析出json
        string sjson;
        if(get_files_json(msg,sjson))
        {
            //检查json
            string stream;
            string type;
            if(check_json(sjson,stream,type))
            {
                if(stream == _cs_)
                {
                    //执行绑定函数
                    auto it = _map_task.find(type);
                    if(it != _map_task.end()) it->second(channel,sjson);
                    else ERR_BACK(CS_ERR_NOT_TASK);
                }
                else ERR_BACK(CS_ERR_NOT_STREAM);
            }
            else vlogw("transmit_msg check_json not find:" $(sjson));
        }
        else vloge("err: get_files_json " $(msg));
    }
    else if(c == CS_FILES_BINARY)
    {
        //解析出二进制内容与定位
        int64 id;
        string buf;
        if(get_files_binary(msg,id,buf))
        {

        }
        else vloge("err: get_files_binary " $(msg));
    }
    else vlogw("transmit_msg not find");
}

void server_files::task_files_create_upload(const sp_channel &channel, const string &sjson)
{

    vlogi("task_files_create_upload");
}

void server_files::task_files_finish_upload(const sp_channel &channel, const string &sjson)
{
    vlogi("task_files_finish_upload");
}

void server_files::task_files_create_download(const sp_channel &channel, const string &sjson)
{
    vlogi("task_files_create_download");
}

void server_files::task_files_begin_download(const sp_channel &channel, const string &sjson)
{
    vlogi("task_files_begin_download");
}

void server_files::task_files_cancel_download(const sp_channel &channel, const string &sjson)
{
    vlogi("task_files_cancel_download");
}
