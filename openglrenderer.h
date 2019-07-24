////.h
//#pragma once
//#include <QOpenGLVertexArrayObject>
//#include <QOpenGLBuffer>
//#include <QOpenGLShaderProgram>
//#include <QOpenGLTexture>
//#include <qopenglfunctions_4_3_core.h>
//#include <mutex>
//#include <QOpenGLFunctions>
//#include <QOpenGLWidget>
//#include <QOpenGLFunctions_2_0>
//#include "IRenderer.h"
//#include "frame.h"
//class __declspec(dllexport) RgbRenderer : protected QOpenGLFunctions, public IRenderer
//{
//public:
//
//    RgbRenderer();
//    ~RgbRenderer();
//
//    void initialize();
//    void render();
//    void update(Frame * ptr);
//
//    void setBrightness(double value);
//    void setContrast(double value);
//    void setSaturation(double value);
//    void setFeather(double value);
//
//    void setCropL(int value);
//    void setCropR(int value);
//    void setCropT(int value);
//    void setCropB(int value);
//
//    float getBrightness();
//    float getContrast();
//    float getSaturation();
//    float getFeather();
//
//    int cropL();
//    int cropR();
//    int cropT();
//    int cropB();
//
//private:
//    QOpenGLShaderProgram *program;
//    QOpenGLBuffer vbo;
//    GLuint textureUniformRgb;
//    QOpenGLTexture *textureRgb;
//    GLuint idRgb;
//
//    std::mutex m_mutex;
//    Frame * m_currentFrame;
//
//    float m_dbrightness;
//    float m_dcontrast;
//    float m_dsaturation;
//    float m_dfeather;
//
//    float m_scale;
//    float m_rotation;
//    int m_cropL, m_cropR, m_cropT, m_cropB;
//
//    bool m_bFrameUpdated;
//};
//
//class __declspec(dllexport) OpenGLRenderer : protected QOpenGLFunctions_4_3_Core, public IRenderer
//{
//public:
//    OpenGLRenderer();
//    ~OpenGLRenderer();
//
//public:
//    void initialize();
//    void render();
//    void update(Frame * ptr);
//
//    void setBrightness(double value);
//    void setContrast(double value);
//    void setSaturation(double value);
//    void setFeather(double value);
//
//    void setCropL(int value);
//    void setCropR(int value);
//    void setCropT(int value);
//    void setCropB(int value);
//
//    float getBrightness();
//    float getContrast();
//    float getSaturation();
//    float getFeather();
//
//    int cropL();
//    int cropR();
//    int cropT();
//    int cropB();
//
//private:
//    void caculateCrop();
//    void cropChanged();
//
//    void scale();
//
//    QOpenGLVertexArrayObject vao;
//    QOpenGLShaderProgram program;
//    QOpenGLBuffer vbo;
//    std::mutex m_mutex;
//    Frame m_preFrame;
//    Frame* m_currentFrame;
//
//    GLuint idY, idU, idV;
//
//    GLuint texUniformY, texUniformU, texUniformV;
//    GLint brightnessUniform;
//    GLint contrastUniform;
//    GLint saturationUniform;
//    GLint featherUniform;
//    GLint scaleUniform;
//    GLint rotationUniform;
//    GLint cropUniform;
//
//    float m_dbrightness;
//    float m_dcontrast;
//    float m_dsaturation;
//    float m_dfeather;
//
//    float m_scale;
//    float m_rotation;
//    int m_cropL, m_cropR, m_cropT, m_cropB;
//    QMatrix4x4 cropMat;
//
//    bool m_cropChanged;
//    bool m_bFrameUpdated;
//};
