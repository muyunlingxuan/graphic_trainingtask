#pragma once
#ifndef PARAMETERIZE_H_
#define PARAMETERIZE_H_

#include <Eigen/dense>
#include <Eigen/Sparse>
#include <Eigen/SparseQR>
#include  <OpenMesh\Core\Mesh\TriMesh_ArrayKernelT.hh>

using namespace OpenMesh;
typedef TriMesh_ArrayKernelT<>  MyMesh;
typedef Eigen::SparseMatrix<double> SpMat; // 声明一个列优先的双精度稀疏矩阵类型
typedef Eigen::Triplet<double> T; //三元组（行，列，值）

class Parameterize
{

public:
	Parameterize(MyMesh * mesh);
	~Parameterize();
	void setTex(MyMesh* mesh_);

//protected:
public:
	void Init(void);
	void doParameterize();
	void fixBoundary();
	void MakeNewPositions(void);
	void WriteNewPositions(void);
	void calculateAb(void);
	void calculateXMatrix(void);
	int getIndex(MyMesh::Point* p);
	virtual void setCoefficient(int i, MyMesh::VertexIter v_it, std::vector<T> *coefficients) {}

//protected:
public:
	MyMesh* ptr_mesh_;
	std::vector<MyMesh::Point> cogs;
	std::vector<MyMesh::Point>::iterator cog_it;
	SpMat A_matrix_;
	Eigen::MatrixXd B_matrix_;
	Eigen::MatrixXd X_matrix_;
	Parameterize *Param;

};
#endif // !PARAMETERIZE_H_

