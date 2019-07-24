//#include "VLCFramePaint.h"
//#ifdef _WIN32
//#include <basetsd.h>
//typedef SSIZE_T ssize_t;
//#endif
//#include "vlc/vlc.h"
//#include <QPixmap>
//#include <QImage>
//#include <QPainter>
//#include <QMutex>
//using namespace std;
//
//// 定义输出视频的分辨率
//#define VIDEO_WIDTH   1920
//#define VIDEO_HEIGHT  1280
//
//
//struct context {
//    QMutex mutex;
//    uchar *pixels;
//};
//
//static void *lock(void *opaque, void **planes)
//{
//    struct context *ctx = (context *)opaque;
//    ctx->mutex.lock();
//    *planes = ctx->pixels;
//    return NULL;
//}
//
//static void unlock(void *opaque, void *picture, void *const *planes)
//{
//    struct context *ctx = (context *)opaque;
//    unsigned char *data = (unsigned char *)*planes; // planes即为帧数据
//    QImage image(data, VIDEO_WIDTH, VIDEO_HEIGHT, QImage::Format_RGBA8888);// 指定生成的图片格式为 RGBA 4通道
//    VLCFramePaint::pThis->updatePicture(image);  // 
//    ctx->mutex.unlock();
//}
//
//static void display(void *opaque, void *picture)
//{
//    (void)opaque;
//}
//VLCFramePaint* VLCFramePaint::pThis = nullptr;
//
//VLCFramePaint::VLCFramePaint(QWidget *parent)
//    : QWidget(parent)
//{
//    pThis = this;
//    //
//    connect(this, SIGNAL(showImage()), this, SLOT(update()));
//
//    libvlc_instance_t * inst;
//    libvlc_media_player_t *mp;
//    libvlc_media_t *m;
//
//    context *ctx = new context;
//    ctx->pixels = new uchar[VIDEO_WIDTH * VIDEO_HEIGHT * 4]; // 申请大小也为4通道的像素
//    memset(ctx->pixels, 0, VIDEO_WIDTH * VIDEO_HEIGHT * 4);
//
//    libvlc_time_t length;
//    int width;
//    int height;
//    inst = libvlc_new(0, NULL);
//    m = libvlc_media_new_path(inst, "D:\\10.mp4");
//    mp = libvlc_media_player_new_from_media(m);
//    //libvlc_media_player_set_hwnd(mp, (void *)this->winId());
//    libvlc_video_set_callbacks(mp, lock, unlock, display, ctx);
//    libvlc_video_set_format(mp, "RGBA", VIDEO_WIDTH, VIDEO_HEIGHT, VIDEO_WIDTH * 4);
//    libvlc_media_release(m);
//    libvlc_media_player_play(mp);
//}
//
//void VLCFramePaint::updatePicture(const QImage &image)
//{
//    lists.push_back(image);
//    emit showImage();
//}
//
//void VLCFramePaint::paintEvent(QPaintEvent *event)
//{
//    if (lists.empty())
//    {
//        return;
//    }
//    QPainter painter(this);
//    painter.drawPixmap(this->rect(), QPixmap::fromImage(lists.front()));
//    lists.pop_front();
//}