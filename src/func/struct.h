#ifndef _FUNC_STRUCT_H_
#define _FUNC_STRUCT_H_

#include <iostream>
using namespace std;    

struct Person {
    string name;
    int age;
};

string getPersonName(Person p) {
    return p.name;
}

void changePersonName(Person p, string name) {
    p.name = name;
}

void changePersonNameByRef(Person &p, string name) {
    p.name = name;
}

void changePersonNameByPtr(Person *p, string name) {
    p->name = name;
}

#endif