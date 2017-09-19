#pragma once
#ifndef  BOUNDARYCONDITION_H_
#define   BOUNDARYCONDITION_H_
#include <QPoint>
#include <Eigen/dense>
#include <vector>
#include "HE_mesh\Mesh3D.h"

class boundaryCondition
{
public:
	boundaryCondition();
	~boundaryCondition();

	std::vector<int>					*indexs();
	std::vector<HE_vert*>				*points();
	std::vector<Eigen::Matrix3f>		*frames();
	Eigen::Matrix4f					transform_matrix();
	double									weight();

	void set_rotate_matrix_(Eigen::Matrix3f matrix);
	void Insert_indexs(int index);
	void Insert_points(HE_vert*);
	void Insert_frames(Eigen::Matrix3f);
	void Insert_weight(double weight);
	void Clear();
private:
	std::vector<int>						*indexs_;
	std::vector<HE_vert*>					*points_;
	std::vector<Eigen::Matrix3f>			*frames_;
	Eigen::Matrix4f							transform_matrix_;
	double									weight_;
};

#endif // ! BOUNDARYCONDITION_H_
