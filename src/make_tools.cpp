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

long long make_tools::make_id(long long int num1, long long int num2, long long int num3)
{
    return num1 + num2 + num3;
}
