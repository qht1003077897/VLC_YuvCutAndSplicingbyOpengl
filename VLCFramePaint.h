//
//// 头文件 
//#pragma once
//#include <QtWidgets/QWidget>
//#include <QPaintEvent>
//
//class VLCFramePaint : public QWidget
//{
//    Q_OBJECT
//
//public:
//    VLCFramePaint(QWidget *parent = Q_NULLPTR);
//    void updatePicture(const QImage &image);
//    static VLCFramePaint *pThis;  //声明 pThis 对象方便我们在静态函数中调用成员函数
//protected:
//    virtual void paintEvent(QPaintEvent *event);
//signals:
//    void showImage();
//
//private:
//    std::list<QImage> lists;
//};