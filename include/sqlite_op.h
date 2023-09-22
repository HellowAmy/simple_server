#ifndef SQLITE_OP_H
#define SQLITE_OP_H

#include <string>
#include <vector>

#include "format.h"
#include "../include/sqlite3/sqlite3.h"

using std::string;
using std::vector;

class sqlite_base
{
public:
    //通用操作
    bool open_db(string file);      //打开
    bool close_db();                //关闭
    bool delete_db(string table);   //删除数据
    bool drop_db(string table);     //删除表
    bool select_db(string table,vector<string> &data);   //查数据

    int count_db(string table); //统计条目

    string get_error_exec();   //错误显示
    string get_error();        //错误显示

protected:
    sqlite3 *_db = nullptr;
    char* _error = nullptr;
};

//ac passwd
class sqlite_account : public sqlite_base
{
public:
    struct data_account
    {
        string account;
        string passwd;
    };

public:
    sqlite_account();

    bool open_account();
    bool create_account();
    bool insert_account(uint account,string passwd);
    bool update_account(uint account,string passwd);
    bool delete_account(uint account);

    data_account* get_data();
    string get_file();
    string get_table();

private:
    string _file = "../data/ac_passwd.db";
    string _table = "ac_passwd";
    data_account _data;
};

#endif // SQLITE_OP_H
