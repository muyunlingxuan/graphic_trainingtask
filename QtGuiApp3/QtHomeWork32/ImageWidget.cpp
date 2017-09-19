#include "ImageWidget.h"
#include <QImage>
#include <QFileDialog>
#include <QPainter>
#include <QtWidgets> 
#include "WarpingIDW.h"
#include "WarpingRBF.h"
#include "WarpingIDW_D.h"
#include "WarpingRBF2.h"
#include <sys/timeb.h>
#include <opencv2/flann.hpp>

ImageWidget::ImageWidget(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
}

ImageWidget::~ImageWidget()
{
}

void ImageWidget::SaveAs()
{
	QString filename = QFileDialog::getSaveFileName(this, tr("Save Image"), ".", tr("Images(*.bmp *.png *.jpg)"));
	if (filename.isNull())
	{
		return;
	}
	cv::Mat image_save;
	cv::cvtColor(image_mat_, image_save, CV_RGB2BGR);
	cv::imwrite(filename.toLatin1().data(), image_save);
}

void ImageWidget::Open() {
	// Open file
	QString fileName = QFileDialog::getOpenFileName(this, tr("Read Image"), ".", tr("Images(*.bmp *.png *.jpg)"));

	// Load file
	if (!fileName.isEmpty())
	{
		image_mat_ = cv::imread(fileName.toLatin1().data());
		cv::cvtColor(image_mat_, image_mat_, CV_BGR2RGB);
		image_mat_backup_ = image_mat_.clone();
	}

	update();
}

void ImageWidget::paintEvent(QPaintEvent *paintevent) {
	QPainter painter;
	painter.begin(this);

	// Draw background
	painter.setBrush(Qt::lightGray);
	QRect back_rect(0, 0, width(), height());
	painter.drawRect(back_rect);

	// Draw image
	QImage image_show = QImage((unsigned char *)(image_mat_.data), image_mat_.cols, image_mat_.rows, image_mat_.step, QImage::Format_RGB888);
	QRect rect = QRect(0, 0, image_show.width(), image_show.height());
	painter.drawImage(rect, image_show);

	// Draw control line
	QPen pen(Qt::red, 2);
	painter.setPen(pen);
	
	painter.drawLine(point_start_qt_, point_end_qt_);
	if (vec_points_start_.size() != 0)
	{
		for (size_t i = 0; i<vec_points_end_.size(); i++)
		{
			painter.drawLine(vec_points_start_[i], vec_points_end_[i]);
		}
	}
	painter.end();
}

void ImageWidget::mousePressEvent(QMouseEvent *mouseevent) {
	if (is_warp_ && (mouseevent->button() == Qt::LeftButton)) {
		point_start_qt_ = point_end_qt_ = mouseevent->pos();
	}
}

void ImageWidget::mouseMoveEvent(QMouseEvent *mouseevent) {
	if (is_warp_)
	{
		point_end_qt_ = mouseevent->pos();
	}
	update();
}

void ImageWidget::mouseReleaseEvent(QMouseEvent *mouseevent) {
	if (is_warp_)
	{
		point_end_qt_ = mouseevent->pos();
		vec_points_start_.push_back(point_start_qt_);
		vec_points_end_.push_back(point_end_qt_);
	}
}

void ImageWidget::Invert()
{
	cv::MatIterator_<cv::Vec3b> iter, iterend;
	for (iter = image_mat_.begin<cv::Vec3b>(), iterend = image_mat_.end<cv::Vec3b>(); iter != iterend; ++iter)
	{
		(*iter)[0] = 255 - (*iter)[0];
		(*iter)[1] = 255 - (*iter)[1];
		(*iter)[2] = 255 - (*iter)[2];
	}
	update();
}

void ImageWidget::Mirror(bool ishorizontal, bool isvertical)
{
	int width = image_mat_.cols;
	int height = image_mat_.rows;

	if (ishorizontal)
	{
		if (isvertical)
		{
			for (int i = 0; i<width; i++)
			{
				for (int j = 0; j<height; j++)
				{
					image_mat_.at<cv::Vec3b>(j, i) = image_mat_backup_.at<cv::Vec3b>(height - 1 - j, width - 1 - i);
				}
			}
		}
		else
		{
			for (int i = 0; i<width; i++)
			{
				for (int j = 0; j<height; j++)
				{
					image_mat_.at<cv::Vec3b>(j, i) = image_mat_backup_.at<cv::Vec3b>(j, width - 1 - i);
				}
			}
		}

	}
	else
	{
		if (isvertical)
		{
			for (int i = 0; i<width; i++)
			{
				for (int j = 0; j<height; j++)
				{
					image_mat_.at<cv::Vec3b>(j, i) = image_mat_backup_.at<cv::Vec3b>(height - 1 - j, i);
				}
			}
		}
	}

	update();
}

void ImageWidget::TurnGray()
{
	cv::MatIterator_<cv::Vec3b> iter, iterend;
	for (iter = image_mat_.begin<cv::Vec3b>(), iterend = image_mat_.end<cv::Vec3b>(); iter != iterend; ++iter)
	{
		int itmp = ((*iter)[0] + (*iter)[1] + (*iter)[2]) / 3;
		(*iter)[0] = itmp;
		(*iter)[1] = itmp;
		(*iter)[2] = itmp;
		
	}
	update();
}

void ImageWidget::EraseLines() {
	is_warp_ = false;
	vec_points_start_.clear();
	vec_points_end_.clear();

	
	point_start_qt_ = QPoint(0, 0);
	point_end_qt_ = QPoint(0, 0);
	update();
}

void ImageWidget::Restore() {
	image_mat_ = image_mat_backup_.clone();
	update();
}

void ImageWidget::SetControlPoint() {
	is_warp_ = true;
}

void ImageWidget::doWarp(WarpType type) {

	Warping *warping;
	if (type == WarpType::IDW) {
		warping = new WarpingIDW();
	}
	else if (type == WarpType::RBF) {
		warping = new WarpingRBF();
	}
	else if (type == WarpType::IDW_D) {
		warping = new WarpingIDW_D();
	}
	else if (type == WarpType::RBF_T) {
		warping = new WarpingRBF2();
	}
	warping->setControlPoint(vec_points_start_, vec_points_end_);
	warping->doWarp(&image_mat_);
	warping->holeFilling(&image_mat_);
	repaint();
}

void ImageWidget::IDW_Warp() {
	is_warp_ = false;
	doWarp(WarpType::IDW);


}

void ImageWidget::RBF_Warp() {
	is_warp_ = false;
	doWarp(WarpType::RBF);
}

void ImageWidget::IDW_D_Warp() {
	is_warp_ = false;
	doWarp(WarpType::IDW_D);
}

void ImageWidget::RBF_T_Warp() {
	is_warp_ = false;
	doWarp(WarpType::RBF_T);
}