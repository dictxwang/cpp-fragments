#include <iostream>
#include <thread>
#include <chrono>
#include "util/time.h"
#include "util/log.h"
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

    while(true) {
        cout << "Keep Running..." << endl;
        info_log("Main Process Keep Running...");
        std::this_thread::sleep_for(chrono::seconds(3));
    }
    return 0;
}