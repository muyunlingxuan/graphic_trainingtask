#include "boundaryCondition.h"
#include <math.h>
boundaryCondition::boundaryCondition()
:indexs_(NULL),points_(NULL),frames_(NULL), transform_matrix_(Eigen::Matrix4f::Identity(4, 4))
{
}

boundaryCondition::~boundaryCondition()
{
}

std::vector<int>* boundaryCondition::indexs()
{
	return indexs_;
}

std::vector<HE_vert*>* boundaryCondition::points()
{
	return points_;
}

std::vector<Eigen::Matrix3f>* boundaryCondition::frames()
{
	return frames_;
}

double boundaryCondition::weight()
{
	return weight_;
}

void boundaryCondition::Insert_indexs(int index)
{
	if (indexs_ == NULL)
	{
		indexs_ = new std::vector<int>;
	}
	indexs_->push_back(index);
}
void boundaryCondition::Insert_points(HE_vert* vert)
{
	if (points_ == NULL)
	{
		points_ = new std::vector<HE_vert*>;
	}
	points_->push_back(vert);
}

void boundaryCondition::Insert_frames(Eigen::Matrix3f frame)
{
	if (frames_== NULL)
	{
		frames_ = new std::vector<Eigen::Matrix3f>;
	}
	frames_->push_back(frame);
}

void boundaryCondition::Insert_weight(double weight)
{
	weight_ = weight;
}

void boundaryCondition::Clear()
{
	delete indexs_;
	delete points_;
	delete frames_;

	indexs_ = NULL;
	points_ = NULL;
	frames_ = NULL;
	weight_ = 0;
}

Eigen::Matrix4f boundaryCondition::transform_matrix()
{
	return transform_matrix_;
}



void boundaryCondition::set_rotate_matrix_(Eigen::Matrix3f matrix)
{	
	transform_matrix_.block(0, 0, 3, 3) = matrix;
}