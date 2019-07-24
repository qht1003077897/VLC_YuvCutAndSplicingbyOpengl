#pragma once
#include "vlcplayerwidget.h"
#ifdef _WIN32
#include <basetsd.h>
typedef SSIZE_T ssize_t;
#endif
#include "vlc/vlc.h"
#include <QPainter>
#include <QOpenGLFunctions_2_0>
#include <QDebug>

#define ATTRIB_VERTEX 3
#define ATTRIB_TEXTURE 4

VlcPlayerWidget::VlcPlayerWidget(QWidget *parent) :
    QOpenGLWidget(parent),
    m_vlcplayer(NULL),
    m_vlc(NULL),
    m_Front(NULL),
    m_Back(NULL)
{
    const char* vlc_args[32] = {};
    int vlc_args_num = 0;
    vlc_args[vlc_args_num++] = "--network-caching=200";
    vlc_args[vlc_args_num++] = "--file-logging";
    vlc_args[vlc_args_num++] = "--logfile=D:/vlc.log";
    vlc_args[vlc_args_num++] = "--log-verbose=2";
    vlc_args[vlc_args_num++] = "--logmode=text";
    m_vlc = libvlc_new(vlc_args_num, vlc_args);
    //m_vlc = libvlc_new(0, 0);

    m_vlcplayer = libvlc_media_player_new(m_vlc);
    libvlc_video_set_callbacks(m_vlcplayer, lock_cb, unlock_cb, display_cb, this);
    libvlc_video_set_format_callbacks(m_vlcplayer, setup_cb, cleanup_cb);
    ////connect(this, &VlcPlayerWidget::signaldraw, this, &VlcPlayerWidget::slotdraw);
}

VlcPlayerWidget::~VlcPlayerWidget()
{
    stop();
    libvlc_release(m_vlc);
}

void VlcPlayerWidget::setInput(QString input)
{
    m_input = input;
}

void VlcPlayerWidget::play()
{
    stop();
    libvlc_media_t *pmedia = libvlc_media_new_location(m_vlc, m_input.toLocal8Bit().data());

    //libvlc_media_add_option(pmedia, ":rtsp-tcp=true");
    //libvlc_media_add_option(pmedia, ":network-caching=300");
    libvlc_media_player_set_media(m_vlcplayer, pmedia);
    libvlc_media_player_play(m_vlcplayer);

    libvlc_media_release(pmedia);

}

void VlcPlayerWidget::pause()
{
    if (libvlc_media_player_can_pause(m_vlcplayer))
    {
        libvlc_media_player_pause(m_vlcplayer);
    }
}

void VlcPlayerWidget::stop()
{
    libvlc_media_player_stop(m_vlcplayer);

    if (m_Front)
    {
        delete m_Front;
        m_Front = NULL;
    }
    if (m_Back)
    {
        delete m_Back;
        m_Back = NULL;
    }
}


void *VlcPlayerWidget::lock_cb(void *opaque, void **planes)
{
    VlcPlayerWidget *pthis = static_cast<VlcPlayerWidget*>(opaque);

    planes[0] = pthis->m_Back->GetY();
    planes[1] = pthis->m_Back->GetU();
    planes[2] = pthis->m_Back->GetV();


    return pthis->m_Back;
}

void VlcPlayerWidget::unlock_cb(void *opaque, void *picture, void * const *planes)
{
    VlcPlayerWidget *pthis = static_cast<VlcPlayerWidget*>(opaque);

    I420Image* p = pthis->m_Front;
    pthis->m_Front = pthis->m_Back;
    pthis->m_Back = p;

}

void VlcPlayerWidget::display_cb(void *opaque, void *picture)
{
    VlcPlayerWidget *pthis = static_cast<VlcPlayerWidget*>(opaque);

    pthis->update();
}

unsigned VlcPlayerWidget::setup_cb(void **opaque, char *chroma, unsigned *width, unsigned *height, unsigned *pitches, unsigned *lines)
{
    VlcPlayerWidget *pthis = static_cast<VlcPlayerWidget*>(*opaque);
    assert(pthis);

    pthis->m_Front = new I420Image(*width, *height);
    pthis->m_Back = new I420Image(*width, *height);

    pitches[0] = *width;
    lines[0] = *height;

    pitches[1] = pitches[2] = *width / 2;
    lines[1] = lines[2] = *height / 2;

    return 1;
}

void VlcPlayerWidget::cleanup_cb(void *opaque)
{

}

void VlcPlayerWidget::initializeGL()
{
    initializeOpenGLFunctions();
    InitShaders();

}

static const char *vertexShader = "\
attribute vec4 vertexIn;\
attribute vec2 textureIn;\
varying vec2 textureOut;\
uniform mat4 mWorld;\
uniform mat4 mView;\
uniform mat4 mProj;\
void main(void)\
{\
    gl_Position =vertexIn * mWorld * mView * mProj  ;\
    textureOut = textureIn;\
}";

static const char *fragmentShader = "\
varying vec2 textureOut;\
uniform sampler2D tex_y;\
uniform sampler2D tex_u;\
uniform sampler2D tex_v;\
void main(void)\
{\
    vec3 yuv;\
    vec3 rgb;\
    yuv.x = texture2D(tex_y, textureOut).r;\
    yuv.y = texture2D(tex_u, textureOut).r - 0.5;\
    yuv.z = texture2D(tex_v, textureOut).r - 0.5;\
    rgb = mat3( 1,       1,         1,\
                0,       -0.39465,  2.03211,\
                1.13983, -0.58060,  0) * yuv;\
    gl_FragColor = vec4(rgb, 1);\
}";

static const GLfloat vertexVertices[] = {
    -1.0f, -1.0f,
    1.0f, -1.0f,
    -1.0f,  1.0f,
    1.0f,  1.0f,
};

static const GLfloat textureVertices[] = {
    0.0f,  1.0f,
    1.0f,  1.0f,
    0.0f,  0.0f,
    1.0f,  0.0f,
};


void VlcPlayerWidget::paintGL()
{

    // 清除缓冲区
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glClear(GL_COLOR_BUFFER_BIT);

    if (m_Front)
    {
        int w = m_Front->GetWidth();
        int h = m_Front->GetHeight();
        int desW = w / 2;
        int desH = h;
        des = m_Front->GetY();

        dstLeft = new uint8_t[(w * h + w * h / 2) / 2];
        dstRight = new uint8_t[(w * h + w * h / 2) / 2];
        dstTotal = new uint8_t[(w* h + w * h / 2)];
        Cut_I420(des, 0, 0, w, h, dstLeft, desW, desH);
        Cut_I420(des, desW, 0, w, h, dstRight, desW, desH);
        Ver_Con_2(dstLeft, dstRight, desW, desH, dstTotal);
        //烤鸭 3840*2160

         /*Y*/
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, tex_y);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, desW , desH*2, 0, GL_RED, GL_UNSIGNED_BYTE, (GLvoid*)dstTotal);
        glUniform1i(sampler_y, 0);

         /*U*/
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, tex_u);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, desW / 2, desH, 0, GL_RED, GL_UNSIGNED_BYTE, (GLvoid*)(dstTotal + desW * desH * 2));
        glUniform1i(sampler_u, 1);

        /* V*/
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, tex_v);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, desW / 2, desH, 0, GL_RED, GL_UNSIGNED_BYTE, (GLvoid*)(dstTotal + desW * desH * 2 + desW * desH * 2 / 4));
        glUniform1i(sampler_v, 2);


        /*       QOpenGLShaderProgram::setUniformValue();*/
        glUniformMatrix4fv(matWorld, 1, GL_FALSE, mWorld.constData());
        glUniformMatrix4fv(matView, 1, GL_FALSE, mView.constData());
        glUniformMatrix4fv(matProj, 1, GL_FALSE, mProj.constData());

        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        //glFlush();

        if (dstLeft)
        {
            delete []dstLeft;
            dstLeft = nullptr;
        }

        if (dstRight)
        {
            delete []dstRight;
            dstRight = nullptr;
        }
        if (dstTotal)
        {
            delete[]dstTotal;
            dstTotal = nullptr;
        } 
    }
}

void VlcPlayerWidget::resizeGL(int w, int h)
{
//    float viewWidth = 2.0f;
//    float viewHeight = 2.0f;
//
//    mWorld.setToIdentity();
//
//    mView.setToIdentity();
//    mView.lookAt(QVector3D(0.0f, 0.0f, 1.0f), QVector3D(0.f, 0.f, 0.f), QVector3D(0.f, 1.f, 0.f));
//
//    mProj.setToIdentity();
//    if (m_Front)
//    {
//        float aspectRatio = float(m_Front->GetWidth()) / m_Front->GetHeight();
//        //aspectRatio = float(4) / 3; // 强制长宽比
//        if (float(float(w) / h > aspectRatio))
//        {
//            viewHeight = 2.0f;
//            viewWidth = w * viewHeight / (aspectRatio * h);
//        }
//        else
//        {
//            viewWidth = 2.0f;
//            viewHeight = h * viewWidth / (1 / aspectRatio * w);
//        }
//    }
    //留黑边
    //mProj.ortho(-viewWidth / 2, viewWidth / 2, -viewHeight / 2, viewHeight / 2, -1.f, 1.0f);

}

void VlcPlayerWidget::InitShaders()
{
    GLint vertCompiled, fragCompiled, linked;
    GLint v, f;

    //Shader: step1
    v = glCreateShader(GL_VERTEX_SHADER);
    f = glCreateShader(GL_FRAGMENT_SHADER);

    //Shader: step2
    glShaderSource(v, 1, &vertexShader, NULL);
    glShaderSource(f, 1, &fragmentShader, NULL);

    //Shader: step3
    glCompileShader(v);
    glGetShaderiv(v, GL_COMPILE_STATUS, &vertCompiled);    //Debug

    glCompileShader(f);
    glGetShaderiv(f, GL_COMPILE_STATUS, &fragCompiled);    //Debug

    //Program: Step1
    program = glCreateProgram();
    //Program: Step2
    glAttachShader(program, v);
    glAttachShader(program, f);

    glBindAttribLocation(program, ATTRIB_VERTEX, "vertexIn");
    glBindAttribLocation(program, ATTRIB_TEXTURE, "textureIn");
    //Program: Step3
    glLinkProgram(program);
    //Debug
    glGetProgramiv(program, GL_LINK_STATUS, &linked);

    glUseProgram(program);

    //Get Uniform Variables Location
    sampler_y = glGetUniformLocation(program, "tex_y");
    sampler_u = glGetUniformLocation(program, "tex_u");
    sampler_v = glGetUniformLocation(program, "tex_v");

    //Init Texture
    glGenTextures(1, &tex_y);
    glBindTexture(GL_TEXTURE_2D, tex_y);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glGenTextures(1, &tex_u);
    glBindTexture(GL_TEXTURE_2D, tex_u);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glGenTextures(1, &tex_v);
    glBindTexture(GL_TEXTURE_2D, tex_v);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glVertexAttribPointer(ATTRIB_VERTEX, 2, GL_FLOAT, 0, 0, vertexVertices);
    glEnableVertexAttribArray(ATTRIB_VERTEX);

    glVertexAttribPointer(ATTRIB_TEXTURE, 2, GL_FLOAT, 0, 0, textureVertices);
    glEnableVertexAttribArray(ATTRIB_TEXTURE);

    matWorld = glGetUniformLocation(program, "mWorld");
    matView = glGetUniformLocation(program, "mView");
    matProj = glGetUniformLocation(program, "mProj");
}


// YV12和I420裁剪时交换UV分量位置
void VlcPlayerWidget:: Cut_I420(uint8_t* Src, int x, int y, int srcWidth, int srcHeight, uint8_t* Dst, int desWidth, int desHeight)//图片按位置裁剪  
{
    int nIndex = 0;
    int BPosX = x;//列  
    int BPosY = y;//行  
    for (int i = 0; i < desHeight; i++)
    {
        // Dst + desWidth * i  一行一行往下挪
        //Src + (srcWidth*BPosY) + BPosX + nIndex   (BPosY 是Y轴偏移量，一般为0，即为高度上无裁剪，因此就是  Src + BPosX + nIndex，加上 nIndex是因为每次挪一行
        //desWidth  一次拷贝一行
        memcpy(Dst + desWidth * i, Src + (srcWidth*BPosY) + BPosX + nIndex, desWidth);
        nIndex += (srcWidth);
    }

    nIndex = 0;
    uint8_t *pUSour = Src + srcWidth * srcHeight;
    uint8_t *pUDest = Dst + desWidth * desHeight;
    // 此处遍历条件为 i < desHeight / 2 因为src总大小为3/2，而srcWidth * srcHeight只占2/2是存储Y分量的，所有还有1/2的高度是存储UV的
    //YUV420P UV连续存储，先U后V，各占1/4
    for (int i = 0; i < desHeight / 2; i++)//  
    {
        // 不懂为什么这儿都是除2？ 难道存储是一个U一个V一个U一个V？
        memcpy(pUDest + desWidth / 2 * i, pUSour + (srcWidth / 2 * BPosY / 2) + BPosX / 2 + nIndex, desWidth / 2);
        nIndex += (srcWidth / 2);
    }

    nIndex = 0;
    uint8_t *pVSour = Src + srcWidth * srcHeight * 5 / 4;
    uint8_t *pVDest = Dst + desWidth * desHeight * 5 / 4;
    for (int i = 0; i < desHeight / 2; i++)//  
    {
        memcpy(pVDest + desWidth / 2 * i, pVSour + (srcWidth / 2 * BPosY / 2) + BPosX / 2 + nIndex, desWidth / 2);
        nIndex += (srcWidth / 2);
    }
}

void VlcPlayerWidget::Ver_Con_2(uint8_t* Src1, uint8_t* Src2, int dstW, int dstH, uint8_t* Dst)//图片按位置合并
{
    int src1Width = dstW;
    int src1Height = dstH;
    int src2Width = dstW;
    int src2Height = dstH;

    // 先拷贝SRC1和SRC2的Y分量
    int length = (src1Width * src1Height);
    memcpy(Dst, Src1, length);
    int length2 = (src2Width * src2Height);
    memcpy(Dst + length, Src2, length2);

   // SRC1 的U 分量
   int nIndex = 0;
   uint8_t *pUSour1 = Src1 + src1Width * src1Height;
   uint8_t *pUDest1 = Dst + length + length2;
   for (int i = 0; i < src1Height * 2 / 2 / 2; i++)
   {
       memcpy(pUDest1 + src1Width / 2 * i, pUSour1 + nIndex, src1Width / 2);
       nIndex += (src1Width / 2);
   }
   // SRC2 的U 分量
   nIndex = 0;
   uint8_t *pUSour2 = Src2 + src1Width * src1Height;
   uint8_t *pUDest2 = pUDest1 + ((src1Height / 2) * (src1Width / 2)/*后面这一部分是SRC1Y+SRC2Y+SRC1U*/);
   for (int i = 0; i < src1Height * 2 / 2 / 2; i++)
   {
       memcpy(pUDest2 + src2Width / 2 * i, pUSour2 + nIndex, src2Width / 2);
       nIndex += (src2Width / 2);
   }
   // SRC1 的V 分量
   nIndex = 0;
   uint8_t *pVSour1 = Src1 + src1Width * src1Height * 5 / 4;
   uint8_t *pVDest1 = pUDest2 + ((src1Height / 2) * (src2Width / 2)/*后面这一部分是SRC1Y+SRC2Y+SRC1U+SRC2U*/);
   for (int i = 0; i < src1Height * 2 / 2 / 2; i++)
   {
       memcpy(pVDest1 + src1Width / 2 * i, pVSour1 + nIndex, src1Width / 2);
       nIndex += (src1Width / 2);
   }
   // SRC2 的V 分量
   nIndex = 0;
   uint8_t *pVSour2 = Src2 + src2Width * src2Height * 5 / 4;
   uint8_t *pVDest2 = pVDest1 + ((src1Height / 2) * (src1Width / 2)/*后面这一部分是SRC1Y+SRC2Y+SRC1U+SRC2U+SRC1V*/);
   for (int i = 0; i < src2Height * 2 / 2 / 2; i++)
   {
       memcpy(pVDest2 + src1Width / 2 * i, pVSour2 + nIndex, src2Width / 2);
       nIndex += (src2Width / 2);
   }
}