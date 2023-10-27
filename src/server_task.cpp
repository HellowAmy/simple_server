#include "server_task.h"

#include "../util/Tvlog.h"

#define ADD_TASK(func) _map_task.emplace(func,std::bind(&server_task::task_##func,this,_1,_2))

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

#define UPDATE_AC_INFO(ac,field,ok) \
    ok = _db_info->update_db(_db_info->get_table(),_db_info->get_data().ac,ac,_db_info->get_data().field,field)


server_task::server_task()
{
    fn_open = bind(&server_task::open,this,_1,_2);
    fn_message = bind(&server_task::message,this,_1,_2);
    fn_close = bind(&server_task::close,this,_1);

    ADD_TASK(swap_msg);
    ADD_TASK(swap_cache);
    ADD_TASK(ac_login);
    ADD_TASK(ac_register);
//    ADD_TASK(ac_info);
//    ADD_TASK(ac_info_remarks);
    ADD_TASK(ac_info_all);
    ADD_TASK(ac_update_info);
    ADD_TASK(ac_update_remarks);
    ADD_TASK(ac_update_friends);

//    ADD_TASK(friends_list);
//    ADD_TASK(friends_status);



}

server_task::~server_task()
{

}

bool server_task::passwd_verify(string passwd_db, string passwd)
{
    if(passwd_db == passwd) return true;
    return false;
}

void server_task::transmit_msg(const sp_channel &channel,const string &sjson)
{
    //检查json
    string stream;
    string type;
    if(check_json(sjson,stream,type))
    {
        if(stream == _cc_) { task_swap_msg(channel,sjson); }
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
    vlogd("close: " $(*sp_account));
}

bool server_task::check_sjson_head(string flg)
{
    if(flg == "/"+protocol::_head_) return true;
    return false;
}

void server_task::init_data(sqlite_account *account,sqlite_friends *friends,sqlite_info *info,sqlite_cache *cache)
{
    _db_account    = account;
    _db_friends    = friends;
    _db_info       = info;
    _db_cache      = cache;
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

int64 server_task::insert_account(string passwd,int count)
{
    int64 account = make_tools::rand_num();
    bool ok = _db_account->insert_account(account,passwd);
    if(ok) return account;
    else if(count <= 0) return 0;
    else return insert_account(passwd,count - 1);
}

sp_channel server_task::find_connect_th(int64 account)
{
    std::unique_lock<mutex> lock(_mut_connect);
    auto it = _map_connect.find(account);
    if(it != _map_connect.end()) { return it->second; }
    return nullptr;
}

void server_task::task_ac_login(const sp_channel &channel, const string &sjson)
{
    //解析json
    int64 account;
    string passwd;
    if(get_ac_login(sjson,account,passwd))
    {
        //账号查库
        string passwd_db;
        if(_db_account->select_account(account,passwd_db))
        {
            //密码验证
            if(passwd_verify(passwd_db,passwd))
            {
                //加入连接池
                channel->setContextPtr(std::make_shared<int64>(account));
                add_connect_th(account, channel);

                //获取好友列表
                vector<int64> vec_friends_ac;
                {
                    //查询账号好友
                    vector<string> vec_friends;
                    vector<map<string,string>> vec_line;
                    if(_db_friends->select_friends(account,vec_line))
                    {
                        for(auto a:vec_line)
                        {
                            for(auto b:a)
                            {
                                if(b.first == _db_friends->get_data().friends)
                                { vec_friends.push_back(b.second); }
                            }
                        }
                    }
                    else ERR_BACK(CS_ERR_SELECT_DATA);
                    vec_stoi(vec_friends,vec_friends_ac);
                }

                //组合好友信息到容器
                vector<string> vec_friends_info;
                for(auto ac_fr : vec_friends_ac)
                {
                    //获取好友在线
                    bool online = true;
                    if(find_connect_th(ac_fr) == nullptr) online = false;

                    //获取好友备注
                    string remarks;
                    if(_db_friends->select_remarks(account,ac_fr,remarks) == false)
                    { vlogw("select_remarks failed"); }

                    //获取好友信息
                    sqlite_info::data fdata;
                    if(_db_info->select_info(ac_fr,fdata) == false)
                    { ERR_BACK(CS_ERR_SELECT_DATA); }

                    //组合好友信息
                    vec_friends_info.push_back(set_friends_info_json(ac_fr,fdata.nickname,fdata.icon,remarks,online));
                }

                //获取账号信息
                sqlite_info::data fdata;
                if(_db_info->select_info(account,fdata))
                {
                    //容器转json格式
                    string svec_friends_info = set_json_vec(vec_friends_info);
                    string ac_nickname = fdata.nickname;
                    string ac_icon = fdata.icon;

                    //反馈信息
                    string s = set_ac_login_back(account,ac_nickname,ac_icon,svec_friends_info);
                    channel->send(s);
                }
                else { ERR_BACK(CS_ERR_SELECT_DATA); }
            }
            else ERR_BACK(CS_LOGIN_PASSWD_ERR);
        }
        else ERR_BACK(CS_LOGIN_NOT_AC);
    }
    else ERR_BACK_S(CS_ERR_PARSE_JSON,sjson);
}

void server_task::task_swap_msg(const sp_channel &channel, const string &sjson)
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
            if(_db_cache->insert_cache(target,sjson) == false)
            { ERR_BACK(CS_ERR_SWAP_SJSON); }
        }
    }
    else ERR_BACK_S(CS_ERR_PARSE_JSON,sjson);
}

//void server_task::task_friends_list(const sp_channel &channel, const string &sjson)
//{
//    //解析json
//    int64 account;
//    if(get_friends_list(sjson,account))
//    {
//        //查好友库
//        vector<string> vec_friends;
//        vector<map<string,string>> vec_line;
//        if(_db_friends->select_friends(account,vec_line))
//        {
//            for(auto a:vec_line)
//            {
//                for(auto b:a)
//                {
//                    if(b.first == _db_friends->get_data().friends)
//                    { vec_friends.push_back(b.second); }
//                }
//            }
//
//            //反馈信息
//            string scev = set_json_vec(vec_friends);
//            channel->send(set_friends_list_back(account,scev,true));
//        }
//        else ERR_BACK(CS_ERR_SELECT_DATA);
//    }
//    else ERR_BACK_S(CS_ERR_PARSE_JSON,sjson);
//}
//
//void server_task::task_friends_status(const sp_channel &channel, const string &sjson)
//{
//    //解析json
//    int64 account;
//    string svec_ac_fs;
//    if(get_friends_status(sjson,account,svec_ac_fs))
//    {
//        //转类型
//        vector<string> vec_ac = get_json_vec(svec_ac_fs);
//        vector<int64> vec_aci;
//        bool ok = vec_stoi(vec_ac,vec_aci);
//
//        //组合好友信息
//        vector<string> vec_ac_info;
//        for(auto ac:vec_aci)
//        {
//            //查好友备注
//            string remarks;
//            if(_db_friends->select_remarks(account,ac,remarks) == false)
//            { vlogw("select_remarks failed"); }
//
//            //查信息库
//            sqlite_info::data fdata;
//            if(_db_info->select_info(ac,fdata))
//            {
//                //查在线列表
//                bool online = true;
//                if(find_connect_th(ac) == nullptr) online = false;
////                vec_ac_info.push_back(set_ac_info_json(ac,fdata.nickname,fdata.icon,remarks,online));
//            }
//            else ERR_BACK(CS_ERR_SELECT_DATA);
//        }
//
//        //反馈信息
//        string svec = set_json_vec(vec_ac_info); //生成svec的json格式字符串
//        string s = set_friends_status_back(account,svec,ok);
//        channel->send(s);
//    }
//    else ERR_BACK_S(CS_ERR_PARSE_JSON,sjson);
//}

void server_task::task_ac_register(const sp_channel &channel, const string &sjson)
{
    //解析json
    int64 phone;
    int64 age;
    int64 sex;
    string nickname;
    string location;
    string passwd;
    if(get_ac_register(sjson,phone,age,sex,nickname,location,passwd))
    {
        //生成账号并加入账号
        bool ok = false;
        int64 account  = insert_account(passwd);
        if(account != 0)
        {
            //附加信息
            bool ok_info = _db_info->insert_info({account,phone,age,sex,nickname,location,"icon_default.png"});
            if(ok_info) ok = true;
        }
        else vlogw("err: task_ac_register insert account failed");

        //反馈信息
        string s = set_ac_register_back(account,passwd,ok);
        channel->send(s);
    }
    else ERR_BACK_S(CS_ERR_PARSE_JSON,sjson);
}

void server_task::task_swap_cache(const sp_channel &channel, const string &sjson)
{
    //解析json
    int64 target;
    if(get_swap_cache(sjson,target))
    {
        //获取暂存信息
        vector<string> data;
        if(_db_cache->select_cache(target,data))
        {
            //移除暂存信息
            _db_cache->delete_cache(target);

            //反馈信息
            string svec = set_json_vec(data);
            string s = set_swap_cache_back(svec);
            channel->send(s);
        }
    }
    else ERR_BACK_S(CS_ERR_PARSE_JSON,sjson);
}

//void server_task::task_ac_info(const sp_channel &channel, const string &sjson)
//{
//    //解析json
//    int64 account;
//    if(get_ac_info(sjson,account))
//    {
//        //查信息库
//        sqlite_info::data fdata;
//        bool ok = _db_info->select_info(account,fdata);
//
//        //反馈信息
//        string s = set_ac_info_back(account,fdata.nickname,fdata.icon,ok);
//        channel->send(s);
//    }
//    else ERR_BACK_S(CS_ERR_PARSE_JSON,sjson);
//}

void server_task::task_ac_update_info(const sp_channel &channel, const string &sjson)
{
    int64 account;
    int64 phone;
    int64 age;
    int64 sex;
    string nickname;
    string location;
    string icon;
    if(get_ac_update_info(sjson,account,phone,age,sex,nickname,location,icon))
    {
        //修改信息
        bool ok1 = true;
        bool ok2 = true;
        bool ok3 = true;
        bool ok4 = true;
        bool ok5 = true;
        bool ok6 = true;
        if(phone != -1)     { UPDATE_AC_INFO(account,phone,ok1); }
        if(age != -1)       { UPDATE_AC_INFO(account,age,ok2); }
        if(sex != -1)       { UPDATE_AC_INFO(account,sex,ok3); }
        if(nickname != "")  { UPDATE_AC_INFO(account,nickname,ok4); }
        if(location != "")  { UPDATE_AC_INFO(account,location,ok5); }
        if(icon != "")      { UPDATE_AC_INFO(account,icon,ok6); }

        //反馈信息
        bool ok = (ok1 && ok2 && ok3 && ok4 && ok5 && ok6);
        string s = set_ac_update_info_back(account,icon,ok);
        channel->send(s);

        vlogi("update info:"<< $(account) $(ok) $(phone) $(age) $(sex) $(nickname) $(location) $(icon) );
    }
    else ERR_BACK_S(CS_ERR_PARSE_JSON,sjson);
}

void server_task::task_ac_update_remarks(const sp_channel &channel, const string &sjson)
{
    int64 account;
    int64 friends;
    string remarks;
    if(get_ac_update_remarks(sjson,account,friends,remarks))
    {
        if(_db_friends->update_remarks(account,friends,remarks))
        {
            string s = set_ac_update_remarks_back(account,friends,remarks);
            channel->send(s);
        }
    }
    else ERR_BACK_S(CS_ERR_PARSE_JSON,sjson);
}

void server_task::task_ac_info_all(const sp_channel &channel, const string &sjson)
{
    //解析json
    int64 types;
    int64 account;
    if(get_ac_info_all(sjson,types,account))
    {
        //查信息库
        sqlite_info::data fdata;
        bool ok = _db_info->select_info(account,fdata);
        if(ok)
        {
            try {
                int64 phone = std::stoll(fdata.phone);
                int64 age = std::stoll(fdata.age);
                int64 sex = std::stoll(fdata.sex);
                string nickname = fdata.nickname;
                string location = fdata.location;
                string icon = fdata.icon;

                //反馈信息
                string s = set_ac_info_all_back(types,account,phone,age,sex,nickname,location,icon);
                channel->send(s);
            }
            catch(...) {}
        }
    }
    else ERR_BACK_S(CS_ERR_PARSE_JSON,sjson);
}

//void server_task::task_ac_info_remarks(const sp_channel &channel, const string &sjson)
//{
//    //解析json
//    int64 account;
//    int64 friends;
//    if(get_ac_info_remarks(sjson,account,friends))
//    {
//        string remarks;
//        if(_db_friends->select_remarks(account,friends,remarks))
//        {
//            //反馈信息
//            string s = set_ac_info_remarks_back(friends,remarks);
//            channel->send(s);
//        }
//        else vlogw("select_remarks failed");
//    }
//    else ERR_BACK_S(CS_ERR_PARSE_JSON,sjson);
//}



void server_task::task_ac_update_friends(const sp_channel &channel, const string &sjson)
{
    //获取好友列表信息
    auto fn_info = [=](int64 account) ->string {
        //获取好友在线
        bool online = true;
        if(find_connect_th(account) == nullptr) online = false;

        //获取好友备注
        string remarks;
        if(_db_friends->select_remarks(account,account,remarks) == false)
        { vlogw("select_remarks failed"); }

        //获取好友信息
        sqlite_info::data fdata;
        if(_db_info->select_info(account,fdata) == false)
        { ERR_BACK(CS_ERR_SELECT_DATA); }

        return set_friends_info_json(account,fdata.nickname,fdata.icon,remarks,online);
    };

    //解析json
    int64 account;
    int64 friends;
    string ac_remarks;
    string fr_remarks;
    if(get_ac_update_friends(sjson,account,friends,ac_remarks,fr_remarks))
    {
        //加入双方好友列表
        _db_friends->insert_ac_both_friend(account,friends,ac_remarks,fr_remarks);

        //反馈信息 -- 接收方
        {
            string s = set_ac_update_friends_back(fn_info(friends));
            channel->send(s);
        }

        //反馈信息 -- 发送方
        {
            string s = set_ac_update_friends_back(fn_info(account));
            auto it = find_connect_th(account);
            if(it != nullptr) it->send(s);
        }
    }
    else ERR_BACK_S(CS_ERR_PARSE_JSON,sjson);
}







