#pragma once
#pragma once
#ifndef Parameterize5_H_
#define Parameterize5_H_

#include <Eigen/dense>
#include <Eigen/Sparse>
#include <Eigen/SparseQR>
#include  <OpenMesh\Core\Mesh\TriMesh_ArrayKernelT.hh>

using namespace OpenMesh;
typedef TriMesh_ArrayKernelT<>  MyMesh;
typedef Eigen::SparseMatrix<double> SpMat; // 声明一个列优先的双精度稀疏矩阵类型
typedef Eigen::Triplet<double> T; //三元组（行，列，值）
class Parameterize5
{
public:
	Parameterize5(MyMesh *mesh_);
	~Parameterize5();

	void fixBoundary();
	void MakeNewPositions(void);
	void WriteNewPositions(void);
	void calculateAb(void);
	void setCoefficient(int index_p, MyMesh::VertexIter v_prt_it,std::vector<T>* coefficients);
	void calculateXMatrix(void);

	void mapToPlane(MyMesh::VertexIter v_ori_it, std::vector<MyMesh::Point>* vec_point_2d);
	void calculateWpi(MyMesh::VertexIter v_prt_it, MyMesh::VertexIter v_ori_it, std::vector<MyMesh::Point>* vec_point_2d, std::vector<float>* wp);
	void setMatrixAb(int index_p, MyMesh::VertexIter v_prt_it, std::vector<float>* wp, std::vector<T>* coefficients);
	double crossproduct2D(OpenMesh::Vec3f v1, OpenMesh::Vec3f v2);
	int getIndex(MyMesh::Point* p);
	void setTex(MyMesh* mesh_);
	double getAngle(MyMesh::Point p, MyMesh::Point q1, MyMesh::Point q2);
	bool isTriangle(MyMesh::Point A, MyMesh::Point B, MyMesh::Point C, MyMesh::Point P);
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

