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
#include "QTimer"
#include <math.h>

#define ATTRIB_VERTEX 3
#define ATTRIB_TEXTURE 4


VlcPlayerWidget::VlcPlayerWidget(QWidget *parent) :
    QOpenGLWidget(parent),
    m_vlcplayer(NULL),
    m_vlc(NULL),
    m_Front(NULL),
    m_Back(NULL)
{
   setGeometry(0, 0, 800, 800);


    m_fold.count = 2;
    m_fold.enable = true;
    m_fold.orientation = HORIZONTAL;
    Layout layout1;
    layout1.x = 0;
    layout1.y = 0;
    layout1.width = 400;
    layout1.height = 800;

    Layout layout2;
    layout2.x = 0;
    layout2.y = 800;
    layout2.width = 400;
    layout2.height = 800;

    Layout layout3;
    //layout3.x = 0;
    //layout3.y = 1600;
    //layout3.width = 400;
    //layout3.height = 800;
    std::vector<Layout> vectors;
    vectors.push_back(layout1);
    vectors.push_back(layout2);
    //vectors.push_back(layout3);
    m_fold.layoutItemns = vectors;



    //const char* vlc_args[32] = {};
    //int vlc_args_num = 0;
    //vlc_args[vlc_args_num++] = "--network-caching=200";
    ////vlc_args[vlc_args_num++] = "--file-logging";
    ////vlc_args[vlc_args_num++] = "--logfile=D:/vlc.log";
    ////vlc_args[vlc_args_num++] = "--log-verbose=2";
    ////vlc_args[vlc_args_num++] = "--logmode=text";
    //m_vlc = libvlc_new(vlc_args_num, vlc_args);
    m_vlc = libvlc_new(0, 0);

    m_vlcplayer = libvlc_media_player_new(m_vlc);
    libvlc_video_set_callbacks(m_vlcplayer, lock_cb, unlock_cb, display_cb, this);
    libvlc_video_set_format_callbacks(m_vlcplayer, setup_cb, cleanup_cb);
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
    widgetWidth = this->width();
    widgetHeight = this->height();
    stop();
    libvlc_media_t *pmedia = libvlc_media_new_location(m_vlc, m_input.toLocal8Bit().data());

    libvlc_media_add_option(pmedia, ":rtsp-tcp=true");
    libvlc_media_add_option(pmedia, ":network-caching=300");
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
    // ���������
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glClear(GL_COLOR_BUFFER_BIT);

        if (m_Front)
        {
            des = m_Front->GetY();
            int w = m_Front->GetWidth();
            int h = m_Front->GetHeight();
            int size = m_fold.layoutItemns.size();
            double Video2WidgetRation_W = (double)w / (double)widgetWidth;
            double Video2WidgetRation_H = (double)h / (double)widgetHeight;
            if (0 == dstList.size())
            {
                for (int i = 0; i < size; i++)
                {
                    double si= (w * h + w * h / 2) / size;
                    int s = ceil(si);
                    uint8_t *dst = new uint8_t[ceil((w * h + w * h / 2) / size)];
                    dstList.push_back(dst);
                }
                dstTotal = new uint8_t[(w * h + w * h / 2)];
            }
#pragma region  ���������ü���Ƶ   
            
            int blockX = 0; // �ü�ǰÿ���Xƫ����
            int index = 0;
             for (auto layout : m_fold.layoutItemns)
            {
                if (0 != index)
                {
                    blockX += (m_fold.layoutItemns.at(index - 1).width) * Video2WidgetRation_W;
                }
                Cut_I420(
                    des,
                    blockX,                                                             // �ü�Ŀ���Xƫ��()
                    0,                                                                  // �ü�Ŀ���yƫ�ƣ�����֧��XYƫ����֮����Ĵ˲������ɣ�
                    w, h,                                                               // ��ƵԴ���
                    dstList.at(index),
                    m_fold.layoutItemns.at(index).width * Video2WidgetRation_W,
                    m_fold.layoutItemns.at(index).height * Video2WidgetRation_H);           //�ü�Ŀ����
                index++;
            }
#pragma endregion

#pragma region  ���ü��õ���Ƶ���������ƴ��  
             index = 0;
             for (auto layout : m_fold.layoutItemns) 
             {
                 Ver_Con_2(
                     dstList.at(0), dstList.at(1),
                     m_fold.layoutItemns.at(0).width * Video2WidgetRation_W,
                     m_fold.layoutItemns.at(0).height * Video2WidgetRation_H,
                     dstTotal);
                 index++;
             }
#pragma endregion

            int desW = m_fold.layoutItemns.at(0).width * Video2WidgetRation_W;
            int desH = m_fold.layoutItemns.at(0).height * Video2WidgetRation_H;
         /*Y*/
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, tex_y);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, desW, desH * 2 , 0, GL_RED, GL_UNSIGNED_BYTE, (GLvoid*)dstTotal);
        glUniform1i(sampler_y, 0);

         /*U*/
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, tex_u);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, desW / 2, desH, 0, GL_RED, GL_UNSIGNED_BYTE, (GLvoid*)(dstTotal + desW * desH * 2 ));
        glUniform1i(sampler_u, 1);

        /* V*/
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, tex_v);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, desW / 2, desH, 0, GL_RED, GL_UNSIGNED_BYTE, (GLvoid*)(dstTotal + desW * desH * 2  + desW * desH * 2 / 4));
        glUniform1i(sampler_v, 2);

        glUniformMatrix4fv(matWorld, 1, GL_FALSE, mWorld.constData());
        glUniformMatrix4fv(matView, 1, GL_FALSE, mView.constData());
        glUniformMatrix4fv(matProj, 1, GL_FALSE, mProj.constData());

        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        glFlush();
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
//        //aspectRatio = float(4) / 3; // ǿ�Ƴ����
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
    //���ڱ�
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

// YV12��I420�ü�ʱ����UV����λ��
void VlcPlayerWidget:: Cut_I420(uint8_t* Src, int x, int y, int srcWidth, int srcHeight, uint8_t* Dst, int desWidth, int desHeight)//ͼƬ��λ�òü�  
{
    int nIndex = 0;
    int BPosX = x;//��  
    int BPosY = y;//��  
    for (int i = 0; i < desHeight; i++)
    {
        // Dst + desWidth * i  һ��һ������Ų
        //Src + (srcWidth*BPosY) + BPosX + nIndex   (BPosY ��Y��ƫ������һ��Ϊ0����Ϊ�߶����޲ü�����˾���  Src + BPosX + nIndex������ nIndex����Ϊÿ��Ųһ��
        //desWidth  һ�ο���һ��
        memcpy(Dst + desWidth * i, Src + (srcWidth*BPosY) + BPosX + nIndex, desWidth);
        nIndex += (srcWidth);
    }

    nIndex = 0;
    uint8_t *pUSour = Src + srcWidth * srcHeight;
    uint8_t *pUDest = Dst + desWidth * desHeight;
    // �˴���������Ϊ i < desHeight / 2 ��Ϊsrc�ܴ�СΪ3/2����srcWidth * srcHeightֻռ2/2�Ǵ洢Y�����ģ����л���1/2�ĸ߶��Ǵ洢UV��
    //YUV420P UV�����洢����U��V����ռ1/4
    for (int i = 0; i < desHeight / 2; i++)//  
    {
        // ����Ϊʲô������ǳ�2�� �ѵ��洢��һ��Uһ��Vһ��Uһ��V��
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

void VlcPlayerWidget::Ver_Con_2(uint8_t* Src1, uint8_t* Src2, int dstW, int dstH, uint8_t* Dst)//ͼƬ��λ�úϲ�
{
    int src1Width = dstW;
    int src1Height = dstH;
    int src2Width = dstW;
    int src2Height = dstH;

    // �ȿ���SRC1��SRC2��Y����
    int length = (src1Width * src1Height);
    memcpy(Dst, Src1, length);
    int length2 = (src2Width * src2Height);
    memcpy(Dst + length, Src2, length2);

   // SRC1 ��U ����
   int nIndex = 0;
   uint8_t *pUSour1 = Src1 + src1Width * src1Height;
   uint8_t *pUDest1 = Dst + length + length2;
   for (int i = 0; i < src1Height * 2 / 2 / 2; i++)
   {
       memcpy(pUDest1 + src1Width / 2 * i, pUSour1 + nIndex, src1Width / 2);
       nIndex += (src1Width / 2);
   }
   // SRC2 ��U ����
   nIndex = 0;
   uint8_t *pUSour2 = Src2 + src1Width * src1Height;
   uint8_t *pUDest2 = pUDest1 + ((src1Height / 2) * (src1Width / 2)/*������һ������SRC1Y+SRC2Y+SRC1U*/);
   for (int i = 0; i < src1Height * 2 / 2 / 2; i++)
   {
       memcpy(pUDest2 + src2Width / 2 * i, pUSour2 + nIndex, src2Width / 2);
       nIndex += (src2Width / 2);
   }
   // SRC1 ��V ����
   nIndex = 0;
   uint8_t *pVSour1 = Src1 + src1Width * src1Height * 5 / 4;
   uint8_t *pVDest1 = pUDest2 + ((src1Height / 2) * (src2Width / 2)/*������һ������SRC1Y+SRC2Y+SRC1U+SRC2U*/);
   for (int i = 0; i < src1Height * 2 / 2 / 2; i++)
   {
       memcpy(pVDest1 + src1Width / 2 * i, pVSour1 + nIndex, src1Width / 2);
       nIndex += (src1Width / 2);
   }
   // SRC2 ��V ����
   nIndex = 0;
   uint8_t *pVSour2 = Src2 + src2Width * src2Height * 5 / 4;
   uint8_t *pVDest2 = pVDest1 + ((src1Height / 2) * (src1Width / 2)/*������һ������SRC1Y+SRC2Y+SRC1U+SRC2U+SRC1V*/);
   for (int i = 0; i < src2Height * 2 / 2 / 2; i++)
   {
       memcpy(pVDest2 + src1Width / 2 * i, pVSour2 + nIndex, src2Width / 2);
       nIndex += (src2Width / 2);
   }
}