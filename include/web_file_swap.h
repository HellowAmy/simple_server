
#ifndef WEB_FILE_SWAP_H
#define WEB_FILE_SWAP_H

#include "inter_server.h"

#include <fstream>
#include <memory>
#include <thread>

#define WRITE_BUFSIZE_HIGH_WATER    (1U << 23)  // 8M

using std::fstream;

typedef int tpsend(const char* buf, int len, enum ws_opcode opcode, bool fin);
typedef long long int64;

class web_file_swap
{
public:
    struct file_swap_handle
    {
        int64 id;
        string filename;
        std::shared_ptr<fstream> sp_fs;
    };

public:


    bool send_file(const sp_channel &channel,tpsend psend,string file)
    {
        size_t max_length = 0;
        size_t count_length = 0;

        std::fstream fs(file,std::ios::in);
        if(fs.is_open())
        {
            fs.seekg(0,std::ios::end);
            max_length =  fs.tellg();
            fs.seekg(0,std::ios::beg);

            char buf[1024];
            while(fs.eof() == false)
            {
                if(channel->writeBufsize() >= WRITE_BUFSIZE_HIGH_WATER)
                {
                    std::this_thread::sleep_for(std::chrono::milliseconds(50));
                }
                else
                {
                    fs.read(buf,sizeof(buf));
                    int send_size = fs.tellg();
                    channel->send(buf,sizeof(send_size));
                    count_length += send_size;
                }
            }
            fs.close();
        }

        if((max_length != 0) && (count_length == max_length)) return true;
        return false;
    }
};


#endif //WEB_FILE_SWAP_H
