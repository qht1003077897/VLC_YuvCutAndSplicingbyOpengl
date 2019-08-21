#pragma once

#include <QWidget>
#include "ui_widget.h"
#include "vlcplayerwidget.h"
namespace Ui {
    class Widget;
}

class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = 0);
    ~Widget();

private slots:
    void on_pushButton_clicked();

    void on_pushButton_2_clicked();

    void on_pushButton_3_clicked();

	void on_pushButton_4_clicked();
public slots:
	void checkSlot1(bool);
	void checkSlot2(bool);
	void checkSlot3(bool);
	void checkSlot4(bool);
private:
	QString m_filepath;
    Ui::Widget *ui;
    VlcPlayerWidget *player;
};

