#pragma once
#ifndef CHILDWINDOW_H_
#define CHILDWINDOW_H_

#include <QMainWindow>
#include "ui_ChildWindow.h"

QT_BEGIN_NAMESPACE
class QImage;
class QPainter;
class QRect;
class ImageWidget;
QT_END_NAMESPACE

class ChildWindow :
	public QMainWindow
{
	Q_OBJECT

public:
	ChildWindow(void);
	~ChildWindow(void);

	bool LoadFile(QString filename);
	QString current_file() { return current_file_; }

	public slots:


public:
	ImageWidget			*imagewidget_;
	QString				current_file_;

private:
	Ui::ChildWindow ui;
};

#endif // !CHILDWINDOW_H_
