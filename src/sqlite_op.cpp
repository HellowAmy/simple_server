#include "sqlite_op.h"

bool sqlite_base::open_db(cstr file)
{
    return (sqlite3_open(file.c_str(),&_db) == SQLITE_OK ? true : false);
}

bool sqlite_base::close_db()
{
    return (sqlite3_close_v2(_db) == SQLITE_OK ? true : false);
}

bool sqlite_base::create_db(cstr table_sql)
{
    return exec_db(table_sql);
}

bool sqlite_base::delete_db(cstr table)
{
    string sql("DELETE FROM {0};");
    sql = sformat(sql)(table);
    return exec_db(sql);
}

bool sqlite_base::drop_db(cstr table)
{
    string sql = "DROP TABLE IF EXISTS {0};";
    sql = sformat(sql)(table);
    return exec_db(sql);
}

bool sqlite_base::select_db(cstr table, vector<string> &data)
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
    return exec_db(sql,fn_cb,&data);
}

string sqlite_base::get_error_exec()
{
    return (_error == nullptr ? "" : _error);
}

string sqlite_base::get_error()
{
    return (sqlite3_errmsg(_db) == nullptr ? "" : sqlite3_errmsg(_db));
}

int sqlite_base::count_row_db(cstr table)
{
    string sql(R"(SELECT COUNT(*) FROM {0};)");
    sql = sformat(sql)(table);
    return step_column_db(sql,sqlite3_column_int);
}

bool sqlite_base::exec_db(cstr sql,int (*cb)(void*,int,char**,char**),void *data)
{
    return (sqlite3_exec(_db,sql.c_str(),cb,data,&_error) == SQLITE_OK ? true : false);
}

sqlite_base::~sqlite_base()
{
    close_db();
}

bool sqlite_base::commit_begin()
{
    string sql("BEGIN;");
    return exec_db(sql);
}

bool sqlite_base::commit_end()
{
    string sql("COMMIT;");
    return exec_db(sql);
}

bool sqlite_base::commit_back()
{
    string sql("ROLLBACK;");
    return exec_db(sql);
}

bool sqlite_ac_abs::open_info()
{
    string sql("PRAGMA foreign_keys = ON;");
    if(open_db(_file) && exec_db(sql)) return true;
    {
        close_db();
        return false;
    }
}

string sqlite_ac_abs::get_file()
{
    return _file;
}

string sqlite_ac_abs::get_table()
{
    return _table;
}

sqlite_account::sqlite_account()
{
    _table = "ac_passwd";
    _data.account = "account";
    _data.passwd = "passwd";
}

bool sqlite_account::create_table()
{
    //!
    //! account : 账号，范围在9位数之间 (100000000 - 999999999)
    //! passwd  : 密码，范围在6-18之间
    //!
    string sql = R"(
        CREATE TABLE {0} (
            {1} INTEGER PRIMARY KEY CHECK ({1} >= 100000000 AND {1} <= 999999999),
            {2} TEXT CHECK (LENGTH({2}) >= 6 AND LENGTH({2}) <= 18)
        );
        )";
    sql = sformat(sql)(_table,_data.account,_data.passwd);
    return exec_db(sql);
}

bool sqlite_account::insert_account(int64 account, string passwd)
{
    return insert_db(_table,account,passwd);
}

bool sqlite_account::update_account(int64 account, string passwd)
{
    return update_db(_table,_data.account,account,_data.passwd,passwd);
}

bool sqlite_account::delete_account(int64 account)
{
    return delete_db(_table,_data.account,account);
}

sqlite_account::data sqlite_account::get_data()
{
    return _data;
}

bool sqlite_account::select_account(int64 account, string &passwd)
{
    std::tuple<bool,string> tup = std::make_tuple(false,_data.passwd);
    passwd = _data.passwd;
    auto fn_cb = [](void *data, int argc, char **argv, char **name){
        std::tuple<bool,string> *ptup = (std::tuple<bool,string>*)data;
        string fname = std::get<1>(*ptup);
        for (int i=0; i<argc;i++)
        {
            if(string(name[i]) == fname) *ptup = std::make_tuple(true,argv[i]);
        }
        return 0;
    };
    string sql("SELECT * FROM {0} WHERE {1} = {2};");
    sql = sformat(sql)(_table,_data.account,account);
    exec_db(sql,fn_cb,&tup);

    passwd = std::get<1>(tup);
    return std::get<0>(tup);
}

bool sqlite_account::select_account(vector<int64> &vec)
{
    std::tuple<string,vector<int64>*> tup = std::make_tuple(_data.account,&vec);

    auto fn_cb = [](void *data, int argc, char **argv, char **name){
        auto ptup = (std::tuple<string,vector<int64>*>*)data;
        string fname = std::get<0>(*ptup);
        vector<int64>* fdata = std::get<1>(*ptup);
        for (int i=0; i<argc;i++)
        {
            try {
                if(string(name[i]) == fname) fdata->push_back(std::stoll(argv[i]));
            } catch (...){}
        }
        return 0;
    };

    string sql("SELECT * FROM {0} ;");
    sql = sformat(sql)(_table);
    return exec_db(sql,fn_cb,&tup);
}


sqlite_friends::sqlite_friends()
{
    _table = "ac_friends";
    _data.account = "account";
    _data.friends = "friends";
    _data.remarks = "remarks";
}

bool sqlite_friends::create_table()
{
    //!
    //! account : 账号，约束于 ac_passwd
    //! friends : 好友，约束于 ac_passwd
    //!
    string sql = R"(
        CREATE TABLE {0} (
            {1} INTEGER ,
            {2} INTEGER ,
            {3} INTEGER ,
            CHECK ({1} != {2}),
            FOREIGN KEY ({1}) REFERENCES {4} ({5}),
            FOREIGN KEY ({2}) REFERENCES {4} ({5}),
            PRIMARY KEY ({1}, {2})
        );
        )";
    sqlite_account ac;
    sql = sformat(sql)(_table,_data.account,_data.friends,_data.remarks,ac.get_table(),ac.get_data().account);
    return exec_db(sql);
}

sqlite_friends::data sqlite_friends::get_data()
{
    return _data;
}

bool sqlite_friends::insert_friends(int64 account, int64 friends,string remarks)
{
    return insert_db(_table,account,friends,remarks);
}

bool sqlite_friends::delete_friends(int64 account, int64 friends)
{
    string sql(R"( DELETE FROM {0} WHERE {1} = {2} AND {3} = {4}; )");
    sql = sformat(sql)(_table,_data.account,account,_data.friends,friends);
    return exec_db(sql);
}

bool sqlite_friends::select_friends(int64 account, vector<map<string,string>> &vec_line)
{
    return select_line_db(_table,vec_line,_data.account,account);
}

bool sqlite_friends::select_remarks(int64 account, int64 ac_friends, string &remarks)
{
    vector<map<string,string>> vec_line;
    bool ok = select_friends(account,vec_line);
    if(ok)
    {
        for(auto &a:vec_line)
        {
            //获取一行信息
            data fdate;
            for(auto &b:a)
            {
                if(b.first == _data.friends)
                {
                    fdate.friends = b.second;
                }
                else if(b.first == _data.remarks)
                {
                    fdate.remarks = b.second;
                }
            }

            //对比好友账号
            try {
                int64 ac = std::stoll(fdate.friends);
                if(ac == ac_friends)
                {
                    remarks = fdate.remarks;
                    return true;
                }
            }
            catch(...) { return false; }
        }
    }
    return ok;
}

bool sqlite_friends::insert_ac_both_friend(int64 account, int64 friends, string ac_remarks,string fr_remarks)
{
    commit_begin();
    bool ok1 = insert_friends(account,friends,ac_remarks);
    bool ok2 = insert_friends(friends,account,fr_remarks);

    bool ok_ret = (ok1 && ok2);
    if(ok_ret) commit_end();
    else commit_back();
    return ok_ret;
}

bool sqlite_friends::update_remarks(int64 account, int64 friends, string remarks)
{
    string sql(R"( UPDATE {0} SET {1} = '{2}' WHERE {3} = {4} AND {5} = {6}; )");
    sql = sformat(sql)(_table,_data.remarks,remarks,
                       _data.account,account,
                       _data.friends,friends);
    return exec_db(sql);
}

bool sqlite_info::create_table()
{
    //!
    //! account     :
    //! phone       :
    //! age         :
    //! sex         :
    //! nickname    :
    //! location    :
    //! icon        :
    //!
    string sql = R"(
        CREATE TABLE {0} (
            {1} INTEGER PRIMARY KEY,
            {2} INTEGER CHECK ({2} >= 0) ,
            {3} INTEGER CHECK ({3} >= 0 AND {3} <= 200) ,
            {4} INTEGER CHECK ({4} >= 0 AND {4} <= 3) ,
            {5} TEXT,
            {6} TEXT,
            {7} TEXT,
            FOREIGN KEY ({1}) REFERENCES {8} ({9})
        );
        )";
    sqlite_account ac;
    sql = sformat(sql)(_table,
                            _data.account,
                            _data.phone,
                            _data.age,
                            _data.sex,
                            _data.nickname,
                            _data.location,
                            _data.icon,
                       ac.get_table(),ac.get_data().account);
    return exec_db(sql);
}

sqlite_info::sqlite_info()
{
    _table          = "ac_info";
    _data.account   = "account";
    _data.phone     = "phone";
    _data.age       = "age";
    _data.sex       = "sex";
    _data.nickname  = "nickname";
    _data.location  = "location";
    _data.icon      = "icon";
}

bool sqlite_info::insert_info(std::tuple<int64, int64, int64, int64, string, string, string> tup)
{
    return insert_db(_table,std::get<0>(tup),std::get<1>(tup),std::get<2>(tup),
                      std::get<3>(tup),std::get<4>(tup),std::get<5>(tup),std::get<6>(tup));
}

bool sqlite_info::select_info(int64 account, std::tuple<int64,int64, int64, int64, string, string, string> &tup)
{
    std::tuple<data,data> ftup = std::make_tuple(_data,data());
    auto fdata = std::get<1>(ftup);
    bool ok = select_info(account,fdata);
    if(ok)
    {
        try {
            tup = std::make_tuple(std::stoll(fdata.account),std::stoll(fdata.phone),
                                  std::stoll(fdata.age),std::stoll(fdata.sex),
                                  fdata.nickname,fdata.location,fdata.icon);
        } catch(...) { ok = false; }
    }
    return ok;
}

sqlite_info::data sqlite_info::get_data()
{
    return _data;
}

bool sqlite_info::select_info(int64 account, sqlite_info::data &fdata)
{
    std::tuple<data,data*> ftup = std::make_tuple(_data,&fdata);

    auto fn_cb = [](void *in_data, int argc, char **argv, char **name){
        auto ptup = (std::tuple<data,data*>*)(in_data);
        auto field = std::get<0>(*ptup);
        auto fdata = std::get<1>(*ptup);

        for (int i=0; i<argc;i++)
        {
            if(name[i] == field.account)        fdata->account   = argv[i];
            else if(name[i] == field.phone)     fdata->phone     = argv[i];
            else if(name[i] == field.age)       fdata->age       = argv[i];
            else if(name[i] == field.sex)       fdata->sex       = argv[i];
            else if(name[i] == field.nickname)  fdata->nickname  = argv[i];
            else if(name[i] == field.location)  fdata->location  = argv[i];
            else if(name[i] == field.icon)      fdata->icon      = argv[i];
        }
        *ptup = std::make_tuple(field,fdata);
        return 0;
    };
    string sql("SELECT * FROM {0} WHERE {1} = {2};");
    sql = sformat(sql)(_table,_data.account,account);
    return exec_db(sql,fn_cb,&ftup);
}






sqlite_cache::sqlite_cache()
{
    _data.target = "target";
    _data.sjson = "sjson";
}

bool sqlite_cache::open_cache()
{
    return open_db(_file);
}

bool sqlite_cache::create_cache()
{
    //!
    //! target : 目标账号
    //! sjson  : 暂存信息
    //!
    string sql = R"(
        CREATE TABLE {0} (
            {1} INTEGER ,
            {2} TEXT
        );
        )";
    sql = sformat(sql)(_table,_data.target,_data.sjson);
    return exec_db(sql);
}

bool sqlite_cache::insert_cache(int64 target, string sjson)
{
    return insert_db(_table,target,sjson);
}

bool sqlite_cache::select_cache(int64 target, vector<string> &data)
{
    auto fn_cb = [](void *data, int argc, char **argv, char **name){
        auto *ptup = (std::tuple<string,vector<string>*>*)data;
        string fname = std::get<0>(*ptup);
        vector<string> *vec = std::get<1>(*ptup);
        for (int i=0; i<argc;i++)
        {
            if(name[i] == fname) vec->push_back(argv[i]);
        }
        return 0;
    };

    std::tuple<string,vector<string>*> tup = std::make_tuple(_data.sjson,&data);
    string sql("SELECT * FROM {0} WHERE {1} = {2};");
    sql = sformat(sql)(_table,_data.target,target);
    return exec_db(sql,fn_cb,&tup);
}

bool sqlite_cache::delete_cache(int64 target)
{
    return delete_db(_table,_data.target,target);
}

string sqlite_cache::get_file()
{
    return _file;
}

string sqlite_cache::get_table()
{
    return _table;
}

