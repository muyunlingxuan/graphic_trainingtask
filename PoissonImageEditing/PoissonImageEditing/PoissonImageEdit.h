#pragma once
#ifndef  POISSONIMAGEEDIT_H_
#define  POISSONIMAGEEDIT_H_

#include <Eigen/dense>
#include <Eigen/Sparse>
#include <Eigen/SparseQR>
#include <QImage>
QT_BEGIN_NAMESPACE
class QImage;
class QPainter;
QT_END_NAMESPACE
typedef Eigen::SparseMatrix<double> SpMat; // 声明一个列优先的双精度稀疏矩阵类型
typedef Eigen::Triplet<double> T; //三元组（行，列，值）

class PoissonImageEdit
{
public:
	PoissonImageEdit();
	~PoissonImageEdit();

	void makeMatrix_A(int cols_source, int rows_source);
	void makeMatrix_b(QPoint p_start, const QImage *image_);
	void clone(QImage* image_, QPoint p_start_);

private:
	void addBoundaryToMatrix_b(QPoint p_start, const QImage* image_);
	void calculate_newP();
	void write_newP(QImage* image_, QPoint p_start_);

	void set_elem(QRgb color, int index);
	void set_elem(QPoint p, int index, const QImage* image_ );
	 int get_grad1(QPoint p, QPoint q, const QImage* image_, int rgb);

	 int getIndex(int offset_x,int offset_y);
private:
	SpMat matrix_A;
	//Eigen::SparseLU<Eigen::SparseMatrix<double>, Eigen::COLAMDOrdering<int>> LU;
	Eigen::SimplicialLLT<Eigen::SparseMatrix<double>> LU;
	Eigen::MatrixXd matrix_b;
	Eigen::MatrixXd matrix_q;
	Eigen::MatrixXd matrix_p;
	int width;
	int height;
	int count_point;
};
#endif // ! POISSONIMAGEEDIT_H_

