
#include "server_files.h"

#include "../util/Tvlog.h"

#define ADD_TASK(str) _map_task.emplace(str,std::bind(&server_files::task_##str,this,_1,_2))


#define ERR_BACK_S(err,sjson)                                       \
{                                                                   \
    channel->send(set_files_json(set_error_info(err,sjson)));       \
    vlogw("err sjson: "$(sjson));                                   \
}

server_files::server_files()
{
    _path_temp = "../temp_file/";

    fn_open = bind(&server_files::open,this,_1,_2);
    fn_message = bind(&server_files::message,this,_1,_2);
    fn_close = bind(&server_files::close,this,_1);

    ADD_TASK(files_create_upload);
    ADD_TASK(files_begin_upload);
    ADD_TASK(files_finish_upload);

    ADD_TASK(files_create_download);
    ADD_TASK(files_begin_download);
    ADD_TASK(files_finish_download);
}

void server_files::open(const sp_channel &channel, const sp_http &http)
{
    bool ok = _fs_swap.add_id_channel(channel->id());
    vlogif(ok,$(ok));

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
    string abs_path = _path_temp + std::to_string(id);

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
                vlogi("transmit_msg:" << $(type) $(stream));
                if(stream == _cs_)
                {
                    //执行绑定函数
                    auto it = _map_task.find(type);
                    if(it != _map_task.end()) it->second(channel,sjson);
                    else vlogw("not find task func" $(stream) $(type));
                }
                else vlogw("not find stream type" $(stream) $(type));
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
    int64 length_file;
    string abs_file;
    string save_file;
    if(get_files_create_upload(sjson,length_file,abs_file,save_file))
    {
        //生成交换文件id
        int64 id = make_fs_id();

        //创建文件写入
        string filename;
        if(save_file == "") filename = _path_temp + std::to_string(id);
        else filename = save_file;

        bool ok = _fs_swap.open_file_recv_channel(channel->id(),id,length_file,filename);
        vlogif(ok,$(ok) $(id) $(length_file) $(abs_file));

        //反馈数据
        string s = set_files_create_upload_back(id,abs_file,ok);
        send_msg(channel,s);

        vlogd("create_upload:" $(id));
    }
    else vlogw("err: task_files_create_upload" << $(time) $(length_file) $(abs_file));
}

void server_files::task_files_begin_upload(const sp_channel &channel, const string &sjson)
{
    int64 id;
    if(get_files_begin_upload(sjson,id))
    {
        //反馈数据
        string s = set_files_begin_upload_back(id);
        send_msg(channel,s);
    }
    else vlogw("task_files_begin_upload");
}

void server_files::task_files_finish_upload(const sp_channel &channel, const string &sjson)
{
    int64 id;
    bool finish;
    if(get_files_finish_upload(sjson,id,finish))
    {
        //判断文件大小是否相等
        int64 count;
        int64 length;
        bool recv_full = _fs_swap.check_length_recv(id,count,length);

        //完成文件接收，关闭文件流
        _fs_swap.close_file_recv_channel(channel->id(),id);

        //删除不完整文件
        bool is_recv_success = true;
        if(recv_full == false || finish == false)
        {
            auto it = _fs_swap.find_fs_send(id);
            if(it != nullptr) { files_info::remove_file(it->filename); }
            is_recv_success = false;
            vlogw("task_files_finish_upload not success:" $(is_recv_success));
        }

        //反馈数据
        string s = set_files_finish_upload_back(id,is_recv_success);
        send_msg(channel,s);

        vlogd("finish_upload:" $(id) $(is_recv_success));
    }
    else vlogw("err: task_files_finish_upload");
}

void server_files::task_files_create_download(const sp_channel &channel, const string &sjson)
{
    //发送数据
    auto fn_send = [=](int64 id,const string &data){
        return _fs_swap.check_linit_flux(send_data(channel,id,data),id);
    };

    string abs_path;
    string save_path;
    if(get_files_create_download(sjson,abs_path,save_path))
    {
        //检查文件存在
        bool ok = false;
        int64 length_max = 0;
        int64 id = make_fs_id();

        if(files_info::is_exists(abs_path))
        {
            length_max = files_info::get_file_size(abs_path);
            ok = _fs_swap.open_file_send_channel(channel->id(),id,abs_path,fn_send);
        }
        else vlogw("task_files_create_download not exists" $(abs_path));

        //反馈数据
        string s = set_files_create_download_back(id,length_max,abs_path,save_path,ok);
        send_msg(channel,s);

        vlogd("create_download:" $(id));
    }
    else vlogw("err: task_files_create_download");
}

void server_files::task_files_begin_download(const sp_channel &channel, const string &sjson)
{
    int64 id;
    if(get_files_begin_download(sjson,id))
    {
        //发送数据
        bool ok_send = _fs_swap.add_data_send(id);
        if(ok_send)
        {
            string s = set_files_finish_download_back(id,true);
            send_msg(channel,s);
        }
        else
        {
            string s = set_files_begin_download_back(id);
            send_msg(channel,s);
        }
    }
    else vlogw("task_files_begin_download");
}

void server_files::task_files_finish_download(const sp_channel &channel, const string &sjson)
{
    int64 id;
    bool ok;
    if(get_files_finish_download_back(sjson,id,ok))
    {
        //完成文件接收，关闭文件流
        _fs_swap.close_file_send_channel(channel->id(),id);

        vlogd("finish_download:" $(id) $(ok));
    }
    else vlogw("get_files_finish_download");
}








