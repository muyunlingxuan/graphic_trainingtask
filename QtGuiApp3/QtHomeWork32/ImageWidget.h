#pragma once
#ifndef IMAGEWIDGET_H_
#define IMAGEWIDGET_H_
#include <iostream>
#include <QWidget>
#include "ui_ImageWidget.h"
#include "Warping.h"
#include <opencv2/core/core.hpp> 
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp> 
using namespace std;
QT_BEGIN_NAMESPACE
class QImage;
class QPainter;
QT_END_NAMESPACE

class ImageWidget : public QWidget
{
	Q_OBJECT

public:
	ImageWidget(QWidget *parent = Q_NULLPTR);
	~ImageWidget();

public:
	enum WarpType { IDW, RBF ,IDW_D, RBF_T};

protected:
	void paintEvent(QPaintEvent *paintevent);
	void mousePressEvent(QMouseEvent *mouseevent);
	void mouseMoveEvent(QMouseEvent *mouseevent);
	void mouseReleaseEvent(QMouseEvent *mouseevent);


public slots:
	// File IO
	void Open();												// Open an image file, support ".bmp, .png, .jpg" format
	void SaveAs();

	void Invert();
	void Mirror(bool horizontal = false, bool vertical = true);
	void TurnGray();


	void Restore();												// Restore image to origin
	void SetControlPoint();
	void doWarp(WarpType type);
	void IDW_Warp();
	void IDW_D_Warp();
	void RBF_Warp();
	void RBF_T_Warp();
	void EraseLines();

private:
	Ui::ImageWidget ui;

	QPoint point_start_qt_;
	QPoint point_end_qt_;

	vector<QPoint> vec_points_start_;
	vector<QPoint> vec_points_end_;

	/*vector<cv::Point2f> vec_points_start_;
	vector<cv::Point2f> vec_points_end_;*/
	
	cv::Mat	image_mat_;
	cv::Mat	image_mat_backup_;

	bool is_warp_ = false;
	
};
#endif // !IMAGEWIDGET_H_


