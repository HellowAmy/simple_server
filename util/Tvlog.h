//!
//! Tvlog.h
//!
#ifndef TVLOG_H
#define TVLOG_H

#include <iostream>
#include <fstream>
#include <mutex>

//== 单例模板 ==
template<class T>
class Tsingle_d
{
public:
    static T* get()
    {
        if(_obj == nullptr)
        {
            _mut.lock();
            if(_obj == nullptr) { _obj = new T; }
            _mut.unlock();
        }
        return _obj;
    }

    void clean()
    {
        _mut.lock();
        delete _obj; _obj = nullptr;
        _mut.unlock();
    }

private:
    static T *_obj;
    static std::mutex _mut;
    friend T;
    Tsingle_d() = default;
    ~Tsingle_d() = default;
    Tsingle_d(const Tsingle_d &) = delete ;
};

template<class T> T *Tsingle_d<T>::_obj = nullptr;
template<class T> std::mutex Tsingle_d<T>::_mut;
//== 单例模板 ==


//== 等级 ==
namespace level4
{ enum level{ e_error,e_warning,e_debug,e_info }; }

namespace level8
{ enum level{ e_off,e_fatal,e_error,e_warning,e_debug,e_trace,e_all }; }
//== 等级 ==


//== 调试日志模板 ==
//提供等级自定义模板参数
template<class Tlevel>
class Tvlog
{
public:
    inline void set_level(Tlevel el){ _level = el; } //设置等级

    inline Tvlog& operator<<(Tlevel el)
    { if(el <= _level) _ok = true; else _ok = false; return *this; }

    inline Tvlog& operator<<(std::ostream& (*end)(std::ostream&))
    { if(_ok) std::cout<<end; return *this; }

    template<class T>
    inline Tvlog& operator<<(const T &log)
    { if(_ok) std::cout<<log; return *this; }

private:
    bool _ok = false;   //判断等级是否符合
    Tlevel _level;      //最低显示的等级
};
//== 调试日志模板 ==


//== 文件日志模板 ==
//提供等级自定义模板参数
template<class Tlevel>
class Tflog
{
public:
    inline bool init(const std::string &file = "Tflog.log",bool app = true) //初始化日志
    {
        _file = file;
        if(app) _mode = std::ios::app; else _mode = std::ios::out;
        if(_fs.is_open() == false) _fs.open(_file,_mode);
        return _fs.is_open();
    }

    static std::string date_time() //获取秒精度的日期时间
    {
        time_t t; time(&t); char buf[64] = {0};
        strftime(buf,sizeof(buf),"%Y-%m-%d %H:%M:%S",localtime(&t));
        return std::string(buf);
    }

    inline void set_limit(int max) { _limit_max = max; }    //设置数量限制
    inline void set_level(Tlevel el) { _level = el; }       //设置等级
    inline void set_length(size_t len) { _len_max = len;}   //设置文件长度
    inline void close_log() { _fs.close(); }                //关闭日志

    inline Tflog& operator<<(Tlevel el)
    { if(el <= _level) _ok = true; else _ok = false; return *this; }

    inline Tflog& operator<<(std::ostream& (*end)(std::ostream&))
    { if(_ok &&_fs.is_open()){ _fs<<end; update_file(); } return *this; }

    template<class T>
    inline Tflog& operator<<(const T &log)
    { if(_ok &&_fs.is_open()) _fs<<log; return *this; }

private:
    bool _ok = false;       //判断等级是否符合
    int _limit_max = 0;     //日志文件限制数量
    int _limit_now = 1;     //当前写入日志
    Tlevel _level;          //最低显示的等级
    std::fstream _fs;       //文件对象
    std::string _file;      //文件名
    size_t _len_max = (1 << 26);    //最大长度--64M
    std::ios_base::openmode _mode;  //文件打开模式

    void update_file() //超出最大文件限制后更新文件名
    {
        if(_len_max < (size_t)_fs.tellg())
        {
            if(_limit_max == 0) write_unlimited();
            else write_limit();
        }
    }

    void write_unlimited() //无限制日志
    {
        _fs.close();
        for(int i=1;;i++)
        {
            std::string new_file = std::to_string(i)+"_"+_file;
            if(exist_file(new_file) == false)
            { rename(_file.c_str(),new_file.c_str()); break; }
        }
        _fs.open(_file,_mode);
    }

    void write_limit() //限制日志数量
    {
        _fs.close();
        {
            std::string new_file = std::to_string(_limit_now)+"_"+_file;
            rename(_file.c_str(),new_file.c_str());
            _limit_now++;
            if(_limit_now > _limit_max) _limit_now = 1;
        }
        _fs.open(_file,_mode);
    }

    static bool exist_file(const std::string &filename)    //判断文件是否存在
    { std::ifstream f(filename); return f.is_open(); }
};
//== 文件日志模板 ==

typedef level4::level vlevel4;
typedef Tsingle_d<Tflog<vlevel4>> Tflogs;
typedef Tsingle_d<Tvlog<vlevel4>> Tvlogs;

//颜色打印--注释掉则无颜色
#define VLOG_COLOR

//调试打印--开启则关闭调试打印
//#define VLOG_CLOSE


//== 功能宏 ==
//快速打印变量值
//参数：打印的值
#define $(value) "["#value": "<<value<<"] "
//== 功能宏 ==


//== 打印工厂宏 ==
#define VMAKE_LOG(txt,type,...)                                             \
    *Tvlogs::get()<<type                                                    \
    <<txt "["<<__FILE__<<":<"<<__LINE__<<">] <<<< "                         \
    <<__VA_ARGS__<<std::endl                                                \

#define VMAKE_LOG_COL(txt,type,...)                                         \
    *Tvlogs::get()<<type                                                    \
    <<txt "["<<__FILE__<<":<"<<__LINE__<<">] <<<< "                         \
    <<__VA_ARGS__<<"\033[0m"<<std::endl                                     \

#define FMAKE_LOG(txt,type,...)                                             \
    *Tflogs::get()<<type                                                    \
    <<"["<<Tflogs::get()->date_time()<<"] "                                 \
    <<txt " <<<< "<<__VA_ARGS__                                             \
    <<" ["<<__FILE__<<":<"<<__LINE__<<">]"<<std::endl                       \
//== 打印工厂宏 ==


//== 调试日志宏 ==
#ifndef VLOG_CLOSE
    #ifndef VLOG_COLOR
        #define vlogi(...) VMAKE_LOG("[Inf]",vlevel4::e_info,__VA_ARGS__)
        #define vlogd(...) VMAKE_LOG("[Deb]",vlevel4::e_debug,__VA_ARGS__)
        #define vlogw(...) VMAKE_LOG("[War]",vlevel4::e_warning,__VA_ARGS__)
        #define vloge(...) VMAKE_LOG("[Err]",vlevel4::e_error,__VA_ARGS__)
    #else //== 颜色打印分界 ==
        #define vlogi(...) VMAKE_LOG_COL("[Inf]",vlevel4::e_info,__VA_ARGS__)
        #define vlogd(...) VMAKE_LOG_COL("\033[32m[Deb]",vlevel4::e_debug,__VA_ARGS__)
        #define vlogw(...) VMAKE_LOG_COL("\033[33m[War]",vlevel4::e_warning,__VA_ARGS__)
        #define vloge(...) VMAKE_LOG_COL("\033[31m[Err]",vlevel4::e_error,__VA_ARGS__)
    #endif
#else
    #define vlogi(...)
    #define vlogd(...)
    #define vlogw(...)
    #define vloge(...)
#endif

//== 调试日志宏 ==


//== 文件日志宏 ==
#define flogi(...) FMAKE_LOG("[Inf]",vlevel4::e_info,__VA_ARGS__)
#define flogd(...) FMAKE_LOG("[Deb]",vlevel4::e_debug,__VA_ARGS__)
#define flogw(...) FMAKE_LOG("[War]",vlevel4::e_warning,__VA_ARGS__)
#define floge(...) FMAKE_LOG("[Err]",vlevel4::e_error,__VA_ARGS__)
//== 文件日志宏 ==


//===== 容器打印 =====
//参数：容器
template<class T>
void print_con(const T& con)
{
    std::cout<<"| size: "<<con.size()<<std::endl<<"| ";
    for(const auto &a:con)
    {
        std::cout<<a<<" ";
    }
    std::cout<<std::endl;
}

//参数：容器，换行长度
template<class T>
void print_con(const T& con,int len)
{
    int count = 0;
    std::cout<<"| size: "<<con.size()<<std::endl<<"| ";
    for(const auto &a:con)
    {
        if(count >= len) { count = 0; std::cout<<std::endl<<"| "; }
        count++;
        std::cout<<a<<" ";
    }
    std::cout<<std::endl;
}

//参数：容器，换行长度,分割符
template<class T>
void print_con(const T& con,int len,const std::string &flg)
{
    int count = 0;
    std::cout<<"| size: "<<con.size()<<std::endl<<"| ";
    for(const auto &a:con)
    {
        if(count >= len) { count = 0; std::cout<<std::endl<<"| "; }
        count++;
        std::cout<<a<<flg;
    }
    std::cout<<std::endl;
}

//参数：开始迭代器，结束迭代器,长度
template<class Tit>
void print_con(Tit begin,Tit end,int len)
{
    int count = 0;
    std::cout<<"| size: "<<end - begin<<std::endl<<"| ";
    for(auto it=begin;it!=end;it++)
    {
        if(count >= len) { count = 0; std::cout<<std::endl<<"| "; }
        count++;
        std::cout<<*it<<" ";
    }
    std::cout<<std::endl;
}

//参数：开始迭代器，结束迭代器,长度，分割符号
template<class Tit>
void print_con(Tit begin,Tit end,int len,const std::string &flg)
{
    int count = 0;
    std::cout<<"| size: "<<end - begin<<std::endl<<"| ";
    for(auto it=begin;it!=end;it++)
    {
        if(count >= len) { count = 0; std::cout<<std::endl<<"| "; }
        count++;
        std::cout<<*it<<flg;
    }
    std::cout<<std::endl;
}

#ifndef VLOG_CLOSE
    //参数：$v的容器打印，$v的参数
    #define vlogc(...)                                                  \
        std::cout<<"| ["<<__FILE__<<":<"<<__LINE__<<">]"<<std::endl;    \
        print_con(__VA_ARGS__);
#else
    #define vlogc(...)
#endif
//== 容器打印格式 ==


#define vlogif(ok,...) (ok ? vlogd(__VA_ARGS__) : vloge(__VA_ARGS__))

#endif // TVLOG_H
