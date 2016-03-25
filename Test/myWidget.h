#ifndef MYWIDGET_H
#define MYWIDGET_H

#include <QtWidgets/QWidget>

class myWidget : public QWidget
{
	Q_OBJECT

public:
	myWidget(QWidget *parent = 0);
	~myWidget();

private:
};

#endif // MYWIDGET_H
