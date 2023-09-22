#ifndef MAKE_JSON_H
#define MAKE_JSON_H

#include <string>

#include "../include/nlohmann/json.hpp"

using std::string;
typedef nlohmann::json json;

class make_json
{
public:
    bool parse_swap(const string &sjson);


};


#endif //MAKE_JSON_H
