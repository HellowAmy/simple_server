#ifndef FORMAT_H
#define FORMAT_H

#include <string>
#include <tuple>
#include <vector>
#include <sstream>
#include <algorithm>
#include <iomanip>

//using namespace std;

struct sformat
{
    std::string _str;
    std::vector<std::string> _vec;

    sformat(const std::string &str) : _str(str){}

    //== 字符串转换函数 ==
    template <class T>
    inline std::string to_string(T && val)
    { return std::to_string(std::forward<T>(val)); }

    inline std::string to_string(const char *val)
    { return val; }

    inline std::string to_string(const std::string &val)
    { return val; }

    inline std::string to_string(double val)
    {
        std::ostringstream os;
        os<<std::setprecision(10)<<std::noshowpoint<<val;
        return os.str();
    }
    //== 字符串转换函数 ==




    //== 处理格式化字符 ==
    template<class ...Tarr>
    std::string sformat_t()
    {
        int val = -1;
        int end = 0;
        while(true)
        {
            std::tuple<bool,int,int> a = find_flg(_str,end);
            int flg_begin = std::get<1>(a);
            int flg_end = std::get<2>(a);

            if(std::get<0>(a))
            {
                if(to_num(val,_str,flg_begin,flg_end))
                {
                    try{
                        _str = replace_string(_str,_vec.at(val),flg_begin,flg_end);
                    }
                    catch(...) { }
                }
                end = flg_end + _vec.at(val).size();
            }
            else break;
        }
        return _str;
    }

    template<class T,class ...Tarr>
    std::string sformat_t(T && val,const Tarr &...arg)
    {
        _vec.push_back(to_string(std::forward<T>(val)));
        return sformat_t(arg...);
    }

    template<class ...Tarr>
    std::string operator()(const Tarr &...arg)
    { return sformat_t(arg...); }
    //== 处理格式化字符 ==



    //!
    //! 功能：发现长字符串中标记的下标范围
    //!
    //! std::tuple<bool,int,int> :
    //!     [1:bool]    : 是否查到"{}" 标记,
    //!     [2:int]     : "{" 前标记的下标
    //!     [3:int]     : "}" 后标记的下标
    //!
    //! str      : 传入带标记的字符串
    //! in_begin : 从str的下标开始遍历
    //!
    std::tuple<bool,int,int> find_flg(const std::string &str,int in_begin = 0)
    {
        bool ret = false;
        int begin = -1;
        int end = -1;
        for(int i=in_begin;i<str.size();i++)
        {
            if(str[i] == '{') begin = i;
            else if(str[i] == '}' && begin != -1)
            {
                end = i;
                ret = true;
                break;
            }
        }
        return std::make_tuple(ret,begin,end);
    }

    //!
    //! 功能：从长字符串中定位，并返回标记数字
    //!
    //! val     : 返回字符串 "{x}" ,x的值
    //! str     : 传入带标记的字符串
    //! begin   : 传入 "{x}" 的字符串，"{"的前下标
    //! end     : 传入 "{x}" 的字符串，"}"的后下标
    //!
    bool to_num(int &val,const std::string &str,int begin,int end)
    {
        bool ret = false;
        std::string s(str.begin()+begin+1,str.begin()+end);

        try {
            val = std::stoi(s);
            ret = true;
        }
        catch(...) {}
        return ret;
    }

    //!
    //! 功能：传入长字串和拼接字符串，返回新拼接的字符串，需要传入拼接覆盖范围
    //!
    std::string replace_string(const std::string &str,const std::string &flg,int begin,int end)
    {
        std::string str_front(str.begin(),str.begin()+begin);
        std::string str_after(str.begin()+end+1,str.end());
        return str_front + flg + str_after;
    }
};

#endif // FORMAT_H
