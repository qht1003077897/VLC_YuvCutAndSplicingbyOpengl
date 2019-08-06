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
#include <iostream>
#include <algorithm>
using namespace std;

#define ATTRIB_VERTEX 3
#define ATTRIB_TEXTURE 4

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


VlcPlayerWidget::VlcPlayerWidget(QWidget *parent) :
    QOpenGLWidget(parent),
    m_vlcplayer(NULL),
    m_vlc(NULL),
    m_Front(NULL),
    m_Back(NULL)
{
   //this->setWindowFlag(Qt::FramelessWindowHint);
   //this->setWindowFlags(this->windowFlags() | Qt::WindowStaysOnTopHint);
	setGeometry(0,0, 800, 600);// 无标题栏全屏模式下高度不能超过屏幕高度（高度设置不能超过屏幕分辨率1080 ，底部任务栏19，因此带标题栏时不能超过1061）
    m_vlc = libvlc_new(0, 0);

    m_vlcplayer = libvlc_media_player_new(m_vlc);
    libvlc_video_set_callbacks(m_vlcplayer, lock_cb, unlock_cb, display_cb, this);
    libvlc_video_set_format_callbacks(m_vlcplayer, setup_cb, cleanup_cb);
	for (int i =0; i < 10; i++)	 // 最多10折
	{
		appendList.push_back(nullptr);
		tempList.push_back(nullptr);
	}
}

VlcPlayerWidget::~VlcPlayerWidget()
{
    stop();
    libvlc_release(m_vlc);
	for (auto dst : dstList)
	{
		if (dst)
		{
			delete dst;
			dst = nullptr;
		}
	}
	for (auto append : appendList)
	{
		if (append)
		{
			delete append;
			append = nullptr;
		}
	}
	for (auto temp : tempList)
	{
		if (temp)
		{
			delete temp;
			temp = nullptr;
		}
	}
}

void VlcPlayerWidget::setInput(QString input)
{
    m_input = input;
}

void VlcPlayerWidget::play()
{
    widgetWidth = this->width();
    widgetHeight = this->height();
	initFond();
	int length = 0;

	if (m_fold.orientation == HORIZONTAL)
	{
		m_count = m_fold.count;
		m_firstLeftMargin = m_fold.layoutItemns.at(0).x;
		if (m_count == 1)
		{
			m_lastRightMargin = widgetWidth - m_firstLeftMargin - m_fold.layoutItemns.at(m_fold.layoutItemns.size() - 1).width;
		}
		else
		{
			m_lastRightMargin = widgetWidth - m_fold.layoutItemns.at(m_fold.layoutItemns.size() - 1).width;
		}
	}

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
    for (auto dst : dstList)
    {
        delete dst;
		dst = nullptr;
    }
    dstList.clear();
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

void VlcPlayerWidget::paintGL()
{
    // 清除缓冲区
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glClear(GL_COLOR_BUFFER_BIT);

        if (m_Front)
        {
            des = m_Front->GetY();
#ifdef QT_NO_DEBUG 
			//release 发布时避免第一帧无数据红屏
				if (*des == '\0')
				{
					qDebug() << "data frame  Uninitialized completion  return";
					return;
				}
#endif
        int w = m_Front->GetWidth();
        int h = m_Front->GetHeight();
		int srcLength = 0;
		if (m_fold.orientation == HORIZONTAL)
		{
			for (auto layout : m_fold.layoutItemns)
			{
				srcLength += layout.width;
			}
			if (0 == Video2WidgetRation_W || 0 == Video2WidgetRation_H)
			{
				Video2WidgetRation_W = (double)w / srcLength;
				Video2WidgetRation_H = (double)h / m_fold.layoutItemns[0].height;
			}
		}
		else
		{
			for (auto layout : m_fold.layoutItemns)
			{
				srcLength += layout.height;
			}
			if (0 == Video2WidgetRation_W || 0 == Video2WidgetRation_H)
			{
				Video2WidgetRation_W = (double)w / m_fold.layoutItemns[0].width;
				Video2WidgetRation_H = (double)h / srcLength;
			}
		}

        initializeArrays(w, h, srcLength);
        cutByfondCount(w, h);
		mergeBlock();
		jointVideo();
		drawFrame();

        glUniformMatrix4fv(matWorld, 1, GL_FALSE, mWorld.constData());
        glUniformMatrix4fv(matView, 1, GL_FALSE, mView.constData());
        glUniformMatrix4fv(matProj, 1, GL_FALSE, mProj.constData());
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		glFlush();
    }
}

void VlcPlayerWidget::drawFrame()
{
	int desW = 0;
	int desH = 0;
	if (EnumOrientation::HORIZONTAL == m_fold.orientation)
	{
		desW = (widgetWidth * Video2WidgetRation_W);
		desH = (widgetHeight * Video2WidgetRation_H);
	}
	else
	{
		desW = m_fold.layoutItemns.at(0).width * Video2WidgetRation_W * dstList.size();
		desH = m_fold.layoutItemns.at(0).height * Video2WidgetRation_H;
	}
		/*Y*/
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, tex_y);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, desW, desH, 0, GL_RED, GL_UNSIGNED_BYTE, (GLvoid*)dstTotal);
		glUniform1i(sampler_y, 0);

		///*U*/
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, tex_u);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, desW / 2, desH / 2, 0, GL_RED, GL_UNSIGNED_BYTE, (GLvoid*)(dstTotal + desW * desH));
		glUniform1i(sampler_u, 1);

		///*V*/
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, tex_v);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, desW / 2, desH / 2, 0, GL_RED, GL_UNSIGNED_BYTE, (GLvoid*)(dstTotal + desW  * desH * 5 / 4));
		glUniform1i(sampler_v, 2);
}

//将裁剪好的视频块进行拼接
void VlcPlayerWidget::jointVideo()
{
	std::vector<DstData> connectList;
	//for (int i = 0; i < dstList.size(); i++)
	//{
	//	DstData dst;
	//	dst.data = dstList.at(i);
	//	dst.dstW = widgetWidth * Video2WidgetRation_W;
	//	dst.dstH = m_fold.layoutItemns.at(i).height * Video2WidgetRation_H;
	//	connectList.push_back(dst);
	//}
		for (int i = 0; i < tempList.size(); i++)
	{
			if (tempList[i])
			{
				DstData dst;
				dst.data = tempList[i];
				dst.dstW = widgetWidth * Video2WidgetRation_W;
				dst.dstH = m_fold.layoutItemns.at(i).height * Video2WidgetRation_H;
				connectList.push_back(dst);
			}
	}
	
	if (m_fold.orientation == HORIZONTAL)
	{
		connectI420Ver(connectList, dstTotal);
	}
	else
	{
		connectI420Hon(connectList, dstTotal);
	}
}

void VlcPlayerWidget::initFond()
{
	m_fold.screenWidth = 1920;
	m_fold.screenHeight = 200;
	m_fold.count = 3;
	m_fold.enable = true;
	m_fold.orientation = HORIZONTAL;
	std::vector<Layout> vectors;
	Layout layout1;
	layout1.x = 107;
	layout1.y = 0;
	layout1.width = 693;
	layout1.height = 200;

	Layout layout2;
	layout2.x = 0;
	layout2.y = 0;
	layout2.width = 800;
	layout2.height = 200;

	//Layout layout3;
	//layout3.x = 0;
	//layout3.y = 0;
	//layout3.width = 560;
	//layout3.height = 200;
	Layout layoutLast;
	layoutLast.x = 0;
	layoutLast.y = 0;
	layoutLast.width = 427;// （ 不支持奇数）
	layoutLast.height = 200;

	int dValue = 0;
	if (layout1.width % 4 != 0)
	{
		dValue = 4 - (layout1.width % 4);
		layout1.width += dValue;
		layout1.x = widgetWidth - layout1.width;
		layoutLast.width -= dValue;
	}

	vectors.push_back(layout1);
	vectors.push_back(layout2);
	vectors.push_back(layoutLast);

	m_fold.layoutItemns = vectors;
}

void VlcPlayerWidget::initializeArrays(int w, int h,int srcLength)
{
	int size = m_fold.layoutItemns.size();
	if (0 == dstList.size())
	{
		if (m_fold.orientation == HORIZONTAL)
		{
			for (int i = 0; i < size; i++)
			{
				uint8_t *dst = new uint8_t[ceil((w * h + w * h / 2) *  (double)(m_fold.layoutItemns.at(i).width) / srcLength)];
				dstList.push_back(dst);
			}
		}
		else
		{
			for (int i = 0; i < size; i++)
			{
				uint8_t *dst = new uint8_t[ceil(((w * h + w * h / 2) * (double)(m_fold.layoutItemns.at(i).height)) / srcLength)];
				dstList.push_back(dst);
			}
		}
		dstTotal = new uint8_t[ceil(widgetWidth * Video2WidgetRation_W * widgetHeight * Video2WidgetRation_H * 3 / 2)];
	}
}

//按打折数裁剪视频
void VlcPlayerWidget::cutByfondCount(int w, int h)
{
	int blockX = 0; // 裁剪前每块的X偏移量
	int blockY = 0; // 裁剪前每块的Y偏移量
	int index = 1;
	for (auto layout : m_fold.layoutItemns)
	{
			if (EnumOrientation::HORIZONTAL == m_fold.orientation)			{
				Cut_I420(
					des,
					blockX,                                                             // 裁剪目标块X偏移()
					0,                                                                  // 裁剪目标块y偏移（后续支持XY偏移量之后更改此参数即可）
					w, h,                                                               // 视频源宽高
					dstList.at(index-1),
					floor((m_fold.layoutItemns.at(index - 1).width * Video2WidgetRation_W)),
					floor(m_fold.layoutItemns.at(index - 1).height * Video2WidgetRation_H));           //裁剪目标宽高
				blockX += ((m_fold.layoutItemns.at(index - 1).width) * Video2WidgetRation_W);
			}
			else
			{
				Cut_I420(
					des,
					0,																	 // 裁剪目标块X偏移()
					blockY,                                                              // 裁剪目标块y偏移（后续支持XY偏移量之后更改此参数即可）
					w, h,                                                                // 视频源宽高
					dstList.at(index - 1),
					m_fold.layoutItemns.at(index - 1).width * Video2WidgetRation_W,
					m_fold.layoutItemns.at(index - 1).height * Video2WidgetRation_H);           //裁剪目标宽高
				blockY += (m_fold.layoutItemns.at(index - 1).height) * Video2WidgetRation_H;
			}

		index++;
	}
}

void VlcPlayerWidget::mergeBlock()
{
	int increase = 0;
	if (m_fold.orientation == HORIZONTAL)
	{
		if (m_count == 1)
		{
			// 打1折，设置首行偏移,暂不考虑1折
		}
		else
		{
			int desH = m_fold.layoutItemns.at(0).height * Video2WidgetRation_H;
			for (int i =0;i<m_count;i++)
			{
				int desW = 0;
				bool left = false;
				bool right = false;
				if (m_fold.layoutItemns.at(i).x != 0)
				{
					left = true;
					desW = m_fold.layoutItemns.at(i).x * Video2WidgetRation_W;
					if (appendList.at(i) == nullptr)
					{
						uint8_t *appendLeft = new uint8_t[ceil(desW * desH * 3 / 2)];
						memset(appendLeft, 0x80, desW * desH * 3 / 2);
						appendList[i] = appendLeft;
					}
				}
				if (widgetWidth - m_fold.layoutItemns.at(i).x - m_fold.layoutItemns.at(i).width != 0)
				{
					right = true;
					desW = (widgetWidth - m_fold.layoutItemns.at(i).x - m_fold.layoutItemns.at(i).width) * Video2WidgetRation_W;
					if (appendList.at(i) == nullptr)
					{
						uint8_t *appendRight = new uint8_t[ceil(desW * desH * 3 / 2)];
						memset(appendRight, 0x80, desW * desH * 3 / 2);
						appendList[i] = appendRight;
					}
				}
				if (!left && !right)
				{
					tempList[i] = dstList[i];
					continue;
				}
				std::vector<DstData> connectList;
				uint8_t *src = dstList.at(i);

				DstData dstsrc;
				dstsrc.data = src;
				dstsrc.dstW = (m_fold.layoutItemns.at(i).width * Video2WidgetRation_W);
				dstsrc.dstH = (m_fold.layoutItemns.at(i).height * Video2WidgetRation_H);

				DstData dstappend;
				dstappend.data = appendList.at(i);
				dstappend.dstW = desW;
				dstappend.dstH = desH;
				if (left)
				{
					connectList.push_back(dstappend);
					connectList.push_back(dstsrc);
				}
				else if(right)
				{
					connectList.push_back(dstsrc);
					connectList.push_back(dstappend);
				}

				if (nullptr == tempList.at(i))
				{
					uint8_t* tempData = new uint8_t[ceil((desW + m_fold.layoutItemns.at(i).width * Video2WidgetRation_W) * desH * 3 / 2)];
					tempList[i] = tempData;
				}
				connectI420Hon(connectList, tempList[i]);
			}
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
    for (int i = 0; i < desHeight >> 1; i++)//  
    {
        memcpy(pUDest + (desWidth >> 1) * i, pUSour + ((srcWidth >> 1) * (BPosY >> 1)) + (BPosX >> 1) + nIndex,desWidth >> 1);
        nIndex += (srcWidth >> 1);
    }

    nIndex = 0;
    uint8_t *pVSour = Src + srcWidth * srcHeight * 5 / 4;
    uint8_t *pVDest = Dst + desWidth * desHeight * 5 / 4;
    for (int i = 0; i < desHeight >> 1; i++)//  
    {
        memcpy(pVDest + (desWidth >> 1) * i, pVSour + ((srcWidth >> 1) * (BPosY >> 1)) + (BPosX >> 1) + nIndex, desWidth >> 1);
        nIndex += (srcWidth >> 1);
    }
}

void VlcPlayerWidget::connectI420Ver(std::vector<DstData> disList, uint8_t* Dst)
{

    // 依次拷贝N块数据的Y分量
    int increaseY = 0;  // Y分量每次递增
    for (auto dstData : disList)
    {
        memcpy(Dst + increaseY, dstData.data, (dstData.dstW * dstData.dstH));
        increaseY += (dstData.dstW * dstData.dstH);
    }

    // 依次拷贝N块数据的U分量
    int increaseU = 0;
    for (auto dstData : disList)
    {
       int nIndex = 0;
        uint8_t *pUSour = dstData.data + dstData.dstW * dstData.dstH;
        uint8_t *pUDest = Dst + increaseY + increaseU;
        for (int i = 0; i < dstData.dstH >> 1; i++)
        {
            memcpy(pUDest + (dstData.dstW >> 1) * i, pUSour + nIndex, dstData.dstW >> 1);
            nIndex += (dstData.dstW >> 1);
        }
        increaseU += (dstData.dstW >> 1) * (dstData.dstH >> 1);
    }

    // 依次拷贝N块数据的V分量
    int increaseV = 0;
    for (auto dstData : disList)
    {
        int nIndex = 0;
        uint8_t *pVSour = dstData.data + dstData.dstW * dstData.dstH * 5 / 4;
        uint8_t *pVDest = Dst + increaseY + increaseU + increaseV;
        for (int i = 0; i < dstData.dstH >> 1; i++)
        {
            memcpy(pVDest + (dstData.dstW >> 1) * i, pVSour + nIndex, dstData.dstW >> 1);
            nIndex += (dstData.dstW >> 1);
        }
		increaseV += (dstData.dstW >> 1) * (dstData.dstH >> 1);
    }
}

void VlcPlayerWidget::connectI420Hon(std::vector<DstData> &disList, uint8_t * Dst)
{
	int nOffY = 0;		// 当前要拼接图像的Y偏移
	int nOffX = 0;		// 当前要拼接图像的X偏移
	UINT nOff = 0;
	int allWidth = 0;
	for (auto dstData : disList)
	{
		allWidth += dstData.dstW;
	}
	// 拷贝 Y 分量
	for (auto dstData : disList)
	{
		for (int i = 0; i < dstData.dstH; i++)
		{
			// Y轴偏移（体积）,移动到新的点，再弥补上后面几份的宽度， 再加上X 偏移（长度） 可以定位当前要拼接图像的起始点(每段等宽)
			nOff = dstData.dstW * nOffY + allWidth * i  + nOffX;
			//逐行拷贝	
			memcpy(Dst + nOff, dstData.data + dstData.dstW * i, dstData.dstW);
		}
		nOffX += dstData.dstW;
	}

	// 拷贝 U 分量
	int YTotal = 0;
	nOffX = 0;
	nOffY = 0;
	nOff = 0;
	int nIndex = 0;
	for (auto dstData : disList)
	{
		YTotal += dstData.dstW * dstData.dstH;
	}

	for (auto dstData : disList)
	{
		int nIndex = 0;
		for (int i = 0; i < (dstData.dstH) >> 1; i++)
		{
			nOff = (dstData.dstW >> 1) * (nOffY >> 1) + (allWidth >> 1) * i + (nOffX);
			//逐行拷贝
			 memcpy(Dst + nOff + YTotal, dstData.data + (dstData.dstW * dstData.dstH) + nIndex, dstData.dstW >> 1);
			// 拷贝 V 分量
			memcpy(Dst + nOff + YTotal * 5 / 4, dstData.data + (dstData.dstW * dstData.dstH * 5 / 4) + nIndex, dstData.dstW >> 1);
			nIndex += (dstData.dstW >> 1);
		}
		nOffX += dstData.dstW >> 1;
	}
}
