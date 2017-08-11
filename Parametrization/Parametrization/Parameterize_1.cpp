#include "Parameterize_1.h"
#include <iostream>

/*	固定边界点
	与极小化相同的方式确定内点坐标
	根据固定边界点计算Ax=b的系数矩阵A
	Aii = degree_point_i;
	Aij = 1 (ij相连)
	Aij = 0 (ij不相连)
	根据Ax=b确定所有内点x的坐标

	这种方式只考虑各点之间的连接关系
*/
Parameterize_1::Parameterize_1(MyMesh *mesh):Parameterize(mesh)
{
	Param = this;
	doParameterize();
}

Parameterize_1::~Parameterize_1()
{
}

/*
aii = degree_i;
aij = -1
b = sum(bi)
*/
void Parameterize_1::setCoefficient(int i, MyMesh::VertexIter v_it, std::vector<T>* coefficients) {
	MyMesh::Scalar valence = 0.0;
	MyMesh::VertexVertexIter vv_it;
	for (vv_it = ptr_mesh_->vv_iter(*v_it); vv_it.is_valid(); vv_it++) {
		if (ptr_mesh_->is_boundary(*vv_it)) {
			B_matrix_(i, 0) += ptr_mesh_->point(*vv_it).data()[0];
			B_matrix_(i, 1) += ptr_mesh_->point(*vv_it).data()[1];
			B_matrix_(i, 2) += ptr_mesh_->point(*vv_it).data()[2];
		}
		else {
			int j = getIndex(&ptr_mesh_->point(*vv_it));
			coefficients->push_back(T(i, j, -1));
		}
		valence++;
	}
	coefficients->push_back(T(i, i, valence));
	
}


