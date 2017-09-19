#include "WarpingRBF2.h"
#include <iostream>
WarpingRBF2::WarpingRBF2() {

}

WarpingRBF2::~WarpingRBF2() {

}

/**
*
*计算f(p) = ∑ai*R(d)+Tq+a
*1、根据控制点 求ai  R(d)Ai = f(p)-q
*2、求所有点变换后的坐标 f(p) = ∑ai*R(d)+Tq+a
**/
void WarpingRBF2::doWarp(cv::Mat *imageIn) {
	//setFixedPoints(imageIn);
	calculateT();
	calculateAi();
	warpImage(imageIn);
}

/*
f(p) = T(p)+b
求矩阵T
*/
void WarpingRBF2::calculateT() {
	T_mat_ = cv::Mat::eye(cv::Size(3, 3), CV_32FC1);
	int num = vec_points_start_.rows;
	cv::Mat p_mat_ = cv::Mat::zeros(cv::Size(3, num), CV_32FC1);
	cv::Mat q_mat_ = cv::Mat::zeros(cv::Size(2, num), CV_32FC1);
	for (int i = 0; i < num; i++) {
		p_mat_.at<float>(i, 0) = vec_points_start_.at<cv::Point2d>(i, 0).x;
		p_mat_.at<float>(i, 1) = vec_points_start_.at<cv::Point2d>(i, 0).y;
		p_mat_.at<float>(i, 2) = 1;

		q_mat_.at<float>(i, 0) = vec_points_end_.at<cv::Point2d>(i, 0).x;
		q_mat_.at<float>(i, 1) = vec_points_end_.at<cv::Point2d>(i, 0).y;
		
	}
	cv::Mat p_mat_t_ = p_mat_.t();
	cv::Mat p_mat_i_ = (p_mat_t_*p_mat_).inv();
	cv::Mat T_mat_t_ = p_mat_i_* p_mat_t_ *q_mat_;
	T_mat_t_ = T_mat_t_.t();
	T_mat_t_.row(0).copyTo(T_mat_.row(0));
	T_mat_t_.row(1).copyTo(T_mat_.row(1));
}

/*
求ai ∑ai*R(d) = p = f(p)-q
*/
void WarpingRBF2::calculateAi() {
	initMatrixRd();//矩阵R(d)
	cv::Mat d_cpoint = calculateDistance_cpoint();//f(p)-q
	calculateMatrixAi(Rd_mat_, d_cpoint);//∑ai*R(d) = p = f(p)-q
}

void WarpingRBF2::warpImage(cv::Mat *image_mat) {

	cv::Mat image_backup = (*image_mat).clone();
	cv::MatIterator_<cv::Vec3b> iter, iterend;

	for (iter = (*image_mat).begin<cv::Vec3b>(), iterend = (*image_mat).end<cv::Vec3b>(); iter != iterend; ++iter)
	{
		(*iter)[0] = 245;
		(*iter)[1] = 245;
		(*iter)[2] = 245;
	}

	for (int i = 0; i < image_backup.rows; i++)
	{
		for (int j = 0; j < image_backup.cols; j++)
		{
			cv::Point2d p = cv::Point2d(j, i);
			cv::Point2d q = f_func(p);
			if (q.x >= 0 && q.x< image_backup.cols && q.y >= 0 && q.y < image_backup.rows) {
				(*image_mat).at<cv::Vec3b>(q.y, q.x) = image_backup.at<cv::Vec3b>(p.y, p.x);
			}
		}
	}
}

/*
求所有点变换后的坐标 f(p) = ∑ai*R(d)+Ap+a
*/
cv::Point2d WarpingRBF2::f_func(cv::Point2d p) {
	int num = vec_points_start_.rows;

	cv::Mat rd_mat_ = cv::Mat::zeros(cv::Size(1, num), CV_32FC1);
	cv::Mat p_mat_ = cv::Mat::zeros(cv::Size(1, 3), CV_32FC1);
	cv::Mat q_mat_ = cv::Mat::zeros(cv::Size(1, 3), CV_32FC1);


	for (int i = 0; i < num; i++) {
		double d = getDistance(p, vec_points_start_.at<cv::Point2d>(i, 0));
		rd_mat_.at<float>(i, 0) = pow(d + r_mat_.at<float>(i, 0), -1);
	}
	p_mat_.at<float>(0, 0) = p.x;
	p_mat_.at<float>(1, 0) = p.y;
	p_mat_.at<float>(2, 0) = 1;

	q_mat_ = Ai_mat_*rd_mat_ + T_mat_*p_mat_;
	cv::Point2d q = cv::Point2d(q_mat_.at<float>(0, 0), q_mat_.at<float>(1, 0));
	return q;
}


void WarpingRBF2::initMatrixRd() {
	int num = vec_points_start_.rows;
	Rd_mat_ = cv::Mat::zeros(cv::Size(num, num), CV_32FC1);
	r_mat_ = cv::Mat::zeros(cv::Size(1, num), CV_32FC1);
	for (int i = 0; i < num; i++) {
		double r = 999999;
		for (int j = 0; j < num; j++) {
			double d = getDistance(vec_points_start_.at<cv::Point2d>(i, 0), vec_points_start_.at<cv::Point2d>(j, 0));
			Rd_mat_.at<float>(i, j) = d;
			r = (d != 0 && r > d) ? d : r;
		}
		r_mat_.at<float>(i, 0) = r;
	}
	for (int i = 0; i < num; i++) {
		for (int j = 0; j < num; j++) {
			Rd_mat_.at<float>(i, j) = pow(Rd_mat_.at<float>(i, j) + r_mat_.at<float>(j, 0), -1);
		}
	}
}

cv::Mat WarpingRBF2::calculateDistance_cpoint() {
	int num = vec_points_start_.rows;
	cv::Mat p_mat_ = cv::Mat::zeros(cv::Size(1, 3), CV_32FC1);
	cv::Mat q_mat_ = cv::Mat::zeros(cv::Size(1, 3), CV_32FC1);
	cv::Mat fp_mat_ = cv::Mat::zeros(cv::Size(1, 3), CV_32FC1);
	cv::Mat d_cpoint_mat_ = cv::Mat::zeros(cv::Size(3, num), CV_32FC1);
	cv::Mat d_cpoint_mat2_ = cv::Mat::zeros(cv::Size(2, num), CV_32FC1);
	for (int i = 0; i < vec_points_start_.rows; i++) {
		p_mat_.at<float>(0, 0) = vec_points_start_.at<cv::Point2d>(i, 0).x;
		p_mat_.at<float>(1, 0) = vec_points_start_.at<cv::Point2d>(i, 0).y;
		p_mat_.at<float>(2, 0) = 1;

		q_mat_.at<float>(0, 0) = vec_points_end_.at<cv::Point2d>(i, 0).x;
		q_mat_.at<float>(1, 0) = vec_points_end_.at<cv::Point2d>(i, 0).y;
		q_mat_.at<float>(2, 0) = 1;

		fp_mat_ = T_mat_*p_mat_;
		((cv::Mat)((q_mat_ - fp_mat_).t())).copyTo(d_cpoint_mat_.row(i));
	}

	d_cpoint_mat_.col(0).copyTo(d_cpoint_mat2_.col(0));
	d_cpoint_mat_.col(1).copyTo(d_cpoint_mat2_.col(1));

	return d_cpoint_mat_;
}


void WarpingRBF2::calculateMatrixAi(const cv::Mat &Rd, const cv::Mat &d_cpoint) {
	cv::solve(Rd, d_cpoint, Ai_mat_, CV_QR);
	Ai_mat_ = Ai_mat_.t();
}