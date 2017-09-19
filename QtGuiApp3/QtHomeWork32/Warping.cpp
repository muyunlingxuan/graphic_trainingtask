#include "Warping.h"
#include <iostream>
Warping::Warping()
{
}


Warping::~Warping()
{
}

void Warping::setControlPoint(vector<QPoint> &ps, vector<QPoint> &qs) {
	resetControlPoint();
	for (int i = 0; i < ps.size(); i++) {
		
		vec_points_start_.push_back(cv::Point2d(ps[i].x(), ps[i].y()));
		vec_points_end_.push_back(cv::Point2d(qs[i].x(), qs[i].y()));
	}
}

void Warping::resetControlPoint() {
	vec_points_start_.release();
	vec_points_end_.release();
}

void Warping::setFixedPoints(cv::Mat *image) {
	cv::Point2d *p = new cv::Point2d(0, 0);
	vec_points_start_.push_back(*p);
	vec_points_end_.push_back(*p);

	p = new cv::Point2d(0, (*image).cols - 1);
	vec_points_start_.push_back(*p);
	vec_points_end_.push_back(*p);

	p = new cv::Point2d((*image).rows - 1, 0);
	vec_points_start_.push_back(*p);
	vec_points_end_.push_back(*p);

	p = new cv::Point2d((*image).rows - 1, (*image).cols - 1);
	vec_points_start_.push_back(*p);
	vec_points_end_.push_back(*p);
}

double Warping::getD_Reverse(cv::Point2d p) {
	double d = 0.0;
	for (int j = 0; j < vec_points_start_.rows; j++)
	{
		d += getD_Reverse(p, vec_points_start_.at<cv::Point2d>(j, 0));
	}
	return d;
}

double Warping::getD_Reverse(cv::Point2d p, cv::Point2d q) {
	double d = (p.x - q.x) * (p.x - q.x) + (p.y - q.y) * (p.y - q.y);
	return  1 / d;
}

double Warping::getDistance(cv::Point2d p, cv::Point2d q) {
	double d = (p.x - q.x) * (p.x - q.x) + (p.y - q.y) * (p.y - q.y);
	return  d;
}


bool Warping::isExist(cv::Mat vec_points_, cv::Point2d p) {
	for (int i = 0; i < vec_points_.rows; i++) {
		if (p.x == vec_points_.at<cv::Point2d>(i,0).x 
			&& p.y == vec_points_.at<cv::Point2d>(i, 0).y) {
			return true;
		}
	}
	return false;
}

void Warping::holeFilling(cv::Mat *image) {
	std::vector<cv::Point2f> dataset_vec_;
	std::vector<cv::Point2f> query_vec_;
	try {
		for (int i = 0; i < (*image).rows; i++)
		{
			for (int j = 0; j < (*image).cols; j++)
			{
				cv::Vec3b item = (*image).at<cv::Vec3b>(j, i);
				if (item[0] == 245 && item[1] == 245 && item[2] == 245) {
					query_vec_.push_back(cv::Point2f(j, i));
				}
				else {
					dataset_vec_.push_back(cv::Point2f(j, i));
				}
				
			}
		}
		cv::Mat dataset = cv::Mat(dataset_vec_).reshape(1);//图片像素点
		cv::Mat query = cv::Mat(query_vec_).reshape(1);//所有空白点
		cv::Mat indeces = cv::Mat::zeros(cv::Size(query_vec_.size(), 1), CV_32FC1);//距空白点最近的像素点的索引集合
		cv::Mat dists = cv::Mat::zeros(cv::Size(query_vec_.size(), 1), CV_32FC1);//空白的据最近的像素点的距离集合

		cv::flann::KDTreeIndexParams indexParams(2);
		cv::flann::SearchParams params(32);
		cv::flann::Index kdtree(dataset, indexParams);

		kdtree.knnSearch(query, indeces, dists, 1, params);

		for (int i = 0; i < dists.rows; i++) {
			if (dists.at<float>(i, 0) < 50) {
				int j = indeces.at<int>(i, 0);
				cv::Point2f p = query_vec_[i];
				cv::Point2f q = dataset_vec_[j];
				(*image).at<cv::Vec3b>(p.x, p.y) = (*image).at<cv::Vec3b>(q.x, q.y);
			}
		}
	}
	catch (exception e) {
		cout << e.what() << endl;
	}
}