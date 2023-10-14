#ifndef SQLITE_OP_H
#define SQLITE_OP_H

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <functional>

#include "format.h"
#include "../include/sqlite3/sqlite3.h"

//#include "../util/Tvlog.h"

typedef const std::string& cstr;
typedef long long int64;
using std::string;
using std::map;
using std::vector;

//
class sqlite_base
{
public:
    //通用操作
    ~sqlite_base();
    bool open_db(cstr file);                                    //打开
    bool close_db();                                            //关闭
    bool create_db(cstr table_sql);                             //创建
    bool drop_db(cstr table);                                   //删除表
    bool delete_db(cstr table);                                 //删数据
    bool select_db(cstr table, vector<string> &data);           //查数据

    int count_row_db(cstr table);   //统计条目
    string get_error_exec();        //错误显示
    string get_error();             //错误显示

    //删除单条
    template <class Tid>
    bool delete_db(cstr table, cstr field_id, Tid id)
    {
        string sql(R"( DELETE FROM {0} WHERE {1} = {2}; )");
        sql = sformat(sql)(table,field_id,
                            (is_string(id) ? "'{0}'" : "{0}"));
        sql = sformat(sql)(id);
        return exec_db(sql);
    }

    //修改数据
    template <class Tid,class Tupdata>
    bool update_db(cstr table, cstr field_id, Tid id, cstr field_update, Tupdata update)
    {
        string sql(R"( UPDATE {0} SET {1} = {2} WHERE {3} = {4}; )");
        sql = sformat(sql)(table,
                           field_update,
                           (is_string(update) ? "'{0}'" : "{0}"),
                           field_id,
                           (is_string(id) ? "'{1}'" : "{1}"));
        sql = sformat(sql)(update,id);
        return exec_db(sql);
    }

    //插入数据 : [ table , value1 , value2 , ...]
    template <class... Tarr>
    bool insert_db(cstr table,Tarr... args)
    {
        string sql(R"(INSERT INTO {0} VALUES ({1});)");
        sql = sformat(sql)(table,"{0}");
        return insert_mpl(sql,args...);
    }

    //! 返回数值命令:
    //!       string sql(R"(SELECT COUNT(*) FROM {0};)");
    //!       sql = sformat(sql)(table);
    //!       return step_column_db(sql,sqlite3_column_int);
    template<class Tfunc>
    auto step_column_db(cstr sql,Tfunc&& func,int index = 0)
        ->typename std::result_of<Tfunc(sqlite3_stmt*,int)>::type
    {
        //准备命令
        typename std::result_of<Tfunc(sqlite3_stmt*,int)>::type value;
        sqlite3_stmt *stmt;
        if(sqlite3_prepare_v2(_db,sql.c_str(),-1,&stmt,nullptr) == SQLITE_OK)
        {
            //获取结果
            if(sqlite3_step(stmt) == SQLITE_ROW)
            { value = func(stmt,index); }
        }
        sqlite3_finalize(stmt); //释放预处理语句
        return value;
    }

    //按行选择
    template<class Tid>
    bool select_line_db(cstr table,vector<map<string,string>> &_vec_line,
                        cstr field_id, Tid id,
                        string sql = "")
    {
        auto fn_cb = [](void *data, int argc, char **argv, char **name){
            auto vec = (vector<map<string,string>>*)data;
            map<string,string> map;
            for (int i=0; i<argc;i++)
            { map[name[i]] = argv[i]; }
            vec->push_back(map);
            return 0;
        };

        if(sql == "")
        {
            sql = "SELECT * FROM {0} WHERE {1} = {2};";
            sql = sformat(sql)(table,field_id,id);
        }
        return exec_db(sql,fn_cb,&_vec_line);
    }



protected:
    sqlite3 *_db = nullptr;
    char* _error = nullptr;

    template <class T>
    bool is_string(T value)
    {
        if(typeid(value) == typeid(string)
           || typeid(value) == typeid(const char *)) return true;
        return false;
    }

    bool exec_db(cstr sql,int (*cb)(void*,int,char**,char**) = nullptr,void *data = nullptr);

    //== 插入数据 : 具体操作 ==
    template <class T>
    string insert_parse_sql(cstr sql,T value,bool app = true)
    {
        //判断数字与字符调整插入选项
        string flg;
        if(typeid(T) == typeid(string)
           || typeid(T) == typeid(const char*) )
        { flg = "'{0}'"; }
        else flg = "{0}";

        //插入并判断是否追加后续
        flg = sformat(flg)(value);
        if(app) flg += ",{0}";
        return sformat(sql)(flg);
    }

    template <class T,class... Tarr>
    bool insert_mpl(string sql,T value)
    {
        sql = insert_parse_sql(sql,value,false);
        return exec_db(sql);
    }

    template <class T, class... Tarr>
    bool insert_mpl(string sql,T value,Tarr... args)
    {
        sql = insert_parse_sql(sql,value);
        return insert_mpl(sql,args...);
    }
    //== 插入数据 : 具体操作 ==
};

//
class sqlite_ac_abs : public sqlite_base
{
public:
    bool open_info();
    virtual bool create_table() = 0;

    string get_file();
    string get_table();

protected:
    string _table = "";

private:
    string _file = "../data/account_info.db";
};

//account and passwd
class sqlite_account : public sqlite_ac_abs
{
public:
    struct data
    {
        string account;
        string passwd;
    };

public:
    sqlite_account();

    bool create_table() override;
    bool insert_account(int64 account,string passwd);
    bool update_account(int64 account,string passwd);
    bool delete_account(int64 account);
    bool select_account(int64 account,string &passwd);
    bool select_account(vector<int64> &vec);

    data get_data();

private:
    data _data;
};

//account and friends
class sqlite_friends : public sqlite_ac_abs
{
public:
    struct data
    {
        string account;
        string friends;
    };

public:
    sqlite_friends();
    bool create_table() override;

    bool insert_friends(int64 account,int64 friends);
    bool select_friends(int64 account,vector<string> &data);
    bool delete_friends(int64 account,int64 friends);

    data get_data();

private:
    data _data;
};


//account info
class sqlite_info : public sqlite_ac_abs
{
public:
    struct data
    {
        string account;
        string phone;
        string age;
        string sex;
        string nickname;
        string location;
        string icon;
    };

public:
    sqlite_info();

    bool create_table() override;

    bool insert_info(std::tuple<int64,int64,int64,int64,string,string,string> tup);
    bool select_info(int64 account,std::tuple<int64,int64,int64,int64,string,string,string> &tup);
    bool select_info(int64 account,data &fdata);

    data get_data();


private:
    data _data;
};

//
class sqlite_cache : public sqlite_base
{
public:
    struct data_cache
    {
        string target;
        string sjson;
    };

public:
    sqlite_cache();
    bool open_cache();
    bool create_cache();

    bool insert_cache(int64 target,string sjson);
    bool select_cache(int64 target,vector<string> &data);
    bool delete_cache(int64 target);

    string get_file();
    string get_table();

private:
    string _file = "../data/swap_cache.db";
    string _table = "swap_cache";
    data_cache _data;
};


#endif // SQLITE_OP_H
