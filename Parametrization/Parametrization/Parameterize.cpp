#include "Parameterize.h"
#include <iostream>


Parameterize::Parameterize(MyMesh* mesh_)
{
	ptr_mesh_ = mesh_;
	Init();
}


Parameterize::~Parameterize()
{
}

void Parameterize::doParameterize() {
	fixBoundary();
	MakeNewPositions();
	WriteNewPositions();
}

void Parameterize::fixBoundary() {
	//get the boundary point
	HalfedgeHandle edge_it, init_it;
	bool firstEdgeFlag = true;
	int count_point_boundary = 0, count_point_pb = 0;
	double interval = 0;
	for (auto it = ptr_mesh_->halfedges_begin(); it != ptr_mesh_->halfedges_end(); ++it)
	{
		if (ptr_mesh_->is_boundary(it))
		{
			if (firstEdgeFlag) {
				edge_it = init_it = *it;
				firstEdgeFlag = false;
			}
			count_point_boundary++;
		}
	}
	/*for (auto it = ptr_mesh_->halfedges_begin(); it != ptr_mesh_->halfedges_end(); ++it)
	{
		if (ptr_mesh_->is_boundary(it))
		{
			MyMesh::Point p_init = ptr_mesh_->point(ptr_mesh_->from_vertex_handle(edge_it));
			MyMesh::Point p_it = ptr_mesh_->point(ptr_mesh_->from_vertex_handle(*it));
			if (p_init.data()[0] > p_it.data()[0]) {
				edge_it = init_it = *it;
			}
		}
	}*/

	count_point_pb = count_point_boundary / 4;
	if (count_point_boundary % 4 != 0) {
		count_point_pb++;
	}
	interval = 1 / (double)count_point_pb;

	ptr_mesh_->set_point(ptr_mesh_->from_vertex_handle(edge_it), MyMesh::Point(0.0f));
	edge_it = ptr_mesh_->next_halfedge_handle(edge_it);
	int col = 0,row=0;
	for (int j = 1; j < count_point_boundary; j++) {
		row = j / count_point_pb;
		col = j%count_point_pb;
		if (row == 0) {
			ptr_mesh_->set_point(ptr_mesh_->from_vertex_handle(edge_it), MyMesh::Point(0, col*interval, 0));
		}
		else if (row==1) {
			ptr_mesh_->set_point(ptr_mesh_->from_vertex_handle(edge_it), MyMesh::Point(col*interval, 1, 0));
		}
		else if (row == 2) {
			ptr_mesh_->set_point(ptr_mesh_->from_vertex_handle(edge_it), MyMesh::Point(1, 1 - col*interval, 0));
		}
		else if (row == 3) {
			ptr_mesh_->set_point(ptr_mesh_->from_vertex_handle(edge_it), MyMesh::Point(1 - col*interval, 0, 0));
		}
		edge_it = ptr_mesh_->next_halfedge_handle(edge_it);
	}
}

void Parameterize::Init() {
	int m = ptr_mesh_->n_vertices();
	cogs.reserve(m);
	A_matrix_.resize(m, m);
	B_matrix_ = Eigen::MatrixXd::Zero(m, 3);
}

void Parameterize::MakeNewPositions() {
	//Ax=b
	calculateAb();
	calculateXMatrix();
}

void Parameterize::calculateAb() {
	std::vector<T> coefficients;
	MyMesh::VertexIter v_it, v_itend(ptr_mesh_->vertices_end());
	int i = 0;

	for (v_it = ptr_mesh_->vertices_begin(); v_it != v_itend; v_it++, i++) {
		if (!ptr_mesh_->is_boundary(*v_it)) {
			Param->setCoefficient(i, v_it, &coefficients);
		}
		else {
			coefficients.push_back(T(i, i, 1));

			B_matrix_(i, 0) += ptr_mesh_->point(*v_it).data()[0];
			B_matrix_(i, 1) += ptr_mesh_->point(*v_it).data()[1];
			B_matrix_(i, 2) += ptr_mesh_->point(*v_it).data()[2];
		}
	}
	A_matrix_.setFromTriplets(coefficients.begin(), coefficients.end());
}

void Parameterize::calculateXMatrix() {
	Eigen::SparseQR<Eigen::SparseMatrix<double>, Eigen::COLAMDOrdering<int>> QR;
	QR.compute(A_matrix_);
	X_matrix_ = QR.solve(B_matrix_);
}

void Parameterize::WriteNewPositions() {
	MyMesh::VertexIter v_it, v_itend(ptr_mesh_->vertices_end());
	int i = 0;
	for (v_it = ptr_mesh_->vertices_begin(), cog_it = cogs.begin();
		v_it != v_itend; v_it++, i++) {
		if (!ptr_mesh_->is_boundary(*v_it)) {
			ptr_mesh_->point(*v_it).data()[0] = X_matrix_(i, 0);
			ptr_mesh_->point(*v_it).data()[1] = X_matrix_(i, 1);
			ptr_mesh_->point(*v_it).data()[2] = X_matrix_(i, 2);
		}
	}
}

int Parameterize::getIndex(MyMesh::Point* p) {
	int i = 0;
	MyMesh::VertexIter v_it, v_itend(ptr_mesh_->vertices_end());
	for (v_it = ptr_mesh_->vertices_begin(); v_it != v_itend; v_it++, i++) {
		if (*p == ptr_mesh_->point(*v_it)) {
			return i;
		}
	}
}

void Parameterize::setTex(MyMesh* mesh_) {

	for (auto v_it = ptr_mesh_->vertices_begin(), v2_it = mesh_->vertices_begin(); v2_it != mesh_->vertices_end(); v_it++, v2_it++) {
		OpenMesh::Vec2f texcoord(ptr_mesh_->point(*v_it).data()[0], ptr_mesh_->point(*v_it).data()[1]);
		mesh_->set_texcoord2D(*v2_it, texcoord);
	}
}
