#include <iostream>

#include "../../simple_chat/web_protocol.h"
#include "../util/Tvlog.h"
#include "make_tools.h"
#include "inter_server.h"
#include "server_task.h"
#include "sqlite_op.h"

using namespace std;


//网络连接
void test_1()
{
    vlogd("== server ==");

    inter_server sk;

    function fn_open = [](const sp_channel&, const sp_http& http){
        HttpRequest* p = http.get();
        vlogi("fn_open"<<p->path);
    };
    function fn_message = [&](const sp_channel& channe, const string& msg){
        vlogi("fn_message: " << msg);
        hv::WebSocketChannel* p = channe.get();
        p->send(msg);
        if(msg == "exit") sk.get_sv()->stop();
        else if(msg == "close") p->close();
    };
    function fn_close = [&](const sp_channel&){
        vlogi("fn_close");
    };

    sk.func_bind(fn_open,fn_message,fn_close);
    sk.open();

    vloge("== end ==");
}

//账号密码
void test_2()
{

    sqlite_account sql;

    {
        bool ok = sql.open_info();
        (ok == true ? vlogd($(ok)) : vloge($(ok)));
    }
    {
        bool ok = sql.create_table();
        (ok == true ? vlogd($(ok)) : vloge($(ok)));
    }

    {
        bool ok = sql.insert_account(123456789,"123456");
        (ok == true ? vlogd($(ok)) : vloge($(ok)));
    }
    {
        bool ok = sql.insert_account(333456789,"1234567");
        (ok == true ? vlogd($(ok)) : vloge($(ok)));
    }
    {
        bool ok = sql.insert_account(444456789,"1234568");
        (ok == true ? vlogd($(ok)) : vloge($(ok)));
    }
    {
        bool ok = sql.insert_account(444456789,"123456");
        (ok == true ? vlogd($(ok)) : vloge($(ok)));
    }
    {
        bool ok = sql.insert_account(544456789,"12345");
        (ok == true ? vlogd($(ok)) : vloge($(ok)));
    }

    {
        int value = sql.count_db(sql.get_table());
        vlogd($(value));
    }
    {
        vector<string> data;
        bool ok = sql.select_db(sql.get_table(),data);
        (ok == true ? vlogd($(ok)) : vloge($(ok)));
        for(auto a:data)
        {
            vlogi(a);
        }
    }

    {
        bool ok = sql.update_account(123456789,"aabbccdd");
        (ok == true ? vlogd($(ok)) : vloge($(ok)));
    }
    {
        bool ok = sql.delete_account(333456789);
        (ok == true ? vlogd($(ok)) : vloge($(ok)));
    }

    {
        int value = sql.count_db(sql.get_table());
        vlogd($(value));
    }
    {
        vector<string> data;
        bool ok = sql.select_db(sql.get_table(),data);
        (ok == true ? vlogd($(ok)) : vloge($(ok)));
        for(auto a:data)
        {
            vlogi(a);
        }
    }
    {
        bool ok = sql.drop_db(sql.get_table());
        (ok == true ? vlogd($(ok)) : vloge($(ok)));
    }
    {
        bool ok = sql.close_db();
        (ok == true ? vlogd($(ok)) : vloge($(ok)));
    }
}

//随即账号
void test_3()
{
    sqlite_account sql;
    {
        {
            bool ok = sql.open_info();
            (ok == true ? vlogd($(ok)) : vloge($(ok)));
        }
        {
            vector<string> data;
            bool ok = sql.select_db(sql.get_table(),data);
            (ok == true ? vlogd($(ok)) : vloge($(ok)));
            for(auto a:data)
            {
                vlogi(a);
            }
        }
        {
            bool ok = sql.close_db();
            (ok == true ? vlogd($(ok)) : vloge($(ok)));
        }
        return ;
    }


    {
        bool ok = sql.open_info();
        (ok == true ? vlogd($(ok)) : vloge($(ok)));
    }
    {
        bool ok = sql.create_table();
        (ok == true ? vlogd($(ok)) : vloge($(ok)));
    }

    for(int i=0;i<100000;i++)
    {
        unsigned  int num = make_tools::rand_num();
        bool ok = sql.insert_account(num,"123456");
        if(ok == false) vloge($(num) $(i));
    }

    {
        int value = sql.count_db(sql.get_table());
        vloge($(value));
    }
//    {
//        bool ok = sql.drop_db(sql.get_table());
//        (ok == true ? vlogd($(ok)) : vloge($(ok)));
//    }
    {
        bool ok = sql.close_db();
        (ok == true ? vlogd($(ok)) : vloge($(ok)));
    }
}

//测试输入
void test_4()
{
    using namespace protocol;

    string sjson = set_login(123456789,"abc");
    vlogi($(sjson));

    int64 v1;
    string v2;
    get_login(sjson,v1,v2);
    vlogi($(v1) $(v2));
    vlogi($(login));
    vlogi($(login_back));
}

//网络转发
void test_5()
{
    vlogd("== server ==");

    server_task t;
    inter_server s;
    s.func_bind(t.fn_open,t.fn_message,t.fn_close);
    if(t.init()) s.open();

    vlogd("== server end ==");
}

//缓存测试
void test_6()
{
    sqlite_cache sql;
    {
        bool ok = sql.open_cache();
        (ok == true ? vlogd($(ok)) : vloge($(ok)));
    }
    {
        bool ok = sql.create_cache();
        (ok == true ? vlogd($(ok)) : vloge($(ok)));
    }
    {
        for(int i=0;i<100;i++)
        {
            bool ok = sql.insert_cache(i,"123456789");
            if(ok == false) vloge($(i));
        }
    }
    {
        vector<string> data;
        bool ok = sql.select_db(sql.get_table(),data);
        (ok == true ? vlogd($(ok)) : vloge($(ok)));
        for(auto a:data)
        {
            vlogd($(a));
        }
    }
    {
        int num = sql.count_db(sql.get_table());
        vlogd($(num));
    }
    {
        bool ok = sql.delete_cache(90);
        (ok == true ? vlogd($(ok)) : vloge($(ok)));
    }
    {
        int num = sql.count_db(sql.get_table());
        vlogd($(num));
    }
    {
        bool ok = sql.drop_db(sql.get_table());
        (ok == true ? vlogd($(ok)) : vloge($(ok)));
    }
    {
        bool ok = sql.close_db();
        (ok == true ? vlogd($(ok)) : vloge($(ok)));
    }
}

//外键约束
void test_7()
{
    sqlite_account sql;
    sqlite_friends sqlf;
    sqlite_info sqli;
    {
        bool ok = sql.open_info();
        (ok == true ? vlogd($(ok)) : vloge($(ok)));
    }
    {
        bool ok = sql.create_table();
        (ok == true ? vlogd($(ok)) : vloge($(ok)));
    }
    {
        bool ok = sqlf.open_info();
        (ok == true ? vlogd($(ok)) : vloge($(ok)));
    }
    {
        bool ok = sqlf.create_table();
        (ok == true ? vlogd($(ok)) : vloge($(ok)));
        vlogd($(sqlf.get_error()) $(sqlf.get_error_exec()));
    }
    {
        bool ok = sqli.open_info();
        (ok == true ? vlogd($(ok)) : vloge($(ok)));
    }
    {
        bool ok = sqli.create_table();
        (ok == true ? vlogd($(ok)) : vloge($(ok)));
        vlogd($(sqlf.get_error()) $(sqlf.get_error_exec()));
    }

    {
        int v = 100000000;
        for(int i=v;i<v+10;i++)
        {
            bool ok = sql.insert_account(i,"123456");
            (ok == true ? vlogd($(ok)) : vloge($(ok)));
        }

    }
    {
        vector<string> data;
        bool ok = sql.select_db(sql.get_table(),data);
        (ok == true ? vlogd($(ok)) : vloge($(ok)));
        for(auto a:data)
        {
            vlogi(a);
        }
    }

    {
        int v = 100000000;
        for(int i=v;i<v+5;i++)
        {
            for(int j=i;j<i+5;j++)
            {
                bool ok = sqlf.insert_friends(i,j);
                (ok == true ? vlogd($(ok)) : vloge($(ok)));
            }
        }
    }
    {
        vector<string> data;
        bool ok = sqlf.select_db(sqlf.get_table(),data);
        (ok == true ? vlogd($(ok)) : vloge($(ok)));
        for(auto a:data)
        {
            vlogi(a);
        }
    }

    {
        vector<string> data;
        bool ok = sqlf.select_friends(100000003,data);
        (ok == true ? vlogd($(ok)) : vloge($(ok)));
        for(auto a:data)
        {
            vlogi(a);
        }
    }

    {
        bool ok = sqlf.delete_friends(100000003,100000004);
        (ok == true ? vlogd($(ok)) : vloge($(ok)));
        vlogd($(sqlf.get_error()) $(sqlf.get_error_exec()));
    }

    {
        vector<string> data;
        bool ok = sqlf.select_db(sqlf.get_table(),data);
        (ok == true ? vlogd($(ok)) : vloge($(ok)));
        for(auto a:data)
        {
            vlogi(a);
        }
    }


    {
        bool ok = sqli.insert_info({100000002,123456,18,1,"bear h11","zhongg","/path1"});
        (ok == true ? vlogd($(ok)) : vloge($(ok)));
    }
    {
        bool ok = sqli.insert_info({100000003,123456789,17,2,"bear h22","zhongg","/path2"});
        (ok == true ? vlogd($(ok)) : vloge($(ok)));
    }
    {
        bool ok = sqli.insert_info({100000004,999456789,17,2,"bear h99","zhongg","/path29"});
        (ok == true ? vlogd($(ok)) : vloge($(ok)));
    }
    {
        bool ok = sqli.insert_info({100000005,999456789,17,2,"bear h99","zhongg","/path29"});
        (ok == true ? vlogd($(ok)) : vloge($(ok)));
    }
    {
        bool ok = sqli.insert_info({100000003,123456789,16,3,"bear h33","zhongg","/path3"});
        (ok == true ? vlogd($(ok)) : vloge("test" $(ok)));
    }
    {
        bool ok = sqli.insert_info({100000019,123456789,13,0,"bear h44","zhongg","/path4"});
        (ok == true ? vlogd($(ok)) : vloge("test" $(ok)));
    }

    {
        vector<string> data;
        bool ok = sqli.select_db(sqli.get_table(),data);
        (ok == true ? vlogd($(ok)) : vloge($(ok)));
        for(auto a:data)
        {
            vlogi(a);
        }
    }
    {
        std::tuple<int64,int64,int64,int64,string,string,string> tup;
        bool ok = sqli.select_info(100000002,tup);
        (ok == true ? vlogd($(ok)) : vloge($(ok)));
        vlogd($(std::get<0>(tup)) $(std::get<1>(tup)) $(std::get<2>(tup))
        $(std::get<3>(tup)) $(std::get<4>(tup)) $(std::get<5>(tup)) $(std::get<6>(tup)) );
    }
    {
        sqlite_info::data fdata;
        bool ok = sqli.select_info(100000002,fdata);
        (ok == true ? vlogd($(ok)) : vloge($(ok)));
        vlogd($(fdata.account) $(fdata.phone) $(fdata.age) $(fdata.sex)
                      $(fdata.nickname) $(fdata.location) $(fdata.icon));
    }

    {
        bool ok = sqli.update_db(sqli.get_table(),sqli.get_data().account,100000003,sqli.get_data().age,89);
        (ok == true ? vlogd($(ok)) : vloge($(ok)));
    }
    {
        bool ok = sqli.delete_db(sqli.get_table(),sqli.get_data().account,100000002);
        (ok == true ? vlogd($(ok)) : vloge($(ok)));
    }
    {
        vector<string> data;
        bool ok = sqli.select_db(sqli.get_table(),data);
        (ok == true ? vlogd($(ok)) : vloge($(ok)));
        for(auto a:data)
        {
            vlogi(a);
        }
    }

    {
        bool ok = sql.drop_db(sqlf.get_table());
        (ok == true ? vlogd($(ok)) : vloge($(ok)));
    }
    {
        bool ok = sql.drop_db(sqli.get_table());
        (ok == true ? vlogd($(ok)) : vloge($(ok)));
    }
    {
        bool ok = sql.drop_db(sql.get_table());
        (ok == true ? vlogd($(ok)) : vloge($(ok)));
    }
}

int main()
{
    Tvlogs::get()->set_level(vlevel4::e_info);
    vlogi("== begin ==");

    int ret = 7;
    if(ret == 1) test_1();
    else if(ret == 2) test_2();
    else if(ret == 3) test_3();
    else if(ret == 4) test_4();
    else if(ret == 5) test_5();
    else if(ret == 6) test_6();
    else if(ret == 7) test_7();


    vloge("== end ==");

    return 0;
}

