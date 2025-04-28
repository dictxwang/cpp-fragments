#ifndef _CONFIG_CONFIG_H_
#define _CONFIG_CONFIG_H_   

#include <string>
#include "rapidjson/document.h"

using std::string;

class Config
{
public:
    Config();
    ~Config();
    bool LoadConfig(const char* inputfile);

public:
    string binance_ticker_ws_ip;
    string binance_ticker_local_ip;

private:
    rapidjson::Document doc_;
};
#endif