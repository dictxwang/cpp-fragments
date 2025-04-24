#ifndef _CLAZZ_BASE_H_
#define _CLAZZ_BASE_H_

#include <iostream>

using namespace std;

class Base {
private:
    int *b;
public:
    Base(/* args */);
    virtual ~Base(); // base destructor should define 'virtual', otherwise, it will not call derived destructor 
};

Base::Base(/* args */) {
    b = new int[10];
    cout << "base constructor" << endl;
}

Base::~Base() {
    delete[] b;
    cout << "base destructor" << endl;
}

class Derived : public Base {
private:
    int *d;
public:
    Derived(/* args */) {
        d = new int[10];
        cout << "derived constructor" << endl;
    }
    ~Derived() {
        delete[] d;
        cout << "derived destructor" << endl;
    }
};

#endif