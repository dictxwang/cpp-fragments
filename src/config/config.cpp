#include "config/config.h"
#include <fstream>
#include <iostream>

Config::Config() {
}

Config::~Config() {
}

bool Config::LoadConfig(const char* inputfile) {
    std::ifstream ifile(inputfile);
    if (!ifile) {
        std::cerr << "Error: Cannot open config file: " << inputfile << std::endl;
        return false;
    }

    ifile.seekg (0, ifile.end);
    int length = ifile.tellg();
    ifile.seekg (0, ifile.beg);

    char * buffer = new char[length+1];
    ifile.read(buffer, length);
    buffer[length] = 0;
    ifile.close();

    doc_.Parse(buffer);
    if (doc_.HasParseError()) {
        std::cerr << "Error: JSON parse error at offset " << doc_.GetErrorOffset() << ": " << doc_.GetParseError() << std::endl;
        return false;
    }

    this->binance_ticker_local_ip = doc_["binance_ticker_local_ip"].GetString();
    this->binance_ticker_ws_ip = doc_["binance_ticker_ws_ip"].GetString();
    this->binance_reset_remote_ip = doc_["binance_reset_remote_ip"].GetString();
    this->binance_reset_local_ip = doc_["binance_reset_local_ip"].GetString();

    return true;
}