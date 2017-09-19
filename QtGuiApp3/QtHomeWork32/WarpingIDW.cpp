#include "WarpingIDW.h"
#include <iostream>
#include <sys/timeb.h>

WarpingIDW::WarpingIDW() {

}

WarpingIDW::~WarpingIDW() {

}

void WarpingIDW::doWarp(cv::Mat *image_mat) {
	//setFixedPoints(image_mat);
	warpImage(image_mat);
	
}


cv::Mat WarpingIDW::warpImage(cv::Mat *image_mat) {
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
			if (q.x > 0 && q.x< image_backup.cols && q.y > 0 && q.y < image_backup.rows) {
				
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
cv::Point2d WarpingIDW::f_func(cv::Point2d p) {

	double d = getD_Reverse(p);
	cv::Point2d fi;
	double x = 0, y = 0, wi = 0;
	for (int i = 0; i < vec_points_start_.rows; i++){
		fi = fi_func(p, i);
		wi = wi_func(p, d, i);

		x += fi.x*wi;
		y += fi.y*wi;
	}
	cv::Point2d q = cv::Point2d(x, y);
	return q;
}

cv::Point2d WarpingIDW::fi_func(cv::Point2d p, int i) {
	cv::Point2d pi = vec_points_start_.at<cv::Point2d>(i, 0);
	cv::Point2d qi = vec_points_end_.at<cv::Point2d>(i, 0);
	cv::Point2d q = cv::Point2d(qi.x + (p.x - pi.x), qi.y + (p.y - pi.y));
	return q;
}

double WarpingIDW::wi_func(cv::Point2d p, double d, int i) {
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
