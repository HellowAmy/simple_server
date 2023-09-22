#ifndef SERVER_TASK_H
#define SERVER_TASK_H

#include <string>

using std::string;


class server_task
{
public:
    void recv_msg(const string &msg);

};


#endif //SERVER_TASK_H
