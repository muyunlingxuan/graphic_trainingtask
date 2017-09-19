#include "WarpingIDW_D.h"
#include <iostream>
#include <sys/timeb.h>

WarpingIDW_D::WarpingIDW_D() {

}

WarpingIDW_D::~WarpingIDW_D() {

}

void WarpingIDW_D::doWarp(cv::Mat *image_mat) {

	/*struct timeb startTime, endTime;
	ftime(&startTime);
	ftime(&endTime);
	cout << "1：循环语句运行了：" << (endTime.time - startTime.time) * 1000 + (endTime.millitm - startTime.millitm) << "毫秒" << endl;
	*/

	//setFixedPoints(image_mat);
	vec_D_mat_ = calculateMatrixD();
	warpImage(image_mat);

}

vector<cv::Mat> WarpingIDW_D::calculateMatrixD() {
	int num = vec_points_start_.rows;
	vector<cv::Mat> D;
	for (int i = 0; i < num; i++) {
		cv::Mat Di = calculateMatrixDi(i);
		D.push_back(Di);
	}
	return D;
}



cv::Mat WarpingIDW_D::warpImage(cv::Mat *image_mat) {
	cv::Mat image_backup = (*image_mat).clone();
	cv::MatIterator_<cv::Vec3b> iter, iterend;

	for (iter = (*image_mat).begin<cv::Vec3b>(), iterend = (*image_mat).end<cv::Vec3b>(); iter != iterend; ++iter)
	{
		(*iter)[0] = 245;
		(*iter)[1] = 245;
		(*iter)[2] = 245;
	}


	/*int nc = 0;
	if ((*image_mat).isContinuous())
	{
	nc = (*image_mat).rows*(*image_mat).cols*(*image_mat).channels();
	}
	uchar* data = (*image_mat).ptr<uchar>(0);
	for (int i = 0; i<nc; i++)
	{
	data[i] = 255;
	}*/

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
	return *image_mat;
}

/*f(p) = sum(wi(p)fi(p))
* wi = di/d
*di = 1/(dip)^u
*d=sum(di)
*/
cv::Point2d WarpingIDW_D::f_func(cv::Point2d p) {

	double d = getD_Reverse(p);
	cv::Point2d fi;
	double x = 0, y = 0, wi = 0;
	for (int i = 0; i < vec_points_start_.rows; i++) {
		fi = fi_func(p, i);
		wi = wi_func(p, d, i);

		x += fi.x*wi;
		y += fi.y*wi;
	}
	cv::Point2d q = cv::Point2d(x, y);
	return q;
}

cv::Point2d WarpingIDW_D::fi_func(cv::Point2d p, int i) {
	cv::Mat Di = vec_D_mat_.at(i);
	cv::Point2d pi = vec_points_start_.at<cv::Point2d>(i, 0);
	cv::Point2d qi = vec_points_end_.at<cv::Point2d>(i, 0);
	cv::Mat q_mat = cv::Mat::zeros(cv::Size(1, 2), CV_32FC1);
	cv::Mat pi_mat = cv::Mat::zeros(cv::Size(1, 2), CV_32FC1);
	cv::Mat qi_mat = cv::Mat::zeros(cv::Size(1, 2), CV_32FC1);
	pi_mat.at<float>(0, 0) = p.x - pi.x;
	pi_mat.at<float>(1, 0) = p.y - pi.y;
	qi_mat.at<float>(0, 0) = qi.x;
	qi_mat.at<float>(1, 0) = qi.y;

	q_mat = qi_mat + Di*pi_mat;
	cv::Point2d q = cv::Point2d(q_mat.at<float>(0, 0), q_mat.at<float>(1, 0));
	return q;
}

double WarpingIDW_D::wi_func(cv::Point2d p, double d, int i) {
	if (p.x == vec_points_start_.at<cv::Point2d>(i, 0).x
		&& p.y == vec_points_start_.at<cv::Point2d>(i, 0).y) {//p点是当前控制点
		return 1;
	}
	else if (isExist(vec_points_start_, p)) {//p点是控制点非当前控制点
		return 0;
	}
	else {//p点不是控制点
		double di = getD_Reverse(p, vec_points_start_.at<cv::Point2d>(i, 0));
		return (di / d);
	}
}


cv::Mat WarpingIDW_D::calculateMatrixDi(int i) {
	cv::Mat wi_matrix = calculateMatrixWi(i);
	int num = vec_points_start_.rows;

	cv::Mat Di = cv::Mat::eye(cv::Size(2, 2), CV_32FC1);
	if (num > 1) {
		cv::Point2d pi = vec_points_start_.at<cv::Point2d>(i, 0);
		cv::Point2d qi = vec_points_end_.at<cv::Point2d>(i, 0);

		cv::Mat pxy = cv::Mat::zeros(cv::Size(2, num - 1), CV_32FC1);
		cv::Mat qx = cv::Mat::zeros(cv::Size(1, num - 1), CV_32FC1);
		cv::Mat qy = cv::Mat::zeros(cv::Size(1, num - 1), CV_32FC1);
		int row = 0;
		for (int j = 0; j < vec_points_start_.rows; j++) {
			cv::Point2d pj = vec_points_start_.at<cv::Point2d>(j, 0);
			cv::Point2d qj = vec_points_end_.at<cv::Point2d>(j, 0);
			if (j != i) {
				pxy.at<float>(row, 0) = pj.x - pi.x;
				pxy.at<float>(row, 1) = pj.y - pi.y;

				qx.at<float>(row, 0) = qj.x - qi.x;
				qy.at<float>(row++, 0) = qj.y - qi.y;
			}
		}

		cv::Mat pxy_t = pxy.t();
		cv::Mat pxy_i = (pxy_t*wi_matrix*pxy).inv();

		cv::Mat dxi = (pxy_i*pxy_t*wi_matrix*qx).t();
		cv::Mat dyi = (pxy_i*pxy_t*wi_matrix*qy).t();
		(dxi).copyTo(Di.row(0));
		(dyi).copyTo(Di.row(1));
	}
	return Di;
}


cv::Mat WarpingIDW_D::calculateMatrixWi(int i) {
	int num = vec_points_start_.rows;
	int row = 0;
	cv::Mat wi_matrix = cv::Mat::eye(cv::Size(num - 1, num - 1), CV_32FC1);
	for (int j = 0; j < num; j++) {
		if (i != j) {
			double wij = getD_Reverse(vec_points_start_.at<cv::Point2d>(i, 0), vec_points_start_.at<cv::Point2d>(j, 0));
			wi_matrix.at<float>(row, row) = wij;
			row++;
		}
	}
	return wi_matrix;
}