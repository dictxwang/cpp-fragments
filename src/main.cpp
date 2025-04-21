#include <iostream>
#include <thread>
#include <chrono>
#include "util/time.h"
#include "util/log.h"
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

using namespace std;
using namespace rapidjson;

int main() {
    uint64_t now = TimestampInMillisec();
    cout << "Current timestamp in milliseconds: " << now << endl;

    const char* json = "{\"project\":\"rapidjson\",\"stars\":10}";
    Document d;
    d.Parse(json);

    Value& s = d["project"];
    const char* jsonProject = s.GetString();
    cout << "Project: " << jsonProject << endl;

    // init_log();
    // init_console_log();
    init_daily_file_log();

    cout << "Hello, World!" << endl;

    while(true) {
        cout << "Keep Running..." << endl;
        info_log("Main Process Keep Running...");
        std::this_thread::sleep_for(chrono::seconds(3));
    }
    return 0;
}