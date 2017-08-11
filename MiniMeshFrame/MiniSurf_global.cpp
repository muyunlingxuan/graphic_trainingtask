#include "MiniSurf_global.h"

int MiniSurf_global::getIndex(std::vector<HE_vert* > vertlist_, HE_vert* pVert) 
{
	for (int i = 0; i < vertlist_.size(); i++) 
	{
		if (pVert == vertlist_.at(i)) 
		{
			return i;
		}
	}
	return -1;
}


MiniSurf_global::~MiniSurf_global()
{

}

 
MiniSurf_global::MiniSurf_global(Mesh3D* mesh_)
{
	mesh = mesh_;
	vertlist_ = mesh->get_vertex_list(); //所有顶点
	int m = vertlist_->size();
	std::vector<T> coefficients;            // list of non-zeros coefficients
	Eigen::MatrixXd b(m, 3);                   // 等号右边的向量b，根据约束条件生成
	buildProblem(coefficients, b, m);
	SpMat A(m, m); // 等号左边的矩阵A
	A.setFromTriplets(coefficients.begin(), coefficients.end());

	// 求解
	Eigen::SparseQR<Eigen::SparseMatrix<double>, Eigen::COLAMDOrdering<int>> QR;
	QR.compute(A);


	//Eigen::SimplicialCholesky<SpMat> chol(A);  // 执行A的 Cholesky分解
	Eigen::MatrixXd x = QR.solve(b);         // 使用A的Cholesky分解来求解等号右边的向量b

										   // Export the result to a file:
	for (int i = 0; i < m; i++)
		{
			point p(x(i,0), x(i,1), x(i,2));
			if (!vertlist_->at(i)->isOnBoundary()) 
			{
				vertlist_->at(i)->set_position(p);
			}
		}
}


void MiniSurf_global::buildProblem(std::vector<T>& coefficients, Eigen::MatrixXd& b, int n)
{
	b.setZero();
	Eigen::ArrayXd boundary = Eigen::ArrayXd::LinSpaced(n, 0, 0).sin().pow(2);
	for (int i = 0; i < n; i++) 
	{
		HE_vert *pVert = vertlist_->at(i);
		
		if (!pVert->isOnBoundary())
		{
			double degree = pVert->degree();
			insertCoefficient(i, i, degree, coefficients, b, *pVert);

			for (int j = 0; j < degree; j++)
			{
				HE_vert *qVert = mesh->get_vertex(pVert->neighborIdx[j]);
				int k = getIndex(*vertlist_, qVert);
				insertCoefficient(i, k, -1, coefficients, b, *qVert);
			}
		}
	}
}

void MiniSurf_global::insertCoefficient(int i, int j, double w, std::vector<T>& coeffs,
	Eigen::MatrixXd& b, HE_vert& pVert)
{
	if (pVert.isOnBoundary()) 
	{
		point p = pVert.position();
		b(i, 0) -= w*p.at(0);
		b(i, 1) -= w*p.at(1);
		b(i, 2) -= w*p.at(2);
	}
	else 
	{
		coeffs.push_back(T(i, j, w));
	}
	
}