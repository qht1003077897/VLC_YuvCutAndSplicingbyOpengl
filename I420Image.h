#pragma once
#include <QOpenGLFunctions_2_0>

class I420Image
{
public:
    I420Image(int w, int h)
        : width(w)
        , height(h)
    {
        data = new uint8_t[w*h + w * h / 2];
    }
    ~I420Image()
    {
        delete[]data;
    }

    int GetWidth() const { return width; }
    int GetHeight() const { return height; }
    uint8_t *GetY() const { return data; }
    uint8_t *GetU()const { return data + width * height; }
    uint8_t *GetV()const { return data + width * height + width * height / 4; }
protected:
public:
    int width = 0;
    int height = 0;
    uint8_t * data;
};