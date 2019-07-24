#pragma once
#include <QOpenGLWidget>
#include <QOpenGLFunctions_2_0>
#include <QMatrix4x4>
#include "I420Image.h"
class subFrame : public QOpenGLWidget, public QOpenGLFunctions_2_0 //, public Player
{
public:
    subFrame(QWidget *parent = 0);
    ~subFrame();
    void setFrame(uint8_t* frame,int,int);
    void updateFrame() { update(); };
protected:

    virtual void initializeGL() override;
    virtual void paintGL() override;
    virtual void resizeGL(int w, int h) override;
    void InitShaders();
private:
    int width_ = 0;
    int height_ = 0;
    uint8_t* frame = nullptr;
    GLuint program;
    GLuint tex_y, tex_u, tex_v;
    GLuint sampler_y, sampler_u, sampler_v;
    GLuint matWorld, matView, matProj;
    QMatrix4x4 mProj;
    QMatrix4x4 mView;
    QMatrix4x4 mWorld;
};

