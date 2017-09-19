#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtWidgets/QMainWindow>
#include "ui_MainWindow.h"

QT_BEGIN_NAMESPACE
class QAction;
class QMenu;
class QImage;
class QPainter;
class QRect;
class ImageWidget;
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	MainWindow(QWidget *parent = Q_NULLPTR);



private:
	void CreateActions();
	void CreateMenus();
	void CreateToolBars();
	void CreateStatusBar();

private:
	Ui::MainWindowClass ui;
	QMenu		*menu_file_;

	QToolBar	*toolbar_file_;
	QToolBar	*toolbar_edit_;

	QAction		*action_open_;
	QAction     *action_saveAs_;
	QAction		*action_invert_;
	QAction		*action_mirror_;
	QAction		*action_gray_;
	QAction		*action_restore_;
	QAction		*action_warp_;
	QAction		*action_IDW_warp_;
	QAction		*action_IDW2_warp_;
	QAction		*action_RBF_warp_;
	QAction		*action_RBF2_warp_;
	QAction		*action_eraseLiner_;

	ImageWidget		*imagewidget_;

};

#endif
