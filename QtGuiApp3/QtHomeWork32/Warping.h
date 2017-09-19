#pragma once
#ifndef WARPING_H_ 
#define WARPING_H_
#include <QImage>
#include <opencv2/core/core.hpp> 
#include <opencv2/flann.hpp>

using namespace std;
class Warping
{
public:
	Warping();
	~Warping();
	
	virtual void doWarp(cv::Mat *imageIn) {};
	void setControlPoint(vector<QPoint> &ps, vector<QPoint> &qs);
	void resetControlPoint();
	void setFixedPoints(cv::Mat *image);
	void holeFilling(cv::Mat *image);

protected:
	double getD_Reverse(cv::Point2d p);
	double getD_Reverse(cv::Point2d p, cv::Point2d q);
	double getDistance(cv::Point2d p, cv::Point2d q);
	bool isExist(cv::Mat vec_points_, cv::Point2d p);
	

protected :
	cv::Mat vec_points_start_; //控制点起始位置 列向量
	cv::Mat vec_points_end_; // 控制点变换后位置 列向量
};

#endif // !WARPING_H_ 

