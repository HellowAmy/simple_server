
#include "server_files.h"

#define ADD_TASK(str) _map_task.emplace(str,std::bind(&server_files::task_##str,this,_1,_2))

//#define ERR_BACK(err)                                               \
//{                                                                   \
//    channel->send(set_files_json(set_error_info(err,err##_S)));     \
//    vlogw($(err##_S));                                              \
//}

#define ERR_BACK_S(err,sjson)                                       \
{                                                                   \
    channel->send(set_files_json(set_error_info(err,sjson)));       \
    vlogw("err sjson: "$(sjson));                                   \
}

server_files::server_files()
{
    _swap_path = "../temp_swap_file/";

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
    vlogi($(msg) $(channel->writeBufsize()));
    transmit_msg(channel,msg);
}

void server_files::close(const sp_channel &channel)
{
    vlogi("close");
}

bool server_files::check_sjson_head(string flg)
{
    if(flg == "/"+protocol::_head_) return true;
    return false;
}

//void server_files::init_data(sqlite_account *account,sqlite_friends *friends,sqlite_info *info,sqlite_cache *cache)
//{
//    _db_account    = account;
//    _db_friends    = friends;
//    _db_info       = info;
//    _db_cache      = cache;
//}

int64 server_files::add_data_to_map(int64 id, const server_files::fs_data &data)
{
    auto it = _map_swap_file.emplace(id,data);
    if(it.second) return id;
    else return add_data_to_map(id+1,data);
}

int64 server_files::add_map_swap_th(int64 id, server_files::fs_data data)
{
    std::unique_lock<mutex> lock(_mut_swap_file);
    return add_data_to_map(id,data);
}

void server_files::move_map_swap_th(int64 id)
{
    std::unique_lock<mutex> lock(_mut_swap_file);
    _map_swap_file.erase(id);
}

void server_files::send_msg(const sp_channel &channel,const string &sjson)
{
    string s = set_files_json(sjson);
    channel->send(s);
}

void server_files::send_data(const sp_channel &channel,int64 id, const string &msg)
{
    string s = set_files_binary(id,msg);
    channel->send(s);
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
                    else vlogw("not find task func");
                }
                else vlogw("not find stream type");
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
            task_recv_binary_data(id,buf);
        }
        else vloge("err: get_files_binary " $(msg));
    }
    else vlogw("transmit_msg not find");
}

void server_files::task_recv_binary_data(int64 id, const string &data)
{
    bool ok = _swap_fs.add_recv_buf(id,data);
    if(ok == false) vlogw("err: task_recv_binary_data ");


    //== 查看进度 ==
    auto it_map = _map_swap_file.find(id);
    auto it_fs = _swap_fs.find_fs(id);
    if(it_map != _map_swap_file.end() && it_fs != nullptr)
    {
        static int sprog = 0;
        int prog = it_fs->tellg()*100 / it_map->second.length_max;
        if(prog > sprog)
        {
            sprog = prog;
            vlogi($(prog) $(it_fs->tellg()) $(it_map->second.length_max));
        }
    }
    else vlogw("err:task_recv_binary_data");
}

void server_files::task_files_create_upload(const sp_channel &channel, const string &sjson)
{
    vlogi("task_files_create_upload");
    int64 time;
    int64 target;
    int64 source;
    int64 length_max;
    string filename;
    if(get_files_create_upload(sjson,time,target,source,length_max,filename))
    {
        //生成交换文件id
        int64 id_tm = make_tools::make_id(time,target,source);
        int64 id = add_map_swap_th(id_tm,{time,length_max,filename});
        string path = _swap_path + std::to_string(id);

        //创建文件写入
        _swap_fs.open_recv_file(id,length_max,path);

        //反馈数据
        string s = set_files_create_upload_back(time,id,true);
        send_msg(channel,s);
    }
    else vlogw("err: task_files_create_upload");

    vlogi($(time) $(target) $(source) $(length_max) $(filename));
}

void server_files::task_files_finish_upload(const sp_channel &channel, const string &sjson)
{
    vlogi("task_files_finish_upload");
    int64 id;
    bool is_swap;
    bool finish;
    if(get_files_finish_upload(sjson,id,is_swap,finish))
    {
        //判断文件大小是否相等
        bool ret = true;
        auto it_map = _map_swap_file.find(id);
        auto it_fs = _swap_fs.find_fs(id);
        if(it_map != _map_swap_file.end() && it_fs != nullptr)
        {
            if(it_map->second.length_max != it_fs->tellg()) ret = false;
        }

        //完成文件接收，关闭文件流
        _swap_fs.close_recv_buf(id);

        //非交换时完成传输并关闭，
        if(is_swap) move_map_swap_th(id);

        //删除不完整文件
        if(ret == false || finish == false)
        { swap_files::remove_file(_swap_path + std::to_string(id)); }

        //反馈数据
        string s = set_files_finish_upload_back(id,ret);
        send_msg(channel,s);
    }
    else vlogw("err: task_files_finish_upload");

    vlogi($(id) $(is_swap) $(finish) );
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





