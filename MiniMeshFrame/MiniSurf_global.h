#pragma once
#include <Eigen/dense>
#include "HE_mesh/Mesh3D.h"
#include <Eigen/Sparse>
#include <Eigen/SparseQR>

typedef Eigen::SparseMatrix<double> SpMat; // ����һ�������ȵ�˫����ϡ���������
typedef Eigen::Triplet<double> T; //��Ԫ�飨�У��У�ֵ��

class MiniSurf_global
{
public:
	MiniSurf_global(Mesh3D * mesh);
	~MiniSurf_global();

private:
	int getIndex(std::vector<HE_vert* > vertlist_, HE_vert* pVert);
	void insertCoefficient(int i, int j, double w, std::vector<T>& coefficients,
		Eigen::MatrixXd& b, HE_vert& pVert);
	void buildProblem(std::vector<T>& coefficients, Eigen::MatrixXd& b, int n);

private:

	Mesh3D* mesh;
	std::vector<HE_vert* > *vertlist_;//mesh�����ж���
	Eigen::SparseMatrix<double> A_matrix_;
};