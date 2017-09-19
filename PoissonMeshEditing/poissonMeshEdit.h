#pragma once
#ifndef POISSONMESHEDIT_H_
#define POISSONMESHEDIT_H_
#include <vector>
#include <Eigen/dense>
#include <Eigen/Sparse>
#include <Eigen/SparseQR>
#include "boundaryCondition.h"
#include "HE_mesh\Mesh3D.h"
#define INFINY 65536
typedef Eigen::SparseMatrix<double> SpMat; // ����һ�������ȵ�˫����ϡ���������
typedef Eigen::Triplet<double> T; //��Ԫ�飨�У��У�ֵ��

class poissonMeshEdit
{
public:
	poissonMeshEdit(Mesh3D *mesh);
	~poissonMeshEdit();

public:
	void				set_coefficientA();
	
	void				set_bc_editable(std::vector<int>);
	void				set_boundary(std::vector<int> indexs_editable, std::vector<int> indexs_fixed);
	void				set_rotate_matrix(Eigen::Matrix3f matrix, std::vector<int> bc_edited,Vec3f center);

	void				set_coefficient_bc();
	void				set_value_bc();
	void				set_coefficient();

	void				Deformation();
	void				Calculate_local_frames(boundaryCondition bc_editable_);

private:
	double				ComputeLaplaceWeight(HE_vert *p, HE_vert *q, HE_vert *q_left, HE_vert *q_right);
	void				set_laplace_coor();
	void				set_bc_fixed(std::vector<int>);
	void				set_bc_edited(std::vector<int>);
	void				set_div_w();
	void				get_deformation_mesh();
	double				get_min_distance(HE_vert *vert, boundaryCondition bc);
	Eigen::Matrix4f		Propagate_transform_matrix_to_pVert(HE_vert *pVert);
	void				shortFloyd();
private:
	Mesh3D						*ptr_mesh_;
	//poisson function Matrix_left
	SpMat						coefficient_A_;//���е�������˹����
//	Eigen::MatrixXd				coefficient_bc_;//�߽�����
	SpMat						coefficient_;

	//poisson function Matrix_right
	Eigen::MatrixXd				laplace_coor_;//���е��������˹����
//	Eigen::MatrixXd				value_bc_;//�߽�����
	Eigen::MatrixXd				value_right_;

	boundaryCondition			bc_editable_;
	boundaryCondition			bc_edited_;
	boundaryCondition			bc_fixed_;

	int							count_points_;

	Vec3f						center_;
	
	//Ԥ�ֽⷽ��
//	Eigen::SimplicialLLT<Eigen::SparseMatrix<double>>							LU;
	Eigen::SparseQR<Eigen::SparseMatrix<double>, Eigen::COLAMDOrdering<int>>	LU;
//	Eigen::JacobiSVD<Eigen::MatrixXd>											svd;
//	Eigen::SimplicialLLT<Eigen::SparseMatrix<double>>							LU;

	Eigen::MatrixXd   shortPath;
	Eigen::MatrixXd  shortPosition;

};

#endif // !POISSONMESHEDIT_H_

