#include <iostream>

#include "../util/Tvlog.h"
#include "make_tools.h"
#include "inter_server.h"
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
        bool ok = sql.open_account();
        (ok == true ? vlogd($(ok)) : vloge($(ok)));
    }
    {
        bool ok = sql.create_account();
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
        bool ok = sql.update_account(123456789,"aabbcc");
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
        bool ok = sql.open_account();
        (ok == true ? vlogd($(ok)) : vloge($(ok)));
    }
    {
        bool ok = sql.create_account();
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

int main()
{
    Tvlogs::get()->set_level(vlevel4::e_info);
    vlogi("== begin ==");

    int ret = 1;
    if(ret == 1) test_1();
    if(ret == 2) test_2();
    if(ret == 3) test_3();


    vloge("== end ==");

    return 0;
}

