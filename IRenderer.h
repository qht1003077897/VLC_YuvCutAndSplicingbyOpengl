#pragma once
#include "frame.h"
class __declspec(dllexport) IRenderer
{
public:
    IRenderer();
    virtual ~IRenderer();

public:
    virtual void initialize() = 0;
    virtual void render() = 0;
    virtual void update(Frame * ptr) = 0;

    virtual void setBrightness(double value) = 0;
    virtual void setContrast(double value) = 0;
    virtual void setSaturation(double value) = 0;
    virtual void setFeather(double value) = 0;

    virtual void setCropL(int value) = 0;
    virtual void setCropR(int value) = 0;
    virtual void setCropT(int value) = 0;
    virtual void setCropB(int value) = 0;
    virtual float getBrightness() = 0;
    virtual float getContrast() = 0;
    virtual float getSaturation() = 0;
    virtual float getFeather() = 0;
    virtual int cropL() = 0;
    virtual int cropR() = 0;
    virtual int cropT() = 0;
    virtual int cropB() = 0;
};

