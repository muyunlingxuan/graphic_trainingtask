#pragma once
#ifndef WARPINGIDW_D_H_
#define WARPINGIDW_D_H_
#include<Warping.h>

using namespace std;

class WarpingIDW_D : public Warping {
public:
	WarpingIDW_D();
	~WarpingIDW_D();

	virtual void doWarp(cv::Mat *image_mat);

private:
	vector<cv::Mat> calculateMatrixD();
	cv::Mat warpImage(cv::Mat *image_mat);

	cv::Mat calculateMatrixDi(int i);
	cv::Mat calculateMatrixWi(int i);

	cv::Point2d f_func(cv::Point2d p);
	cv::Point2d fi_func(cv::Point2d p, int i);
	double wi_func(cv::Point2d p, double d, int i);

private:
	vector<cv::Mat> vec_D_mat_;
};
#endif // !WARPINGIDW_D_H_
