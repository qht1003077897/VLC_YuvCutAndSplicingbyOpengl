//
//// ͷ�ļ� 
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
//    static VLCFramePaint *pThis;  //���� pThis ���󷽱������ھ�̬�����е��ó�Ա����
//protected:
//    virtual void paintEvent(QPaintEvent *event);
//signals:
//    void showImage();
//
//private:
//    std::list<QImage> lists;
//};