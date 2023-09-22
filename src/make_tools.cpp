#include "make_tools.h"

make_tools::make_tools()
{

}

long long make_tools::rand_num(long long min,long long max)
{
    std::random_device rd;
    std::uniform_int_distribution<long long> uid(min,max);
    std::default_random_engine dre(rd());
    return uid(dre);
}