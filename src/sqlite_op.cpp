#include "sqlite_op.h"

bool sqlite_base::open_db(string file)
{
    return (sqlite3_open(file.c_str(),&_db) == SQLITE_OK ? true : false);
}

bool sqlite_base::close_db()
{
    return (sqlite3_close_v2(_db) == SQLITE_OK ? true : false);
}

bool sqlite_base::delete_db(string table)
{
    string sql("DELETE FROM {0};");
    sql = sformat(sql)(table);
    return (sqlite3_exec(_db,sql.c_str(),nullptr,nullptr,&_error) == SQLITE_OK ? true : false);
}

bool sqlite_base::drop_db(string table)
{
    string sql = "DROP TABLE IF EXISTS {0};";
    sql = sformat(sql)(table);
    return (sqlite3_exec(_db,sql.c_str(),nullptr,nullptr,&_error) == SQLITE_OK ? true : false);
}

bool sqlite_base::select_db(string table, vector<string> &data)
{
    auto fn_cb = [](void *data, int argc, char **argv, char **name){
        vector<string> *vec = (vector<string>*)data;
        string str;
        for (int i=0; i<argc;i++)
        {
            str += " |:" + string(argv[i]);
        }
        vec->push_back(str);
        return 0;
    };

    string sql("SELECT * FROM {0};");
    sql = sformat(sql)(table);
    return (sqlite3_exec(_db,sql.c_str(),fn_cb,&data,&_error) == SQLITE_OK ? true : false);
}

string sqlite_base::get_error_exec()
{
    return (_error == nullptr ? "" : _error);
}

string sqlite_base::get_error()
{
    return (sqlite3_errmsg(_db) == nullptr ? "" : sqlite3_errmsg(_db));
}

int sqlite_base::count_db(string table)
{
    int value = -1;
    sqlite3_stmt *stmt;
    string sql(R"(SELECT COUNT(*) FROM {0};)");
    sql = sformat(sql)(table);

    if(sqlite3_prepare_v2(_db,sql.c_str(),-1,&stmt,nullptr) == SQLITE_OK)
    {
        if(sqlite3_step(stmt) == SQLITE_ROW)
        {
            value = sqlite3_column_int(stmt,0);  //获取结果
        }
    }
    sqlite3_finalize(stmt); //释放预处理语句
    return value;
}

sqlite_account::sqlite_account()
{
    _data.account = "account";
    _data.passwd = "passwd";
}

bool sqlite_account::open_account()
{
    return open_db(_file);
}

bool sqlite_account::create_account()
{
    //!
    //! account : 账号，范围在9位数之间 (100000000 - 999999999)
    //! passwd  : 密码，范围在6-18之间
    //!
    string sql = R"(
        CREATE TABLE {0} (
            {1} INTEGER PRIMARY KEY CHECK (account >= 100000000 AND account <= 999999999),
            {2} TEXT CHECK (LENGTH(passwd) >= 6 AND LENGTH(passwd) <= 18)
        );
        )";
    sql = sformat(sql)(_table,_data.account,_data.passwd);
    return (sqlite3_exec(_db,sql.c_str(),nullptr,nullptr,&_error) == SQLITE_OK ? true : false);
}

bool sqlite_account::insert_account(uint account, string passwd)
{
    string sql(R"(INSERT INTO {0} VALUES ({1},'{2}');)");
    sql = sformat(sql)(_table,account,passwd);
    return (sqlite3_exec(_db,sql.c_str(),nullptr,nullptr,&_error) == SQLITE_OK ? true : false);
}

bool sqlite_account::update_account(uint account, string passwd)
{
    string sql(R"( UPDATE {0} SET {1} = '{2}' WHERE {3} = {4}; )");
    sql = sformat(sql)(_table,_data.passwd,passwd,_data.account,account);
    return (sqlite3_exec(_db,sql.c_str(),nullptr,nullptr,&_error) == SQLITE_OK ? true : false);
}

bool sqlite_account::delete_account(uint account)
{
    string sql(R"( DELETE FROM {0} WHERE {1} = {2}; )");
    sql = sformat(sql)(_table,_data.account,account);
    return (sqlite3_exec(_db,sql.c_str(),nullptr,nullptr,&_error) == SQLITE_OK ? true : false);
}

sqlite_account::data_account *sqlite_account::get_data()
{
    return &_data;
}

string sqlite_account::get_file()
{
    return _file;
}

string sqlite_account::get_table()
{
    return _table;
}
