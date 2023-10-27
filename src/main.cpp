#include <iostream>

#include "../../simple_chat/web_protocol.h"
#include "../util/Tvlog.h"
#include "make_tools.h"
#include "inter_server.h"
#include "server_task.h"
#include "server_files.h"
#include "sqlite_op.h"

using namespace std;

//主要程序
void test_0()
{
    vlogd("== server ==");

    int ret = fork();
    if(ret == 0)
    {
        vlogd("== server task ==");

        sqlite_account  db_account;
        sqlite_friends  db_friends;
        sqlite_info     db_info;
        sqlite_cache    db_cache;

        if(db_account.open_info()
           && db_friends.open_info()
           && db_info.open_info()
           && db_cache.open_cache())
        {
            //任务交换
            server_task task;
            task.init_data(&db_account, &db_friends, &db_info, &db_cache);

            inter_server sw_task;
            sw_task.func_bind(task.fn_open, task.fn_message, task.fn_close);
            sw_task.open(CS_PORT_TASKS);
        }
        else vlogw("open data failed");
    }
    else if(ret > 0)
    {
        vlogd("== server files ==");

        //文件传输
        server_files files;
        inter_server sw_files;
        sw_files.func_bind(files.fn_open, files.fn_message, files.fn_close);
        sw_files.open(CS_PORT_FILES);
    }
    else vlogw("fork failed");

    vloge("== server end ==");
}

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
        int value = sql.count_row_db(sql.get_table());
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
        int value = sql.count_row_db(sql.get_table());
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
        int value = sql.count_row_db(sql.get_table());
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

}

//网络转发
void test_5()
{
    vlogd("== server ==");

    sqlite_account  db_account;
    sqlite_friends  db_friends;
    sqlite_info     db_info;
    sqlite_cache    db_cache;

    if(db_account.open_info()
        && db_friends.open_info()
           && db_info.open_info()
              && db_cache.open_cache())
    {
        server_task t;
        t.init_data(&db_account,&db_friends,&db_info,&db_cache);

        inter_server s;
        s.func_bind(t.fn_open,t.fn_message,t.fn_close);
        s.open(CS_PORT_TASKS);
    }
    else vlogw("open data failed");

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
        for(int i=50;i<180;i++)
        {
            bool ok = sql.insert_cache(i,"123456789");
            if(ok == false) vloge($(i));
        }
    }
    {
        vector<string> data;
        bool ok = sql.select_cache(70,data);
        vlogd($("ok:")$(ok));
        vlogc(data,10);
    }
    {
        vector<string> data;
        bool ok = sql.select_db(sql.get_table(),data);
        (ok == true ? vlogd($(ok)) : vloge($(ok)));
        vlogc(data,10);
    }
    {
        int num = sql.count_row_db(sql.get_table());
        vlogd($(num));
    }
    {
        bool ok = sql.delete_cache(90);
        (ok == true ? vlogd($(ok)) : vloge($(ok)));
    }
    {
        int num = sql.count_row_db(sql.get_table());
        vlogd($(num));
    }
    {
        bool ok = sql.delete_db(sql.get_table());
        (ok == true ? vlogd($(ok)) : vloge($(ok)));
    }
    {
        int num = sql.count_row_db(sql.get_table());
        vlogd($(num));
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

//    {
//        int v = 100000000;
//        for(int i=v;i<v+5;i++)
//        {
//            for(int j=i;j<i+5;j++)
//            {
//                bool ok = sqlf.insert_friends(i,j);
//                (ok == true ? vlogd($(ok)) : vloge($(ok)));
//            }
//        }
//    }
//    {
//        vector<string> data;
//        bool ok = sqlf.select_db(sqlf.get_table(),data);
//        (ok == true ? vlogd($(ok)) : vloge($(ok)));
//        for(auto a:data)
//        {
//            vlogi(a);
//        }
//    }

//    {
//        vector<string> data;
//        bool ok = sqlf.select_friends(100000003,data);
//        (ok == true ? vlogd($(ok)) : vloge($(ok)));
//        for(auto a:data)
//        {
//            vlogi(a);
//        }
//    }

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

//账号下发
void test_8()
{
    auto fn_select = [](sqlite_ac_abs *sql,string table){
        vector<string> data;
        bool ok = sql->select_db(table,data);
        (ok == true ? vlogd($(ok) $(table)) : vloge($(ok) $(table)));
        for(auto a:data)
        {
            vlogi("vec: " << a);
        }
    };

    auto fn_vec = [](vector<int64> data){
        for(auto a:data)
        {
            vlogi("sfvec: " << a);
        }
    };

    sqlite_account sqla;
    sqlite_friends sqlf;
    sqlite_info sqli;

    vlogd("== open_info ==");
    {
        {
            bool ok = sqla.open_info();
            (ok == true ? vlogd($(ok)) : vloge($(ok)));
        }
        {
            bool ok = sqlf.open_info();
            (ok == true ? vlogd($(ok)) : vloge($(ok)));
        }
        {
            bool ok = sqli.open_info();
            (ok == true ? vlogd($(ok)) : vloge($(ok)));
        }
        {
            bool ok = sqla.create_table();
            (ok == true ? vlogd($(ok)) : vloge($(ok)));
        }
        {
            bool ok = sqlf.create_table();
            (ok == true ? vlogd($(ok)) : vloge($(ok)));
        }
        {
            bool ok = sqli.create_table();
            (ok == true ? vlogd($(ok)) : vloge($(ok)));
        }
    }

    int v = 100000000;
    {
        vlogd("== insert_account ==");
        for(int i=v;i<v+10;i++)
        {
            bool ok = sqla.insert_account(i,"123456");
            (ok == true ? vlogd($(ok)) : vloge($(ok)));
        }

        vector<int64> data;
        {
            bool ok = sqla.select_account(data);
            (ok == true ? vlogd($(ok)) : vloge($(ok)));
        }
        fn_vec(data);

        vlogd("== insert_friends ==");
        {
//            for(int i=0;i<data.size();i++)
//            {
//                int64 ac = data[i];
//                for(int j=i;j<data.size();j++)
//                {
//                    int64 fs = data[j];
//                    bool ok = sqlf.insert_friends(ac,fs);
//                    (ok == true ? vlogd($(ok)) : (ac == fs ? vlogd($(ok)) : vloge($(ok))));
//                }
//            }
        }

        vlogd("== insert_info ==");
        for(int i=0;i<data.size();i++)
        {
            int64 account = data[i];
            int64 phone = std::stoll("1100" + std::to_string(i));
            int64 age = 18+i;
            int64 sex = (i%2 == 0 ? 0:1);
            string nickname = "test_" + std::to_string(account);
            string location = "zhong_" + std::to_string(i);
            string icon = "/path/default";
            bool ok = sqli.insert_info({account,phone,age,sex,nickname,location,icon});
            (ok == true ? vlogd($(ok)) : vloge($(ok) $(account)));
        }
    }

    vlogd("== fn_select ==");
    {
        fn_select(&sqla,sqla.get_table());
        fn_select(&sqlf,sqlf.get_table());
        fn_select(&sqli,sqli.get_table());
    }

    vlogd("== drop_db ==");
//    {
//        {
//            bool ok = sqla.drop_db(sqlf.get_table());
//            (ok == true ? vlogd($(ok)) : vloge($(ok)));
//        }
//        {
//            bool ok = sqla.drop_db(sqli.get_table());
//            (ok == true ? vlogd($(ok)) : vloge($(ok)));
//        }
//        {
//            bool ok = sqla.drop_db(sqla.get_table());
//            (ok == true ? vlogd($(ok)) : vloge($(ok)));
//        }
//    }
}

//文件转换
void test_9()
{
    vlogd("== server files ==");

    server_files t;
    inter_server s;
    s.func_bind(t.fn_open,t.fn_message,t.fn_close);
    s.open(CS_PORT_FILES);

    vlogd("== server files end ==");

}



//测数据库
void test_10()
{
    sqlite_base sql;
    string table = "tab";
    {
        bool ok = sql.open_db("test_base.db");
        vlogif(ok,$(ok));
    }

    { //PRIMARY KEY
        string str_sql = R"(
        CREATE TABLE tab (
            id INTEGER,
            a INTEGER,
            b INTEGER,
            c TEXT
        );
        )";

        bool ok = sql.create_db(str_sql);
        vlogif(ok,$(ok));
    }

    for(int i=0;i<18;i++)
    {
        bool ok = sql.insert_db(table,i+1,i+11,i+22,"ok:" + std::to_string(i));
        vlogif(ok,$(ok));
    }

    vlogi("======== show ====\n");

    {
        int value = sql.count_row_db(table);
        vlogi("=================: "<<$(value));
    }
    {
        vector<string> vec;
        int value = sql.select_db(table,vec);
        vlogc(vec,1);
    }
    {
        vector<map<string,string>> vec;
        bool ok = sql.select_line_db(table,vec,"id",10);
        vlogif(ok,$(ok) $(vec.size()));

        for(auto a:vec)
        {
            for(auto b:a)
            {
                //key: b.first  | value: b.second
                vlogi($(b.first) $(b.second));
            }
        }
    }

    vlogi("======== end ====\n");
    {
        bool ok = sql.delete_db(table);
        vlogif(ok,$(ok));
    }
    {
        bool ok = sql.drop_db(table);
        vlogif(ok,$(ok));
    }
    {
        bool ok = sql.close_db();
        vlogif(ok,$(ok));
    }
}

void test_11()
{
    sqlite_friends sqlf;
    {
        bool ok = sqlf.open_info();
        vlogif(ok,$(ok));
    }
    {
        bool ok = sqlf.create_table();
        vlogif(ok,$(ok));
    }

    int64 account = 535181553;
    {
        vector<int64> vec;
        vec.push_back(303756815);
        vec.push_back(340717797);
        vec.push_back(411821267);
        vec.push_back(412131535);
        vec.push_back(422157964);
        vec.push_back(434592263);
        vec.push_back(449749116);
        vec.push_back(500902190);
        vec.push_back(518211187);
        vec.push_back(523892629);
//        vec.push_back(607037441);
//        vec.push_back(616660262);
//        vec.push_back(673944815);
//        vec.push_back(714405069);
//        vec.push_back(796304805);
//        vec.push_back(798315362);
//        vec.push_back(834477711);
//        vec.push_back(877370201);
//        vec.push_back(886423439);
//        vec.push_back(987491857);

        for(auto a:vec)
        {
            bool ok = sqlf.insert_ac_both_friend(account,a,"","");
            (ok == true ? vlogd($(ok)) : vloge($(ok)));
        }
    }

//    {
//        int v = 100000000;
//        for(int i=v;i<v+5;i++)
//        {
//            for(int j=i+1;j<i+5;j++)
//            {
//                bool ok = sqlf.insert_ac_both_friend(i,j,"respo+"+std::to_string(i) +"+ww+"+ std::to_string(j));
//                (ok == true ? vlogd($(ok)) : vloge($(ok)));
//            }
//        }
//    }
//    {
//        int v = 100000000 +3;
//        {
//            vector<map<string,string>> vec_line;
//            bool ok = sqlf.select_friends(v,vec_line);
//            vlogif(ok,$(ok));
//            for(auto a:vec_line)
//            {
//                for(auto b:a)
//                {
//                    //key: b.first  | value: b.second
//                    vlogi($(b.first) $(b.second));
//                }
//                vlogi("\n");
//            }
//        }
//        {
//            string remarks;
//            bool ok = sqlf.select_remarks(v,v+2,remarks);
//            vlogif(ok,$(ok));
//        }
//    }

    {
        vector<string> data;
        bool ok = sqlf.select_db(sqlf.get_table(),data);
        vlogc(data,1);
    }

    {
        string remarks;
        bool ok = sqlf.select_remarks(account,412131535,remarks);
        vlogw($(ok) $(remarks));
    }
    {
        string remarks;
        bool ok = sqlf.update_remarks(account,412131535,"不认识的人");
        vlogw($(ok) $(remarks));
    }
    {
        string remarks;
        bool ok = sqlf.select_remarks(account,412131535,remarks);
        vlogw($(ok) $(remarks));
    }

    {
        string remarks;
        bool ok = sqlf.select_remarks(account,798315362,remarks);
        vlogw($(ok) $(remarks));
    }

//    {
//        bool ok = sqlf.drop_db(sqlf.get_table());
//        vlogif(ok,$(ok));
//    }
    {
        bool ok = sqlf.close_db();
        vlogif(ok,$(ok));
    }
}

int main()
{
    Tvlogs::get()->set_level(vlevel4::e_info);
    vlogi("== begin ==");

    int ret = 0;
    if(ret == 0)      test_0();     //主要程序
    else if(ret == 1) test_1();     //网络连接
    else if(ret == 2) test_2();     //账号密码
    else if(ret == 3) test_3();     //随机账号
    else if(ret == 4) test_4();     //测试输入
    else if(ret == 5) test_5();     //网络转发
    else if(ret == 6) test_6();     //缓存测试
    else if(ret == 7) test_7();     //外键约束
    else if(ret == 8) test_8();     //账号下发
    else if(ret == 9) test_9();     //文件转换

    else if(ret == 10) test_10();   //数据接口
    else if(ret == 11) test_11();   //添加好友

    vloge("== end ==");

    return 0;
}

