#pragma once
#ifndef WarpingRBF2_H_
#define WarpingRBF2_H_
#include "Warping.h"

class WarpingRBF2 : public Warping {
public:
	WarpingRBF2();
	~WarpingRBF2();

	virtual void doWarp(cv::Mat *imageIn);

private:
	cv::Point2d f_func(cv::Point2d p);

	void calculateT();
	void calculateAi();
	void warpImage(cv::Mat *image);
	void initMatrixRd();
	cv::Mat calculateDistance_cpoint();
	void calculateMatrixAi(const cv::Mat &Rd, const cv::Mat &d_cpoint);

private:
	cv::Mat Ai_mat_;
	cv::Mat Rd_mat_;
	cv::Mat r_mat_;
	cv::Mat T_mat_;
};

#endif // !WarpingRBF2_H_
