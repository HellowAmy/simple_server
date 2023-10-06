
#include "server_files.h"

#include "../util/Tvlog.h"

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
    bool ok = _fs_swap.add_id_channel(channel->id());
    vlogif(ok,$(ok));

//    channel->setContextPtr(std::make_shared<fs_swap_id>(std::set<int64>(),std::set<int64>()));

    channel->onwrite = [=](hv::Buffer* buf){
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


    auto it = _fs_swap.get_channel_map();
    vlogi($(it.size()));
    for(auto a:it)
    {
        shared_ptr<files_channel::fs_channel> sp = a.second;
        vlogi($(sp->_set_recv.size()) $(sp->_set_send.size()));
    }

    //连接意外断开，清空发送与接收流
    _fs_swap.remove_id_channel(channel->id());

//    close_file_channel(channel);
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
    if(ret <= 0) vloge("send_msg failed:" << $(sjson));
    return ret > 0;
}

bool server_files::send_data(const sp_channel &channel,int64 id, const string &msg)
{
    string s = set_files_binary(id,msg);
    int ret = channel->send(s);
    if(ret <= 0) vloge("send_data failed:" << $(msg));
    return ret > 0;
}

int64 server_files::make_fs_id()
{
    int64 id = make_tools::rand_num(100000000,999999999);
    string abs_path = _path_temp_save + std::to_string(id);

    if(files_info::is_exists(abs_path) == false) return id;
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
    bool ok = _fs_swap.add_data_recv(id,data);
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

        //创建文件写入
        string abs_path = _path_temp_save + std::to_string(id);
        bool ok = _fs_swap.open_file_recv_channel(channel->id(),id,length_max,abs_path);
//        bool ok = _swap_fs.open_file_recv(id,length_max,abs_path);
//        if(ok)
//        {
//
////            auto set_id = channel->getContextPtr<fs_swap_id>();
////            set_id->_set_recv.emplace(id);
//        }
        vlogif(ok,$(ok) $(id));

        //反馈数据
        string s = set_files_create_upload_back(time,id,true);
        send_msg(channel,s);
    }
    else vlogw("err: task_files_create_upload" << $(time) $(target) $(source) $(length_max) $(filename));
}

void server_files::task_files_finish_upload(const sp_channel &channel, const string &sjson)
{
    int64 id;
    bool is_swap;
    bool finish;
    if(get_files_finish_upload(sjson,id,finish))
    {
        //判断文件大小是否相等
        bool recv_full = _fs_swap.check_length_recv(id);

//        bool recv_full = false;
//        auto sp_recv = _fs_swap.find_fs_recv(id);
//        if(sp_recv->count_recv == sp_recv->length_max) recv_full = true;

        //完成文件接收，关闭文件流
        _fs_swap.close_file_recv_channel(channel->id(),id);

//        close_file_recv(channel,id);
//        _swap_fs.close_file_recv(id);

        //删除不完整文件
        bool is_recv_success = true;
        if(recv_full == false || finish == false)
        {
            files_info::remove_file(_path_temp_save + std::to_string(id));
            is_recv_success = false;
            vlogw("upload file failed: " << $(id));
        }
        else vlogd("upload file: " << $(id) $(is_recv_success));

        //反馈数据
        string s = set_files_finish_upload_back(id,is_recv_success);
        send_msg(channel,s);
    }
    else vlogw("err: task_files_finish_upload");
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
    //发送数据
    auto fn_send = [=](int64 id,const string &data){
        return _fs_swap.check_linit_flux(send_data(channel,id,data),id);
    };

    int64 swap_id;
    if(get_files_create_download(sjson,swap_id))
    {
        //检查文件存在
        bool ok = false;
        int64 length_max = 0;
        string filename = std::to_string(swap_id);
        string path_file = _path_temp_save + filename;
        if(files_info::is_exists(path_file))
        {
            length_max = files_info::get_file_size(path_file);
            ok = _fs_swap.open_file_send_channel(channel->id(), swap_id,path_file,fn_send);

            vlogif(ok, $(channel->id()) $(swap_id) $(ok));

//            if(ok)
//            {
////                auto set_id = channel->getContextPtr<fs_swap_id>();
////                set_id->_set_send.emplace(swap_id);
////                bool ok_add = add_send_map(swap_id,channel);
//                vlogd("open file:" $(swap_id) $(ok));
//            }
//            else vlogw("open file err: " $(swap_id) $(ok));
        }
        else vlogw("err: not exists" << $(path_file));

        //反馈数据
        string s = set_files_create_download_back(swap_id,length_max,filename,ok);
        send_msg(channel,s);
    }
    else vlogw("err: task_files_create_download");
}

void server_files::task_files_begin_download(const sp_channel &channel, const string &sjson)
{
    //解析json
    int64 id;
    bool ok;
    if(get_files_begin_download(sjson, id, ok))
    {
        if(ok)
        {
//            _fs_swap.add_send_queue()
//            _fs_swap.start_send_queue();
            bool is_finish = _fs_swap.add_data_send(id);// add_data_send_limit_flux(id);
            string s = set_files_finish_download(id, is_finish);
            send_msg(channel,s);
            vlogi("send part:" $(id) $(ok));

            if(is_finish)
            {
                vlogd("finish:" $(id));
                _fs_swap.close_file_send_channel(channel->id(),id);


//                close_file_send(channel,id);

//                _swap_fs.close_file_send(id);
//                remove_send_map(id);
            }
        }
        else
        {
            _fs_swap.close_file_send_channel(channel->id(),id);
//            _fs_swap.remove_file_send(channel->id(), id);

//            close_file_send(channel,id);
//            _swap_fs.close_file_send(id);
//            remove_send_map(id);
            vlogw("task_files_begin_download failed: " $(id) $(ok));
        }
    }
    else vlogw("err: task_files_begin_download");
}

void server_files::task_files_cancel_download(const sp_channel &channel, const string &sjson)
{
    vlogi("task_files_cancel_download");
}

//void server_files::close_file_send(const sp_channel &channel,int64 id)
//{
//    _swap_fs.close_file_send(id);
//    auto set_id = channel->getContextPtr<fs_swap_id>();
//    set_id->_set_send.erase(id);
//}
//
//void server_files::close_file_recv(const sp_channel &channel,int64 id)
//{
//    _swap_fs.close_file_recv(id);
//    auto set_id = channel->getContextPtr<fs_swap_id>();
//    set_id->_set_recv.erase(id);
//}
//
//void server_files::close_file_channel(const sp_channel &channel)
//{
//    auto set_id = channel->getContextPtr<fs_swap_id>();
//    for(auto id:set_id->_set_recv)
//    { _swap_fs.close_file_recv(id); }
//    for(auto id:set_id->_set_send)
//    { _swap_fs.close_file_send(id); }
//}

//bool server_files::add_send_map(int64 id, const sp_channel &channel)
//{
//    std::unique_lock<mutex> lock(_mut_send_map);
//    auto it = _map_send.emplace(id,channel);
//    return it.second;
//}
//
//void server_files::remove_send_map(int64 id)
//{
//    std::unique_lock<mutex> lock(_mut_send_map);
//    _map_send.erase(id);
//}
//
//sp_channel server_files::find_channel_send_map(int64 id)
//{
//    auto it = _map_send.find(id);
//    if(it != _map_send.end()) return it->second;
//    return nullptr;
//}

//void server_files::push_send_que(int64 id)
//{
//    std::unique_lock<mutex> lock(_mut_send_que);
//    _que_send.push(id);
//}
//
//bool server_files::pop_send_que(int64 &id)
//{
//    std::unique_lock<mutex> lock(_mut_send_que);
//    if(_que_send.size() > 0)
//    {
//        id = _que_send.back();
//        _que_send.pop();
//        return true;
//    }
//    return false;
//}







