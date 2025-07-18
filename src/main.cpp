#include <iostream>
#include <thread>
#include <chrono>
#include <cmath>
#include <cstdint>
#include "util/time.h"
#include "util/log.h"
#include "util/string_helper.h"
#include "clazz/base.h"
#include "clazz/sharp.h"
#include "func/struct.h"
#include "concurrent/thread.h"
#include "concurrent/lock.h"
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

using namespace std;
using namespace rapidjson;

double decimal_process(double value, int decimal) {
    if (decimal == 0) {
        return int(value);
    } else {
        int64_t pow = std::pow(10, decimal);
        return double(int64_t(value * pow)) / double(pow);
    }
}

int calculate_precision_by_min_step(double min_step) {
    if (min_step >= 1) {
        return 0;
    } else {
        int precision = 0;
        do {
            precision = precision + 1;
            min_step = min_step * 10;
        } while (min_step < 1);
        return precision;
    }
}

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

    // constructor and destructor
    cout << "\n<<< constructor and destructor >>>" << endl;
    Base *pBase = new Derived;
    delete pBase;

    cout << "\n<<< pure vitual funtion >>>" << endl;
    // Sharp *pSharp = new Sharp; cannot instantiate abstract class
    Rectangle *pRect = new Rectangle();
    pRect->setWidth(10);
    pRect->setHeight(5);
    cout << "Rectangle area: " << pRect->getArea() << endl;

    Triangle *pTriangle = new Triangle();
    pTriangle->setWidth(10);
    pTriangle->setHeight(5);
    cout << "Triangle area: " << pTriangle->getArea() << endl;
    delete pRect;
    delete pTriangle;

    cout << "\n<<< struct in function >>>" << endl;
    Person p;
    p.name = "John";
    p.age = 30;
    cout << "Person name: " << getPersonName(p) << endl;
    changePersonName(p, "Doe");
    cout << "Person name: " << getPersonName(p) << endl; // name not changed
    changePersonNameByRef(p, "Smith");
    cout << "Person name: " << getPersonName(p) << endl; // name changed
    changePersonNameByPtr(&p, "Jane");
    cout << "Person name: " << getPersonName(p) << endl; // name changed

    cout << "\n<<< basic thread >>>" << endl;
    thread t1(print_delay_message, "DelayMessage: Hello from thread 1", 2);
    thread t2(print_delay_message, "DelayMessage: Hello from thread 2", 4);
    
    // should join threads before exiting main thread, but if main not exit, it will not join
    // if (t1.joinable()) {
    //     t1.join();
    // }
    // if (t2.joinable()) {
    //     t2.join();
    // }

    cout << "\n<<< concurrent thread >>>" << endl;
    thread producers[2];
    thread consumers[2];
    for (int i = 0; i < 2; ++i) {
        producers[i] = thread(producer, i);
        consumers[i] = thread(consumer, i);
    }


    cout << "\nMain Process Start" << endl;


    string dv = "922327.00000000";
    cout << "dv0: " << std::stod(dv) << endl;
    dv = "0.00001000";
    cout << "dv1: " << std::stod(dv) << endl;
    dv = "100000.00000000";
    cout << "dv2: " << std::stod(dv) << endl;
    dv = "0.00010000";
    cout << "dv3: " << std::stod(dv) << endl;

    // Initialize the buffer
    char buffer[] = "Hello, World!";
    std::cout << "Original: " << buffer << std::endl;

    // Number of characters to remove from the start
    size_t chars_to_remove = 7;

    // Get the length of the string
    size_t len = strlen(buffer);

    if (chars_to_remove >= len) {
        // If removing more characters than the string length, make it empty
        buffer[0] = '\0';
    } else {
        // Use std::memmove to shift memory
        std::memmove(buffer, buffer + chars_to_remove, len - chars_to_remove + 1);
    }

    std::cout << "Modified: " << buffer << std::endl;

    std::cout << "Bool value for true: " << std::to_string(true) << std::endl;  // Ouput: 1
    std::cout << "Bool toString for true: " << strHelper::toString(true) << std::endl;  // Ouput: 1

    double pi = 3.14159261415926;
    double pi_2 = decimal_process(pi, 0);
    double pi_3 = decimal_process(pi, 5);
    std::cout << std::setprecision(8) << "pi_2=" << pi_2 << ",pi_3=" << pi_3 <<std::endl;

    int precision_1 = calculate_precision_by_min_step(1);
    int precision_2 = calculate_precision_by_min_step(0.001);
    std::cout << "precision_1=" << precision_1 << ",precision_2=" << precision_2 <<std::endl;

    string message = fmt::format("hello {} is {} kg", "world", 12345);
    std::cout << "<<<<<<<<<< message is " << message << std::endl;

    string double_text = "9.33800000";
    std::cout << "convert double text: " << std::stod(double_text) << std::endl;

    while(true) {
        cout << "Keep Running..." << endl;
        info_log("Main Process Keep Running...");
        std::this_thread::sleep_for(chrono::seconds(3));
    }
    return 0;
}