
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
    _path_temp_save = "../temp_swap_file/";

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
//    //发送数据
//    auto fn_send = [=](int64 id,const string &data){
//        vlogi("fn_send");
//        auto sp_channel = find_channel_send_map(id);
//        return true;
//    };

    channel->onwrite = [=](hv::Buffer* buf){

//        vlogi("onwrite");
//        if(string((char*)buf->data()) == WS_SERVER_PONG_FRAME)
//        {
//            int64 id;
//            if(pop_send_que(id))
//            {
//                vlogi("pop_send_que:" $(id));
//                bool is_finish = _swap_fs.add_data_send(id,fn_send);
//
//                string s = set_files_finish_download(id,is_finish);
//                send_msg(channel,s);
//
//                if(is_finish)
//                {
//                    vlogd("finish:" $(id));
//                    _swap_fs.close_file_send(id);
//                }
//            }
//            vlogi("WS_SERVER_PONG_FRAME");
//        }
//
////        if(pop_send_que)
////        find_channel_send_map()
//
////        vlogi($(buf->data()) $(buf->size()));
//        size_t size = channel->writeBufsize();
//        vlogd($(size) $(buf->size()));

        if(string((char*)buf->data()) == WS_SERVER_PING_FRAME) vlogd("onwrite: WS_SERVER_PING_FRAME 111");
        else if(string((char*)buf->data()) == WS_SERVER_PONG_FRAME) vlogd("onwrite: WS_SERVER_PING_FRAME 222");

    };

    vlogi("open:" $(http->path));
    if(check_sjson_head(http->path) == false)
    {
        channel->close();
        vlogw("check path error:" $(http->path));
    }
}

void server_files::message(const sp_channel &channel, const string &msg)
{
//    vlogi($(msg) $(channel->writeBufsize()));
    transmit_msg(channel,msg);
}

void server_files::close(const sp_channel &channel)
{
    vlogi("close" );
    auto it = channel->getContextPtr<fs_data>();
    if(it != nullptr)
    {
        if(it->type == e_recv) _swap_fs.close_file_recv(it->id);
        else if(it->type == e_send)
        {
            _swap_fs.close_file_send(it->id);
            remove_send_map(it->id);
        }
        vlogd("close: " $(it->type) $(it->id));
    }
}

bool server_files::check_sjson_head(string flg)
{
    if(flg == "/"+protocol::_head_) return true;
    return false;
}

bool server_files::send_msg(const sp_channel &channel,const string &sjson)
{
    string s = set_files_json(sjson);
    int ret = channel->send(s);
    return ret > 0;
}

bool server_files::send_data(const sp_channel &channel,int64 id, const string &msg)
{
    string s = set_files_binary(id,msg);
    int ret = channel->send(s);
    return ret > 0;
}

int64 server_files::make_fs_id()
{
    int64 id = make_tools::rand_num(100000000,999999999);
    string abs_path = _path_temp_save + std::to_string(id);

    if(swap_files::is_exists(abs_path) == false) return id;
    else return make_fs_id();
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
//                    std::unique_lock<mutex> lock(_mut_sjson);
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
    bool ok = _swap_fs.add_data_recv(id,data);
    if(ok == false) vlogw("err: task_recv_binary_data ");
}

void server_files::task_files_create_upload(const sp_channel &channel, const string &sjson)
{
    int64 time;
    int64 target;
    int64 source;
    int64 length_max;
    string filename;
    if(get_files_create_upload(sjson,time,length_max,filename))
    {
        //生成交换文件id
        int64 id = make_fs_id();
        vlogi("create file id: " << $(id));

        //创建文件写入
        channel->setContextPtr(std::make_shared<fs_data>(e_recv,id));
        string abs_path = _path_temp_save + std::to_string(id);
        bool ok = _swap_fs.open_file_recv(id,length_max,abs_path);
        if(ok == false) vloge("open file err" $(ok) $(id));

        //反馈数据
        string s = set_files_create_upload_back(time,id,true);
        send_msg(channel,s);
    }
    else vlogw("err: task_files_create_upload");

    vlogi("task_files_create_upload: " << $(time) $(target) $(source) $(length_max) $(filename));
}

void server_files::task_files_finish_upload(const sp_channel &channel, const string &sjson)
{
    int64 id;
    bool is_swap;
    bool finish;
    if(get_files_finish_upload(sjson,id,finish))
    {
        //判断文件大小是否相等
        bool recv_full = false;
        auto sp_recv = _swap_fs.find_fs_recv(id);
        if(sp_recv->count_recv == sp_recv->length_max) recv_full = true;

        //完成文件接收，关闭文件流
        _swap_fs.close_file_recv(id);

        //删除不完整文件
        bool is_recv_success = true;
        if(recv_full == false || finish == false)
        {
            swap_files::remove_file(_path_temp_save + std::to_string(id));
            is_recv_success = false;
        }

        //反馈数据
        string s = set_files_finish_upload_back(id,is_recv_success);
        send_msg(channel,s);

        vlogd("file finish: " << $(is_recv_success) $(recv_full) $(finish) $(sp_recv->count_recv) $(sp_recv->length_max));
    }
    else vlogw("err: task_files_finish_upload");

    vlogi("task_files_finish_upload: " << $(id) $(is_swap) $(finish) );
}

//bool qweb_files::send_data(int64 id, const string &msg)
//{
//    string s = set_files_binary(id,msg);
//    _wc.get_wc()->send(s);
//    return _wc.get_wc()->isConnected();
//}

//bool qweb_files::check_high_line()
//{
//    size_t size = _wc.get_wc()->channel->writeBufsize();
//    if(size >= WRITE_BUFSIZE_HIGH_WATER) return true;
//    return false;
//}

void server_files::task_files_create_download(const sp_channel &channel, const string &sjson)
{
    vlogi("task_files_create_download");

    //发送数据
    auto fn_send = [=](int64 id,const string &data){
        auto sp_channel = find_channel_send_map(id);
        bool ok = send_data(sp_channel,id,data);

        auto it = _swap_fs.find_fs_send(id);
        if(it->count_send % (1024*1024*2) == 0) return false;
        return ok;
    };

    int64 swap_id;
    if(get_files_create_download(sjson,swap_id))
    {
        //检查文件存在
        channel->setContextPtr(std::make_shared<fs_data>(e_send,swap_id));
        bool ret = false;
        int64 length_max = 0;
        string filename = std::to_string(swap_id);
        string path_file = _path_temp_save + filename;
        if(swap_files::is_exists(path_file))
        {
            length_max = swap_files::get_file_size(path_file);
            ret = _swap_fs.open_file_send(swap_id,path_file,fn_send,false);
            if(ret)
            {
                bool ok_add = add_send_map(swap_id,channel);
                vlogw($(ok_add));
            }
            else vlogw("open file err: " $(swap_id) $(ret));
        }

        //反馈数据
        string s = set_files_create_download_back(swap_id,length_max,filename,ret);
        send_msg(channel,s);

        vlogi("task_files_create_download" $(ret));
    }
}

void server_files::task_files_begin_download(const sp_channel &channel, const string &sjson)
{
    //解析json
    int64 swap_id;
    bool ok;
    if(get_files_begin_download(sjson,swap_id,ok))
    {
        if(ok)
        {
            bool is_finish = _swap_fs.add_data_send(swap_id);
            string s = set_files_finish_download(swap_id,is_finish);
            send_msg(channel,s);
            vlogi("send part:" $(swap_id) $(ok));

            if(is_finish)
            {
                vlogd("finish:" $(swap_id));
                _swap_fs.close_file_send(swap_id);
//                remove_send_map(swap_id);

            }
        }
        else _swap_fs.close_file_send(swap_id);
    }
}

void server_files::task_files_cancel_download(const sp_channel &channel, const string &sjson)
{
    vlogi("task_files_cancel_download");
}

bool server_files::add_send_map(int64 id, const sp_channel &channel)
{
    std::unique_lock<mutex> lock(_mut_send_map);
    auto it = _map_send.emplace(id,channel);
    return it.second;
}

void server_files::remove_send_map(int64 id)
{
    std::unique_lock<mutex> lock(_mut_send_map);
    _map_send.erase(id);
}

sp_channel server_files::find_channel_send_map(int64 id)
{
    auto it = _map_send.find(id);
    if(it != _map_send.end()) return it->second;
    return nullptr;
}

void server_files::push_send_que(int64 id)
{
    std::unique_lock<mutex> lock(_mut_send_que);
    _que_send.push(id);
}

bool server_files::pop_send_que(int64 &id)
{
    std::unique_lock<mutex> lock(_mut_send_que);
    if(_que_send.size() > 0)
    {
        id = _que_send.back();
        _que_send.pop();
        return true;
    }
    return false;
}







