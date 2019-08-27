#pragma once
#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QMatrix4x4>
#include "I420Image.h"
#include <assert.h>
#include <thread>
#include <QMutex>
#include <vector>
struct libvlc_media_track_info_t;
struct libvlc_media_t;
struct libvlc_instance_t;
struct libvlc_media_player_t;
struct libvlc_event_t;

class context;
struct Point
{
	float x = 0.0;
	float y = 0.0;
};
struct DstData
{
    int dstW = 0;
    int dstH = 0;
    uint8_t *data = nullptr;
};

struct Layout
{
    int x;
    int y;
    int width;
    int height;
};

enum EnumOrientation
{
    HORIZONTAL, //���տ�Ȳü�
    VERTICAL	//���ո߶Ȳü�
};
enum EnumCropStyle
{
	OpenGL,		//����opengl�Ķ��㷽ʽ���вü�ƴ��
	YUV			//����YUV���ݸ�ʽ�ü�ƴ��
};
struct Fold {
	int count = 0;
    EnumOrientation orientation = HORIZONTAL;
	EnumCropStyle style = YUV;
    std::vector<Layout> layoutItemns;
};


class VlcPlayerWidget : public QOpenGLWidget, public QOpenGLFunctions
{
    Q_OBJECT

public:
    explicit VlcPlayerWidget(QWidget *parent = 0);
    ~VlcPlayerWidget();

	void setVideoFilePath(QString m_filepath);
	void setCropDirection(EnumOrientation);
	void setCropStyle(EnumCropStyle);
public:
    void play();
    void pause();
    void stop();

public:
    static void *lock_cb(void *opaque, void **planes);
    static void unlock_cb(void *opaque, void *picture, void *const *planes);
    static void display_cb(void *opaque, void *picture);
    static unsigned setup_cb(void **opaque, char *chroma,
        unsigned *width, unsigned *height,
        unsigned *pitches,
        unsigned *lines);
    static void cleanup_cb(void *opaque);

protected:

    virtual void initializeGL() override;

	virtual void paintGL() override;
	virtual void resizeGL(int w, int h) override;
private:
	int m_count = 0;
    double Video2WidgetRation_W = 0.0;
    double Video2WidgetRation_H = 0.0;
	int widgetWidth = 0;
	int widgetHeight = 0;
	int oriScreenWidth = 0;
	int oriScreenHeight = 0;
    std::vector<DstData> dstList;	 // �ü��õ���Ƶ����list
	std::vector<DstData> connectList;
	uint8_t *m_afterScale = nullptr;
    uint8_t *dstTotal = nullptr;
    void InitShaders();
	void initPoint(std::vector<Point> &vertexPoints, std::vector<Point> &texurePoints);
	void Cut_I420(uint8_t * Src, int x, int y, int srcWidth, int srcHeight, uint8_t *Dst, int desWidth, int desHeight);
    void connectI420Ver(std::vector<DstData> disList, uint8_t * Dst);
	void connectI420Hon(std::vector<DstData> &disList, uint8_t * Dst);
	void initializeArrays(int w, int h,int srcLength);
    void cutByfondCount(int w, int h);
	void jointVideo();
	void scaleI420(uint8_t * src_i420_data, int width, int height, uint8_t * dst_i420_data, int dst_width, int dst_height, int mode);
	void fillBlackColor(int desW, int desH, uint8_t * append);
	GLuint program;
    GLuint tex_y, tex_u, tex_v;
    GLuint sampler_y, sampler_u, sampler_v;
    Fold m_fold;
    libvlc_instance_t* m_vlc;
    libvlc_media_player_t *m_vlcplayer;
    I420Image *m_Front;
    I420Image *m_Back;
    QString m_input;
	void initFond();
	void testOneBlock();
	/*
	  ������offxʱ�������xƫ�ƣ�
	  ��offx=0ʱ����Ϊ�����ƫ�ƣ�ֱ��ƴ�ӵ����block������߼��ɣ��ұ߿հ��Զ�������
	*/
	void mergeVideo2HorizontalBlock(DstData & connectblock, int offX, DstData & dst);
	void mergeVideo2VerticalBlock(DstData & connectblock, int offY, DstData & dst);
};