#include "MiniSurf_global.h"
#include <iostream>

MiniSurf_global::~MiniSurf_global()
{
}

void MiniSurf_global::Init() {
	int m = ptr_mesh_->n_vertices();
	cogs.reserve(m);
	A_matrix_.resize(m, m);
	B_matrix_ = Eigen::MatrixXd::Zero(m, 3);
}

MiniSurf_global::MiniSurf_global(MyMesh* mesh_)
{
	ptr_mesh_ = mesh_;
	Init();
	MakeNewPositions();
	WriteNewPositions();
}

void MiniSurf_global::MakeNewPositions() {
	//Ax=b
	calculateAb();
	calculateXMatrix();
}

void MiniSurf_global::calculateAb() {
	std::vector<T> coefficients;
	MyMesh::VertexIter v_it, v_itend(ptr_mesh_->vertices_end());
	int i = 0;

	for (v_it = ptr_mesh_->vertices_begin(); v_it != v_itend; v_it++, i++) {
		if (!ptr_mesh_->is_boundary(*v_it)) {
			setCoefficient(i, v_it, &coefficients);
		}
	}
	A_matrix_.setFromTriplets(coefficients.begin(), coefficients.end());
}

void MiniSurf_global::calculateXMatrix() {
	Eigen::SparseQR<Eigen::SparseMatrix<double>, Eigen::COLAMDOrdering<int>> QR;
	QR.compute(A_matrix_);
	X_matrix_ = QR.solve(B_matrix_);
}

void MiniSurf_global::WriteNewPositions() {
	MyMesh::VertexIter v_it, v_itend(ptr_mesh_->vertices_end());
	int i = 0;
	for (v_it = ptr_mesh_->vertices_begin(), cog_it = cogs.begin();
		v_it != v_itend; v_it++,i++) {
		if (!ptr_mesh_->is_boundary(*v_it)) {
			ptr_mesh_->point(*v_it).data()[0] = X_matrix_(i, 0);
			ptr_mesh_->point(*v_it).data()[1] = X_matrix_(i, 1);
			ptr_mesh_->point(*v_it).data()[2] = X_matrix_(i, 2);
		}
	}
}

/*
aii = degree_i;
aij = -1
b = sum(bi)
*/
void MiniSurf_global::setCoefficient(int i, MyMesh::VertexIter v_it, std::vector<T>* coefficients) {
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

/*
wij = 1/|pi-pj|
aij = -wij
aii = sum(wij)
b = wij j-±ß½çµã
*/
//void MiniSurf_global::setCoefficient(int i, MyMesh::VertexIter v_it, std::vector<T>* coefficients) {
//	MyMesh::VertexVertexIter vv_it;
//	float wij = 0;
//	float wi = 0;
//	for (vv_it = ptr_mesh_->vv_iter(*v_it); vv_it.is_valid(); vv_it++) {
//		MyMesh::Point pi = ptr_mesh_->point(*v_it);
//		MyMesh::Point pj = ptr_mesh_->point(*vv_it);
//		MyMesh::Point pij = pi - pj;
//		wij = pij.data()[0] * pij.data()[0] + pij.data()[1] * pij.data()[1] + pij.data()[2] * pij.data()[2];
//		wij = sqrt(wij);
//		wi += wij;
//		if (ptr_mesh_->is_boundary(*vv_it)) {
//			B_matrix_(i, 0) += wij*ptr_mesh_->point(*vv_it).data()[0];
//			B_matrix_(i, 1) += wij*ptr_mesh_->point(*vv_it).data()[1];
//			B_matrix_(i, 2) += wij*ptr_mesh_->point(*vv_it).data()[2];
//		}
//		else {
//			int j = getIndex(&ptr_mesh_->point(*vv_it));
//			coefficients->push_back(T(i, j, -wij));
//		}
//	}
//	coefficients->push_back(T(i, i, wi));
//}

int MiniSurf_global::getIndex(MyMesh::Point* p) {
	int i = 0;
	MyMesh::VertexIter v_it, v_itend(ptr_mesh_->vertices_end());
	for (v_it = ptr_mesh_->vertices_begin(); v_it != v_itend; v_it++, i++) {
		if (*p == ptr_mesh_->point(*v_it)) {
			return i;
		}
	}
}