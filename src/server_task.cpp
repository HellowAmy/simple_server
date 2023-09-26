#include "server_task.h"

#include "../util/Tvlog.h"

#define ADD_TASK(str,func) _map_task.emplace(str,std::bind(&server_task::func,this,_1,_2))

#define ERR_BACK(err)                               \
{                                                   \
    channel->send(set_error_info(err,err##_S));     \
    vlogw($(err##_S));                              \
}

#define ERR_BACK_S(err,sjson)                       \
{                                                   \
    channel->send(set_error_info(err,sjson));       \
    vlogw("err sjson: "$(sjson));                   \
}



server_task::server_task()
{
    fn_open = bind(&server_task::open,this,_1,_2);
    fn_message = bind(&server_task::message,this,_1,_2);
    fn_close = bind(&server_task::close,this,_1);

    ADD_TASK(login,task_login);
    ADD_TASK(swap_msg,task_swap);
    ADD_TASK(friends_list,task_friends_list);
}

server_task::~server_task()
{

}

void server_task::transmit_msg(const sp_channel &channel,const string &sjson)
{
    //检查json
    string stream;
    string type;
    if(check_json(sjson,stream,type))
    {
        if(stream == _cc_) { task_swap(channel,sjson); }
        else if(stream == _cs_)
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

void server_task::open(const sp_channel &channel, const sp_http &http)
{
    vlogi("open:" $(http->path));
    if(check_sjson_head(http->path) == false)
    {
        channel->close();
        vlogw("check path error:" $(http->path));
    }
}

void server_task::message(const sp_channel &channel, const string &msg)
{
    vlogi("message:" $(msg));
    transmit_msg(channel,msg);
}

void server_task::close(const sp_channel &channel)
{
    auto sp_account = channel->getContextPtr<int64>();
    move_connect_th(*sp_account);
    vlogi("close: " $(*sp_account));
}

bool server_task::check_sjson_head(string flg)
{
    if(flg == "/"+protocol::_head_) return true;
    return false;
}

bool server_task::init()
{
    if(_db_account.open_info() == false) return false;
    if(_db_friends.open_info() == false) return false;
    if(_db_info.open_info() == false) return false;
    if(_db_cache.open_cache() == false) return false;
    return true;
}



void server_task::add_connect_th(int64 account, const sp_channel &channel)
{
    std::unique_lock<mutex> lock(_mut_connect);
    _map_connect.emplace(account,channel);
}

void server_task::move_connect_th(int64 account)
{
    std::unique_lock<mutex> lock(_mut_connect);
    _map_connect.erase(account);
}

sp_channel server_task::find_connect_th(int64 account)
{
    std::unique_lock<mutex> lock(_mut_connect);
    auto it = _map_connect.find(account);
    if(it != _map_connect.end()) { return it->second; }
    return nullptr;
}

void server_task::task_login(const sp_channel &channel, const string &sjson)
{
    //解析json
    int64 account;
    string passwd;
    if(get_login(sjson,account,passwd))
    {
        //账号查库
        string passwd_db;
        if(_db_account.select_account(account,passwd_db))
        {
            //信息验证
            if(passwd_db == passwd)
            {
                //加入连接池
                channel->setContextPtr(std::make_shared<int64>(account));
                add_connect_th(account, channel);

                //反馈信息
                string s = set_login_back(true);
                channel->send(s);
                vlogd("task_login ok" $(account));
            }
            else ERR_BACK(CS_LOGIN_PASSWD_ERR);
        }
        else ERR_BACK(CS_LOGIN_NOT_AC);
    }
    else ERR_BACK_S(CS_ERR_PARSE_JSON,sjson);
}

void server_task::task_swap(const sp_channel &channel, const string &sjson)
{
    //解析json，获取目标账号
    int64 target;
    int64 source;
    if(check_swap(sjson,target,source))
    {
        //成功，直接转发
        auto sp = find_connect_th(target);
        if(sp != nullptr) sp->send(sjson);
        else
        {
            //失败，加入失败记录，离线发送
            if(_db_cache.insert_cache(target,sjson))
            { ERR_BACK(CS_ERR_SWAP_SJSON); }
        }
    }
    else ERR_BACK_S(CS_ERR_PARSE_JSON,sjson);
}

void server_task::task_friends_list(const sp_channel &channel, const string &sjson)
{
    //解析json
    int64 account;
    if(get_friends_list(sjson,account))
    {
        //查好友库
        vector<string> vec_friends;
        if(_db_friends.select_friends(account,vec_friends))
        {
            //反馈信息
            string scev = set_json_vec(vec_friends);
            channel->send(set_friends_list_back(scev,true));
        }
        else ERR_BACK(CS_ERR_SELECT_DATA);
    }
    else ERR_BACK_S(CS_ERR_PARSE_JSON,sjson);
}

void server_task::task_friends_status(const sp_channel &channel, const string &sjson)
{
    //解析json
    int64 account;
    if(get_friends_status(sjson,account))
    {
        //查信息库
        sqlite_info::data fdata;
        if(_db_info.select_info(account,fdata))
        {
            //查在线列表
            bool online = true;
            if(find_connect_th(account) == nullptr) online = false;

            //反馈信息
            string s = set_friends_status_back(account,fdata.nickname,fdata.icon,online,true);
            channel->send(s);
        }
        else ERR_BACK(CS_ERR_SELECT_DATA);
    }
    else ERR_BACK_S(CS_ERR_PARSE_JSON,sjson);
}




