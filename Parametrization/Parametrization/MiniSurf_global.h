#pragma once
#ifndef MINISURF_GLOBAL_H_
#define MINISURF_GLOBAL_H_

#include <Eigen/dense>
#include <Eigen/Sparse>
#include <Eigen/SparseQR>
#include  <OpenMesh\Core\Mesh\TriMesh_ArrayKernelT.hh>

using namespace OpenMesh;
typedef TriMesh_ArrayKernelT<>  MyMesh;
typedef Eigen::SparseMatrix<double> SpMat; // 声明一个列优先的双精度稀疏矩阵类型
typedef Eigen::Triplet<double> T; //三元组（行，列，值）

class MiniSurf_global
{
public:
	MiniSurf_global(MyMesh * mesh);
	~MiniSurf_global();

private:
	void Init(void);
	void MakeNewPositions(void);
	void WriteNewPositions(void);
	void calculateAb(void);
	void calculateXMatrix(void);	
	int getIndex(MyMesh::Point* p);
	void setCoefficient(int i, MyMesh::VertexIter v_it, std::vector<T> *coefficients);
private:

	MyMesh* ptr_mesh_;
	std::vector<MyMesh::Point> cogs;
	std::vector<MyMesh::Point>::iterator cog_it;
	SpMat A_matrix_;
	Eigen::MatrixXd B_matrix_;
	Eigen::MatrixXd X_matrix_;
};
#endif // !MINISURF_GLOBAL_H_
