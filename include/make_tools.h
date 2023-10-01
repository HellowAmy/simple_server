#ifndef MAKE_TOOLS_H
#define MAKE_TOOLS_H

#include <ctime>
//#include <random>

#include <random>

class make_tools
{
public:
    make_tools();
    static long long rand_num(long long min = 100000000,long long max = 999999999);
    static long long make_id(long long num1,long long num2,long long num3);

};

#endif // MAKE_TOOLS_H
