#ifndef _CLAZZ_SHARP_H_
#define _CLAZZ_SHARP_H_

#include <iostream>

using namespace std;


class Sharp
{
protected:
    int width;
    int height;
public:
    void setWidth(int w) { width = w; }
    void setHeight(int h) { height = h; }
    virtual int getArea() = 0; // pure virtual function
};

class Rectangle : public Sharp
{
public:
    int getArea() { return (width * height); }
};

class Triangle : public Sharp
{
public:
    int getArea() { return (width * height) / 2; }
};

#endif