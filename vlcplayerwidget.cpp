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
#include "libyuv.h"
using namespace std;

static const char *vertexShader = "\
	#version 430 core\n \
	layout(location = 0) in vec4 vertexIn; \
	layout(location = 1) in vec2 textureIn; \
	out vec2 textureOut;  \
	void main(void)\
	{\
		gl_Position =vertexIn ;\
		textureOut = textureIn;\
	}";

static const char *fragmentShader = "\
#version 430 core\n \
in vec2 textureOut;\
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
    gl_FragColor = vec4(rgb, 1.0);\
}";


VlcPlayerWidget::VlcPlayerWidget(QWidget *parent) :
    QOpenGLWidget(parent),
    m_vlcplayer(NULL),
    m_vlc(NULL),
    m_Front(NULL),
    m_Back(NULL)
{
   //this->setWindowFlag(Qt::FramelessWindowHint);
   //this->setWindowFlags(this->windowFlags() | Qt::WindowStaysOnTopHint);
	setGeometry(0,0, 600, 800);// 无标题栏全屏模式下高度不能超过屏幕高度（高度设置不能超过屏幕分辨率1080 ，底部任务栏19，因此带标题栏时不能超过1061）
    m_vlc = libvlc_new(0, 0);

    m_vlcplayer = libvlc_media_player_new(m_vlc);
    libvlc_video_set_callbacks(m_vlcplayer, lock_cb, unlock_cb, display_cb, this);
    libvlc_video_set_format_callbacks(m_vlcplayer, setup_cb, cleanup_cb);
}
void VlcPlayerWidget::initFond()
{
	for (int i = 0; i < 10; i++)	 // 最多10折
	{
		DstData dt;
		dt.data = nullptr;
		connectList.push_back(dt);
	}
	m_fold.count = 3;
	std::vector<Layout> vectors;
	if (HORIZONTAL == m_fold.orientation)
	{
		Layout layout1;
		layout1.x = 0;
		layout1.y = 0;
		layout1.width = 400;
		layout1.height = 150;

		Layout layout2;
		layout2.x = 0;
		layout2.y = 150;
		layout2.width = 400;
		layout2.height = 150;

		Layout layoutLast;
		layoutLast.x = 0;
		layoutLast.y = 300;
		layoutLast.width = 40;
		layoutLast.height = 150;
		vectors.push_back(layout1);
		vectors.push_back(layout2);
		vectors.push_back(layoutLast);
		oriScreenWidth = 840; //不包含首行偏移的Y值
		oriScreenHeight = 150;
	}
	else
	{
		Layout layout1;
		layout1.x = 0;
		layout1.y = 0;
		layout1.width = 150;
		layout1.height = 390;

		Layout layout2;
		layout2.x = 150;
		layout2.y = 0;
		layout2.width = 150;
		layout2.height = 390;

		Layout layoutLast;
		layoutLast.x = 300;
		layoutLast.y = 0;
		layoutLast.width = 150;
		layoutLast.height = 20;
		vectors.push_back(layout1);
		vectors.push_back(layout2);;
		vectors.push_back(layoutLast);
		oriScreenWidth = 150; //不包含首行偏移的Y值
		oriScreenHeight = 800;
	}
	m_fold.layoutItemns = vectors;

}
VlcPlayerWidget::~VlcPlayerWidget()
{
    stop();
    libvlc_release(m_vlc);
	//for (auto dst : dstList)
	//{
	//	if (dst)
	//	{
	//		delete dst;
	//		dst = nullptr;
	//	}
	//}

}

void VlcPlayerWidget::setVideoFilePath(QString filepath)
{
	m_input = filepath;
}

void VlcPlayerWidget::setCropDirection(EnumOrientation orition)
{
	m_fold.orientation = orition;
	stop();
	initFond();
}

void VlcPlayerWidget::setCropStyle(EnumCropStyle style)
{
	m_fold.style = style;
	stop();
	initFond();
}

void VlcPlayerWidget::play()
{
	m_count = m_fold.count;
	int dValue = 0;
	if (m_fold.orientation == HORIZONTAL)
	{
		widgetWidth = m_fold.layoutItemns[0].width + m_fold.layoutItemns[0].x;		widgetHeight = m_count * m_fold.layoutItemns[0].height;
	}
	else
	{
		widgetHeight = m_fold.layoutItemns[0].height + m_fold.layoutItemns[0].y;		widgetWidth = m_count * m_fold.layoutItemns[0].width;
	}

	QString path = "file:///" + m_input;
    libvlc_media_t *pmedia = libvlc_media_new_location(m_vlc, path.toLocal8Bit().data());

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
	if (dstTotal)
	{
		delete []dstTotal;
		dstTotal = NULL;
	}
	for (auto dst : dstList)
	{
		if (dst.data)
		{
			delete []dst.data;
			dst.data = nullptr;
		}
	}
	for (auto dst : connectList)
	{
		if (dst.data)
		{
			delete[]dst.data;
			dst.data = nullptr;
		}
	}
	if (m_afterScale)
	{
		delete []m_afterScale;
		m_afterScale = NULL;
	}
    dstList.clear();
	connectList.clear();
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
	VlcPlayerWidget *pthis = static_cast<VlcPlayerWidget*>(opaque);
	assert(pthis);
	if (pthis->m_Front)
	{
		delete pthis->m_Front;
		pthis->m_Front = nullptr;
	}
	if (pthis->m_Back)
	{
		delete pthis->m_Back;
		pthis->m_Back = nullptr;
	}
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

#ifdef QT_NO_DEBUG 
			//release 发布时避免第一帧无数据红屏
				if (*m_Front->GetY() == '\0')
				{
					qDebug() << "data frame  Uninitialized completion  return";
					return;
				}
#endif
		int sourceW = m_Front->GetWidth();
		int sourceH = m_Front->GetHeight();
		int srcLength = 0;
		int dstW = 0;
		int dstH = 0;
		if (m_fold.style == OpenGL)
		{
			dstTotal = m_Front->data;
		}
		else
		{
			if (m_fold.orientation == HORIZONTAL)
			{
				for (auto layout : m_fold.layoutItemns)
				{
					srcLength += layout.width;
				}
				dstW = srcLength;
				dstH = m_fold.layoutItemns[0].height;
			}
			else
			{
				for (auto layout : m_fold.layoutItemns)
				{
					srcLength += layout.height;
				}
				dstW = m_fold.layoutItemns[0].width;
				dstH = srcLength;
			}
			if (nullptr == m_afterScale)
			{
				m_afterScale = new uint8_t[ceil(dstW * dstH * 3 / 2)];
			}
			scaleI420(m_Front->GetY(), sourceW, sourceH, m_afterScale, dstW, dstH, 0);

			initializeArrays(dstW, dstH, srcLength);
			cutByfondCount(dstW, dstH);
			testOneBlock();
			jointVideo();
		}
		
		int desW = 0;
		int desH = 0;
		if (m_fold.orientation == HORIZONTAL)
		{
			//desW = connectList[0].dstW;
			desW = widgetWidth;
			desH = widgetHeight;
		}
		else
		{
			desW = widgetWidth;
			desH = widgetHeight;
			//desH = connectList[0].dstH;
		}
		if (m_fold.style == OpenGL)
		{
			desW = sourceW;
			desH = sourceH;
		}
		glViewport(0, -(widgetHeight - this->height()), widgetWidth, widgetHeight);
		/*Y*/
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, tex_y);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, desW, desH, 0, GL_RED, GL_UNSIGNED_BYTE, (GLvoid*)dstTotal);
		glUniform1i(sampler_y, 0);

		/*U*/
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, tex_u);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, desW / 2, desH / 2, 0, GL_RED, GL_UNSIGNED_BYTE, (GLvoid*)(dstTotal + desW * desH));
		glUniform1i(sampler_u, 1);

		/*V*/
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, tex_v);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, desW / 2, desH / 2, 0, GL_RED, GL_UNSIGNED_BYTE, (GLvoid*)(dstTotal + desW * desH * 5 / 4));
		glUniform1i(sampler_v, 2);
		if (m_fold.style == OpenGL)
		{
			for (int i = 0; i < m_count; i++)
			{
				glDrawArrays(GL_TRIANGLE_FAN, 4 * i, 4);
			}
		}
		else
		{
			glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
		}
		glFlush();
    }
}
//将裁剪好的视频块进行拼接
void VlcPlayerWidget::jointVideo()
{	
	if (m_fold.orientation == HORIZONTAL)
	{
		connectI420Ver(connectList, dstTotal);
	}
	else
	{
		connectI420Hon(connectList, dstTotal);
	}
}

void VlcPlayerWidget::testOneBlock()
{
	if (m_fold.orientation == HORIZONTAL)
	{
		for (int i = 0; i < m_count; i++)
		{
			if (nullptr == connectList[i].data)
			{
				connectList[i].dstH = dstList[i].dstH;
				connectList[i].dstW = widgetWidth;
				connectList[i].data = new uint8_t[ceil(connectList[i].dstW * connectList[i].dstH * 3 / (float)2)];
			}
			auto &connectblock = connectList[i];
			auto &dst = dstList[i];
			fillBlackColor(connectList[i].dstW, connectList[i].dstH, connectblock.data);
			//memset(connectblock.data, 0x80, (connectList[i].dstW * connectList[i].dstH * 3 / (float)2));

			int offX = m_fold.layoutItemns[i].x;
			if (offX != 0)	//存在左偏移
			{
				mergeVideo2HorizontalBlock(connectblock, offX, dst);

			}
			else if (widgetWidth - m_fold.layoutItemns.at(i).x - m_fold.layoutItemns.at(i).width != 0)	// 存在右偏移
			{
				mergeVideo2HorizontalBlock(connectblock, 0, dst);
			}
			else
			{
				// 左右均无偏移
				memcpy(connectblock.data, dst.data, dst.dstW * dst.dstH * 3 / (float)2);
			}
		}
	}
	else
	{
		for (int i = 0; i < m_count; i++)
		{
			if (nullptr == connectList[i].data)
			{
				connectList[i].dstH = widgetHeight;
				connectList[i].dstW = dstList[i].dstW;
				connectList[i].data = new uint8_t[ceil(connectList[i].dstW * connectList[i].dstH * 3 / (float)2)];
			}
			auto &connectblock = connectList[i];
			auto &dst = dstList[i];
			fillBlackColor(connectList[i].dstW, connectList[i].dstH, connectblock.data);

			int offY = m_fold.layoutItemns[i].y;
			if (offY != 0)	//存在上偏移
			{
				mergeVideo2VerticalBlock(connectblock, offY, dst);

			}
			else if (widgetHeight - offY - m_fold.layoutItemns.at(i).height != 0)	// 存在下偏移
			{
				mergeVideo2VerticalBlock(connectblock, 0, dst);
			}
			else
			{
				// 上下均无偏移
				memcpy(connectblock.data, dst.data, dst.dstW * dst.dstH * 3 / (float)2);
			}
		}
	}
}

void VlcPlayerWidget::mergeVideo2HorizontalBlock(DstData & connectblock, int offX, DstData & dst)
{
	//Y 分量
	int nOff = 0;
	for (int j = 0; j < connectblock.dstH; j++)
	{
		nOff = widgetWidth * j + offX;
		memcpy(connectblock.data + nOff, dst.data + dst.dstW * j, dst.dstW);
	}

	nOff = 0;
	int nIndex = 0;
	int YTotal = widgetWidth * connectblock.dstH;
	for (int k = 0; k < (connectblock.dstH >> 1); k++)
	{
		nOff = (widgetWidth >> 1) * k + (offX >> 1);
		//逐行拷贝
		memcpy(connectblock.data + nOff + YTotal, dst.data + dst.dstW * dst.dstH + nIndex, dst.dstW >> 1);
		// 拷贝 V 分量
		memcpy(connectblock.data + nOff + YTotal * 5 / 4, dst.data + (dst.dstW * dst.dstH * 5 / 4) + nIndex, dst.dstW >> 1);
		nIndex += (dst.dstW >> 1);
	}
}

void VlcPlayerWidget::mergeVideo2VerticalBlock(DstData & connectblock, int offY, DstData & dst)
{
	// 依次拷贝N块数据的Y分量
	int totalY = connectblock.dstW * offY;  // Y分量每次递增
	memcpy(connectblock.data + totalY, dst.data, dst.dstW * dst.dstH);

	// 依次拷贝N块数据的U分量
	totalY = connectblock.dstW * widgetHeight;
	int increaseU = (connectblock.dstW >> 1) * (offY >> 1);
	int nIndex = 0;
	for (int i = 0; i < dst.dstH >> 1; i++)
	{
		memcpy(connectblock.data + totalY + increaseU + (connectblock.dstW >> 1) * i, dst.data + dst.dstW * dst.dstH + nIndex, dst.dstW >> 1);
		nIndex += (dst.dstW >> 1);
	}

	// 依次拷贝N块数据的V分量
	increaseU = (connectblock.dstW >> 1) * (widgetHeight >> 1);
	int increaseV = (connectblock.dstW >> 1) * (offY >> 1);
	nIndex = 0;
	uint8_t *pVSour = dst.data + (dst.dstW * dst.dstH * 5 / 4);
	uint8_t *pVDest = connectblock.data + totalY + increaseU + increaseV;
	for (int i = 0; i <(dst.dstH >> 1); i++)
	{
		memcpy(pVDest + (connectblock.dstW >> 1) * i, pVSour + nIndex, dst.dstW >> 1);
		nIndex += (dst.dstW >> 1);
	}
}

void VlcPlayerWidget::initializeArrays(int w, int h,int srcLength)
{
	if (0 == dstList.size())
	{
		if (m_fold.orientation == HORIZONTAL)
		{
			for (int i = 0; i <m_count; i++)
			{
				uint8_t *dst = new uint8_t[ceil((w * h + w * h / 2) *  (double)(m_fold.layoutItemns.at(i).width) / srcLength)];
				DstData da;
				da.data = dst;
				da.dstH = h;
				da.dstW = m_fold.layoutItemns[i].width;
				dstList.push_back(da);
			}
		}
		else
		{
			for (int i = 0; i < m_count; i++)
			{
				uint8_t *dst = new uint8_t[ceil((w * h + w * h / 2) *  (double)(m_fold.layoutItemns.at(i).height) / srcLength)];
				DstData da;
				da.data = dst;
				da.dstH = m_fold.layoutItemns[i].height;
				da.dstW = w;
				dstList.push_back(da);
			}
		}
		dstTotal = new uint8_t[ceil(widgetWidth * widgetHeight * 3 / 2)];
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
					m_afterScale,
					blockX,                                                             // 裁剪目标块X偏移()
					0,                                                                  // 裁剪目标块y偏移（后续支持XY偏移量之后更改此参数即可）
					w, h,                                                               // 视频源宽高
					dstList.at(index - 1).data,
					dstList.at(index - 1).dstW,
					dstList.at(index - 1).dstH);           //裁剪目标宽高
				blockX += dstList.at(index - 1).dstW;
			}
			else
			{
				Cut_I420(
					m_afterScale,
					0,                                                             // 裁剪目标块X偏移()
					blockY,                                                                  // 裁剪目标块y偏移（后续支持XY偏移量之后更改此参数即可）
					w, h,                                                               // 视频源宽高
					dstList.at(index - 1).data,
					dstList.at(index - 1).dstW,
					dstList.at(index - 1).dstH);           //裁剪目标宽高
				blockY += dstList.at(index - 1).dstH;
			}
		index++;

	}
}

void VlcPlayerWidget::resizeGL(int w, int h)
{
    //float viewWidth = 2.0f;
    //float viewHeight = 2.0f;

    //mWorld.setToIdentity();

    //mView.setToIdentity();
    //mView.lookAt(QVector3D(0.0f, 0.0f, 1.0f), QVector3D(0.f, 0.f, 0.f), QVector3D(0.f, 1.f, 0.f));

    //mProj.setToIdentity();
    //if (m_Front)
    //{
    //    float aspectRatio = float(m_Front->GetWidth()) / m_Front->GetHeight();
    //    //aspectRatio = float(4) / 3; // 强制长宽比
    //    if (float(float(w) / h > aspectRatio))
    //    {
    //        viewHeight = 2.0f;
    //        viewWidth = w * viewHeight / (aspectRatio * h);
    //    }
    //    else
    //    {
    //        viewWidth = 2.0f;
    //        viewHeight = h * viewWidth / (1 / aspectRatio * w);
    //    }
    //}
    ////留黑边 强制长宽比
    //mProj.ortho(-viewWidth / 2, viewWidth / 2, -viewHeight / 2, viewHeight / 2, -1.f, 1.0f);

}

void VlcPlayerWidget::InitShaders()
{
	static  GLfloat vertexVertices[100];
	static  GLfloat textureVertices[100];
	if (m_fold.style == OpenGL)
	{

		std::vector<Point> vertexPoints;
		std::vector<Point> texurePoints;
		initPoint(vertexPoints, texurePoints);
		
		for (int i = 0,j=0;i<(m_count * 8),j< (m_count * 4);i++,j++)
		{
			vertexVertices[i] = vertexPoints[j].x;
			vertexVertices[i+1] = vertexPoints[j].y;
			textureVertices[i] = texurePoints[j].x;
			textureVertices[i + 1] = texurePoints[j].y;
			i++;
		}
		//static const GLfloat vertexVertices[] = {
		//	// 顶点逆时针旋转
		//	-1.0f,  0.0f,		// 左上角区域
		//	 0.0f,  0.0f,
		//	 0.0f,  1.0f,
		//	-1.0f,  1.0f,

		//	0.2f, 0.2f,			// 右上角区域
		//	1.0f, 0.2f,
		//	1.0f, 1.0f,
		//	0.2f, 1.0f,

		//	-1.0f,  -1.0f,		// 左下角区域
		//	 0.0f,  -1.0f,
		//	 0.0f,  -0.2f,
		//	-1.0f,  -0.2f,

		//	0.2f, -1.0f,		// 右下角区域
		//	1.0f, -1.0f,
		//	1.0f, 0.0f,
		//	0.2f, 0.0f,
		//};

		//static  const GLfloat textureVertices[] = {
		//	// （Y按照和顶点一样的方式逆时针4个点，从左下角开始），然后1- Y (翻转处理，不然出来的图像是反的)
		//	0.0f, 1 - 0.5f,		// 左上角区域
		//	0.5f, 1 - 0.5f,
		//	0.5f, 1 - 1.0f,
		//	0.0f, 1 - 1.0f,

		//	0.5f,  1 - 0.5f,		// 右上角区域
		//	1.0f,  1 - 0.5f,
		//	1.0f,  1 - 1.0f,
		//	0.5f,  1 - 1.0f,

		//	0.0f, 1 - (0.5f - 0.5f),		// 左下角区域
		//	0.5f, 1 - (0.5f - 0.5f),
		//	0.5f, 1 - (1.0f - 0.5f),
		//	0.0f, 1 - (1.0f - 0.5f),

		//	0.5f,  1 - (0.5f - 0.5f),	// 右下角区域
		//	1.0f,  1 - (0.5f - 0.5f),
		//	1.0f,  1 - (1.0f - 0.5f),
		//	0.5f,  1 - (1.0f - 0.5f),
		//};
	}
	else
	{
		 //注意顶点坐标和下面的纹理坐标不是一一对应的，而是镜面对应的
		vertexVertices[0] = -1.0f;
		vertexVertices[1] = -1.0f;
		vertexVertices[2] = 1.0f;
		vertexVertices[3] = -1.0f;
		vertexVertices[4] = 1.0f;
		vertexVertices[5] = 1.0f;
		vertexVertices[6] = -1.0f;
		vertexVertices[7] = 1.0f;
		//vertexVertices = {	
		//	-1.0f, -1.0f,
		//	1.0f,  -1.0f,
		//	1.0f,  1.0f,
		//	-1.0f, 1.0f
		//};
		textureVertices[0] = 0.0f;
		textureVertices[1] = 1.0f;
		textureVertices[2] = 1.0f;
		textureVertices[3] = 1.0f;
		textureVertices[4] = 1.0f;
		textureVertices[5] = 0.0f;
		textureVertices[6] = 0.0f;
		textureVertices[7] = 0.0f;
		//textureVertices = {
		//	0.0f,  1.0f,
		//	1.0f,  1.0f,
		//	1.0f,  0.0f,
		//	0.0f,  0.0f
		//};
	}

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


	glVertexAttribPointer(0, 2, GL_FLOAT, 0, 0, vertexVertices);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 2, GL_FLOAT, 0, 0, textureVertices);
	glEnableVertexAttribArray(1);


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
 
}

void VlcPlayerWidget::initPoint(std::vector<Point> &vertexPoints, std::vector<Point> &texurePoints)
{
	// 顶点和纹理坐标是镜像同方向转动的，因此每个矩形也是镜像对应的
	//反向遍历，不然图像就颠倒了（*镜像）

	auto fonds = m_fold.layoutItemns;
	if (HORIZONTAL == m_fold.orientation)
	{
		//顶点，横向
		for (int i = m_count - 1; i >= 0; i--)
		{
			// 左下->右下->右上->左上
			Point pointLeftBottom;
			pointLeftBottom.x = (fonds[i].x / ((float)widgetWidth / 2)) - 1;
			pointLeftBottom.y = 1 - ((fonds[i].y + fonds[i].height) / ((float)widgetHeight / 2));
			Point pointRightBottom;
			pointRightBottom.x = (((fonds[i].x + fonds[i].width) - (widgetWidth / 2)) / (float)(widgetWidth / 2));
			pointRightBottom.y = 1 - ((fonds[i].y + fonds[i].height) / ((float)widgetHeight / 2));
			Point pointRightTop;
			pointRightTop.x = (((fonds[i].x + fonds[i].width) - (widgetWidth / 2)) / (float)(widgetWidth / 2));
			pointRightTop.y = (((float)widgetHeight / 2) - fonds[i].y) / (float)(widgetHeight / 2);
			Point pointLeftTop;
			pointLeftTop.x = (fonds[i].x / ((float)widgetWidth / 2)) - 1;
			pointLeftTop.y = (((float)widgetHeight / 2) - fonds[i].y) / (float)(widgetHeight / 2);
			vertexPoints.push_back(pointLeftBottom);
			vertexPoints.push_back(pointRightBottom);
			vertexPoints.push_back(pointRightTop);
			vertexPoints.push_back(pointLeftTop);
		}
		//纹理，竖向切割原视频
		int previousWidth = 0;
		for (int i = m_count - 1; i >= 0; i--)
		{
			// 左上->右上->右下->左下
			Point pointLeftTop;
			pointLeftTop.x = previousWidth / (float)oriScreenWidth;
			pointLeftTop.y = 1;
			Point pointRightTop;
			pointRightTop.x = (previousWidth + fonds[i].width) / (float)oriScreenWidth;
			pointRightTop.y = 1;
			Point pointRightBottom;
			pointRightBottom.x = (previousWidth + fonds[i].width) / (float)oriScreenWidth;
			pointRightBottom.y = 0;
			Point pointLeftBottom;
			pointLeftBottom.x = previousWidth / (float)oriScreenWidth;
			pointLeftBottom.y = 0;

			previousWidth += fonds[i].width;
			texurePoints.push_back(pointLeftTop);
			texurePoints.push_back(pointRightTop);
			texurePoints.push_back(pointRightBottom);
			texurePoints.push_back(pointLeftBottom);
		}
	}
	else
	{
		//顶点，竖向显示
		for (int i = m_count - 1; i >= 0; i--)
		{
			// 左下->右下->右上->左上
			Point pointLeftBottom;
			pointLeftBottom.x = (fonds[i].x / ((float)widgetWidth / 2)) - 1;
			pointLeftBottom.y = (((widgetHeight / 2) - (fonds[i].y + fonds[i].height)) / (float)(widgetHeight / 2));
			Point pointRightBottom;
			pointRightBottom.x = ((fonds[i].x + fonds[i].width) / ((float)widgetWidth / 2)) - 1;
			pointRightBottom.y = (((widgetHeight / 2) - (fonds[i].y + fonds[i].height)) / (float)(widgetHeight / 2));
			Point pointRightTop;
			pointRightTop.x = ((fonds[i].x + fonds[i].width) / ((float)widgetWidth / 2)) - 1;
			pointRightTop.y = 1 - (fonds[i].y / ((float)widgetHeight / 2));
			Point pointLeftTop;
			pointLeftTop.x = (fonds[i].x / ((float)widgetWidth / 2)) - 1;
			pointLeftTop.y = 1 - (fonds[i].y / ((float)widgetHeight / 2));
			vertexPoints.push_back(pointLeftBottom);
			vertexPoints.push_back(pointRightBottom);
			vertexPoints.push_back(pointRightTop);
			vertexPoints.push_back(pointLeftTop);
		}
		//纹理，横向切割原视频
		int previousHeight = 0;
		for (int i = m_count - 1; i >= 0; i--)
		{
			// 左上->右上->右下->左下
			Point pointLeftTop;
			pointLeftTop.x = 0;
			pointLeftTop.y = 1 - (previousHeight / (float)(oriScreenHeight));
			Point pointRightTop;
			pointRightTop.x = 1;
			pointRightTop.y = 1 - (previousHeight / (float)(oriScreenHeight));
			Point pointRightBottom;
			pointRightBottom.x = 1;
			pointRightBottom.y = 1 - ((previousHeight + fonds[i].height) / (float)(oriScreenHeight));
			Point pointLeftBottom;
			pointLeftBottom.x = 0;
			pointLeftBottom.y = 1 - ((previousHeight + fonds[i].height) / (float)(oriScreenHeight));
			previousHeight += fonds[i].height;
			texurePoints.push_back(pointLeftTop);
			texurePoints.push_back(pointRightTop);
			texurePoints.push_back(pointRightBottom);
			texurePoints.push_back(pointLeftBottom);
		}
	}
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
    for (int i = 0; i < (desHeight >> 1); i++)//  
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

void VlcPlayerWidget::scaleI420(uint8_t *src_i420_data, int width, int height, uint8_t *dst_i420_data, int dst_width, int dst_height, int mode)
{

	int src_i420_y_size = width * height;
	int src_i420_u_size = (width >> 1) * (height >> 1);
	uint8_t *src_i420_y_data = src_i420_data;
	uint8_t *src_i420_u_data = src_i420_data + src_i420_y_size;
	uint8_t *src_i420_v_data = src_i420_data + src_i420_y_size + src_i420_u_size;

	int dst_i420_y_size = dst_width * dst_height;
	int dst_i420_u_size = (dst_width >> 1) * (dst_height >> 1);
	uint8_t *dst_i420_y_data = dst_i420_data;
	uint8_t *dst_i420_u_data = dst_i420_data + dst_i420_y_size;
	uint8_t *dst_i420_v_data = dst_i420_data + dst_i420_y_size + dst_i420_u_size;

	libyuv::I420Scale((const uint8_t*)src_i420_y_data, width,
		(const uint8_t *)src_i420_u_data, width >> 1,
		(const uint8_t *)src_i420_v_data, width >> 1,
		width, height,
		(uint8_t *)dst_i420_y_data, dst_width,
		(uint8_t *)dst_i420_u_data, dst_width >> 1,
		(uint8_t *)dst_i420_v_data, dst_width >> 1,
		dst_width, dst_height,
		(libyuv::FilterMode) mode);
}


void VlcPlayerWidget::fillBlackColor(int desW, int desH, uint8_t * append)
{
	for (int i = 0; i < desH; i++)
	{
		memset(append + desW * i, 0x00, desW);
	}
	for (int i = 0; i < desH / 2; i++)
	{
		memset(append + desW * desH + (desW / 2) * i, 0x80, desW / 2);
		memset(append + desW * desH * 5 / 4 + (desW / 2) * i, 0x80, desW / 2);
	}
}