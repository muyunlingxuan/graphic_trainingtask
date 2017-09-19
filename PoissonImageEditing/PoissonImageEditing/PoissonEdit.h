#pragma once
#ifndef POISSONEDIT_H_
#define POISSONEDIT_H_
#include <Eigen/dense>
#include <Eigen/Sparse>
#include <Eigen/SparseQR>
#include <QImage>

typedef Eigen::SparseMatrix<double> SpMat; // 声明一个列优先的双精度稀疏矩阵类型
typedef Eigen::Triplet<double> T; //三元组（行，列，值）

class PoissonEdit
{
public:
	PoissonEdit();
	~PoissonEdit();

public:
	void buildMatrix_A();
	void buildMatrix_b();
	void update_p();

private:
	Eigen::SparseLU<Eigen::SparseMatrix<double>, Eigen::COLAMDOrdering<int>> LU;
	SpMat A;
	Eigen::MatrixXd b;
	Eigen::MatrixXd p;

};

#endif // !POISSONEDIT_H_

