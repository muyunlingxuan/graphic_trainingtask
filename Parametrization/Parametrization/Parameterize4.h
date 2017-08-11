#pragma once
#pragma once
#ifndef PARAMETERIZE4_H_
#define PARAMETERIZE4_H_

#include <Eigen/dense>
#include <Eigen/Sparse>
#include <Eigen/SparseQR>
#include  <OpenMesh\Core\Mesh\TriMesh_ArrayKernelT.hh>

using namespace OpenMesh;
typedef TriMesh_ArrayKernelT<>  MyMesh;
typedef Eigen::SparseMatrix<double> SpMat; // 声明一个列优先的双精度稀疏矩阵类型
typedef Eigen::Triplet<double> T; //三元组（行，列，值）
class Parameterize4
{
public:
	Parameterize4(MyMesh *mesh_);
	~Parameterize4();
	
	void fixBoundary();
	void MakeNewPositions(void);
	void WriteNewPositions(void);
	void calculateAb(void);
	void setCoefficient(int index_p, MyMesh::VertexIter v_prt_it, MyMesh::VertexIter v_ori_it, std::vector<T>* coefficients);
	void calculateXMatrix(void);

	void mapToPlane(MyMesh::VertexIter v_ori_it, std::vector<OpenMesh::Vec2f>* vec_point_2d);
	void calculateWpi(MyMesh::VertexIter v_prt_it, MyMesh::VertexIter v_ori_it, std::vector<OpenMesh::Vec2f>* vec_point_2d, std::vector<float>* wp);
	void setMatrixAb(int index_p, MyMesh::VertexIter v_prt_it, std::vector<float>* wp, std::vector<T>* coefficients);
	double crossproduct2D(OpenMesh::Vec2f v1, OpenMesh::Vec2f v2);
	int Parameterize4::getIndex(MyMesh::Point* p);
	void Parameterize4::setTex(MyMesh* mesh_);
private:
	MyMesh ori_mesh;
	MyMesh *prt_mesh;

	std::vector<MyMesh::Point> cogs;
	std::vector<MyMesh::Point>::iterator cog_it;
	SpMat A_matrix_;
	Eigen::MatrixXd B_matrix_;
	Eigen::MatrixXd X_matrix_;
};
#endif

