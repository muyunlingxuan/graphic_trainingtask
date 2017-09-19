#pragma once
#ifndef WARPINGIDW_H_
#define WARPINGIDW_H_
#include<Warping.h>

using namespace std;

class WarpingIDW : public Warping {
public:
	WarpingIDW();
	~WarpingIDW();
	virtual void doWarp(cv::Mat *image_mat);

private:
	cv::Mat warpImage(cv::Mat *image_mat);

	cv::Point2d f_func(cv::Point2d p);
	cv::Point2d fi_func(cv::Point2d p, int i);
	double wi_func(cv::Point2d p, double d, int i);
};
#endif // !WARPINGIDW_H_
