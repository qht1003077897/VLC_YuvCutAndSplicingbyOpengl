#include "widget.h"
#include "QWidget"
#include "QFileDialog"

Widget::Widget(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::Widget)
{
	ui->setupUi(this);
	player = new VlcPlayerWidget;
	m_filepath = "D:\\ky.mp4";
	ui->lineEdit->setText(m_filepath);
	ui->radioButton1->setChecked(true);
	ui->radioButton4->setChecked(true);
	player->setCropDirection(HORIZONTAL);
	player->setCropStyle(YUV);
	connect(ui->radioButton1, SIGNAL(toggled(bool)), this, SLOT(checkSlot1(bool)));
	connect(ui->radioButton2, SIGNAL(toggled(bool)), this, SLOT(checkSlot2(bool)));
	connect(ui->radioButton3, SIGNAL(toggled(bool)), this, SLOT(checkSlot3(bool)));
	connect(ui->radioButton4, SIGNAL(toggled(bool)), this, SLOT(checkSlot4(bool)));
}

Widget::~Widget()
{
	delete ui;
}

// 播放
void Widget::on_pushButton_clicked()
{
	player->setVideoFilePath(m_filepath);
	player->play();
	player->show();
}
// 暂停
void Widget::on_pushButton_2_clicked()
{
	player->pause();
}
//停止
void Widget::on_pushButton_3_clicked()
{
	player->stop();
}
//文件选择
void Widget::on_pushButton_4_clicked()
{
	QString filepath = QFileDialog::getOpenFileName(NULL, "请选择视频", ".", "Video(*.mp4 *.wmv *.mov *.avi *.h264 *.flv)");
	m_filepath = filepath;
	ui->lineEdit->setText(m_filepath);
	player->setVideoFilePath(m_filepath);
}
// 水平方向
void Widget::checkSlot1(bool)
{
	player->setCropDirection(HORIZONTAL);
}
//垂直方向
void Widget::checkSlot2(bool)
{
	player->setCropDirection(VERTICAL);
}

void Widget::checkSlot3(bool)
{
	player->setCropStyle(OpenGL);
}

void Widget::checkSlot4(bool)
{
	player->setCropStyle(YUV);
}
