#include "widget.h"


Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);
    player = new VlcPlayerWidget;
    player->show();
    on_pushButton_clicked();
}

Widget::~Widget()
{
    delete ui;
}

void Widget::on_pushButton_clicked()
{
    player->setInput(ui->lineEdit->text());
    player->play();
}

void Widget::on_pushButton_2_clicked()
{
    player->pause();
}

void Widget::on_pushButton_3_clicked()
{
    player->stop();
}
