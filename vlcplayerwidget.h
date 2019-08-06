#pragma once
#include <QOpenGLWidget>
#include <QOpenGLFunctions_2_0>
#include <QMatrix4x4>
#include "I420Image.h"
#include <assert.h>
#include <thread>
#include <QMutex>
struct libvlc_media_track_info_t;
struct libvlc_media_t;
struct libvlc_instance_t;
struct libvlc_media_player_t;
struct libvlc_event_t;

class context;
struct DstData
{
    int dstW;
    int dstH;
    uint8_t *data;
};

//����������Ϣ
struct Layout
{
    int x;
    int y;
    int width;
    int height;
};

enum EnumOrientation
{
    HORIZONTAL,//ˮƽ����
    VERTICAL/*��ֱ����*/
};

struct Fold {
	int count = 0;
    bool enable = false;
	int screenWidth = 0;
	int screenHeight = 0;
	int firstMarginLeft = 0;
    EnumOrientation orientation = HORIZONTAL;
    std::vector<Layout> layoutItemns;
};


class VlcPlayerWidget : public QOpenGLWidget, public QOpenGLFunctions_2_0 //, public Player
{
    Q_OBJECT

public:
    explicit VlcPlayerWidget(QWidget *parent = 0);
    ~VlcPlayerWidget();

public:
    void setInput(QString input);
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
	int m_firstLeftMargin = 0;
	int m_lastRightMargin = 0;
    double Video2WidgetRation_W = 0.0;
    double Video2WidgetRation_H = 0.0;
	int widgetWidth = 0;
	int widgetHeight = 0;
    std::vector<uint8_t*> dstList;	 // �ü��õ���Ƶ����list
	std::vector<uint8_t*> appendList;// ����ÿһ֡��������append��С����һ���ģ�����Ż�Ч�ʣ�ֻ����һ��append,�ݲ��������Ҷ������׵����
	std::vector<uint8_t*> tempList;  // ƴ�Ӻõ���Ƶ����list�������ռ䣬��ȡʱ��Ч�ʣ�����ÿ��mergeʱ������joitʱʹ�ã�
    uint8_t *des = nullptr;
    uint8_t *dstTotal = nullptr;
    void InitShaders();
    void Cut_I420(uint8_t * Src, int x, int y, int srcWidth, int srcHeight, uint8_t *Dst, int desWidth, int desHeight);
    void connectI420Ver(std::vector<DstData> disList, uint8_t * Dst);
	void connectI420Hon(std::vector<DstData> &disList, uint8_t * Dst);
	void initializeArrays(int w, int h,int srcLength);
    void cutByfondCount(int w, int h);
	void mergeBlock();
	void drawFrame();
	void jointVideo();
	GLuint program;
    GLuint tex_y, tex_u, tex_v;
    GLuint sampler_y, sampler_u, sampler_v;
    GLuint matWorld, matView, matProj;
    QMatrix4x4 mProj;
    QMatrix4x4 mView;
    QMatrix4x4 mWorld;
    Fold m_fold;
    libvlc_instance_t* m_vlc;
    libvlc_media_player_t *m_vlcplayer;
    I420Image *m_Front;
    I420Image *m_Back;
    QString m_input;
	void initFond();
};