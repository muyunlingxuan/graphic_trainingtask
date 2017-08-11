#include <math.h>
#include "Parameterize5.h"
#include <iostream>


Parameterize5::Parameterize5(MyMesh *mesh_)
{
	ori_mesh = *mesh_;
	prt_mesh = mesh_;
	int m = prt_mesh->n_vertices();
	cogs.reserve(m);
	A_matrix_.resize(m, m);
	B_matrix_ = Eigen::MatrixXd::Zero(m, 3);

	//固定点
	fixBoundary();
	MakeNewPositions();
	WriteNewPositions();
}


Parameterize5::~Parameterize5()
{
}

void Parameterize5::calculateXMatrix() {
	Eigen::SparseQR<Eigen::SparseMatrix<double>, Eigen::COLAMDOrdering<int>> QR;
	QR.compute(A_matrix_);
	X_matrix_ = QR.solve(B_matrix_);
}

void Parameterize5::mapToPlane(MyMesh::VertexIter v_it, std::vector<MyMesh::Point>* vec_point_new) {
	MyMesh::Point p = prt_mesh->point(*v_it);
	double angle_sum = 0;
	//遍历p点的周围顶点计算角度和
	MyMesh::VertexVertexIter vv_it_begin = ori_mesh.vv_iter(*v_it);
	MyMesh::VertexVertexIter vv_it_q1 = vv_it_begin, vv_it_q2 = vv_it_begin;
	vv_it_q2++;

	do {
		MyMesh::Point q1 = ori_mesh.point(*vv_it_q1);
		MyMesh::Point q2 = !vv_it_q2.is_valid() ? ori_mesh.point(*vv_it_begin) : ori_mesh.point(*vv_it_q2);
		angle_sum += getAngle(p, q1, q2);
		vv_it_q1++; vv_it_q2++;

	} while (vv_it_q1.is_valid());
	float length = (p - ori_mesh.point(*vv_it_begin)).norm();
	vec_point_new->push_back(MyMesh::Point(length, 0, 0));

	double angle = 0;
	double angle_new = 0;

	vv_it_q1 = vv_it_begin, vv_it_q2 = vv_it_begin;
	vv_it_q2++;
	do {
		MyMesh::Point q1 = ori_mesh.point(*vv_it_q1);
		MyMesh::Point q2 = ori_mesh.point(*vv_it_q2);

		angle += getAngle(p, q1, q2);
		angle_new = angle * 2 * M_PI / angle_sum;
		length = (p - q2).norm();

		vec_point_new->push_back(MyMesh::Point(length*cos(angle_new), length*sin(angle_new), 0));

		vv_it_q1++; vv_it_q2++;

	} while (vv_it_q2.is_valid());
	for (int i = 0; i < vec_point_new->size(); i++) {
		std::cout << vec_point_new->at(i)[0]<<"," << vec_point_new->at(i)[1]<<"," << vec_point_new->at(i)[2] << std::endl;
	}
}


double Parameterize5::getAngle(MyMesh::Point p, MyMesh::Point q1, MyMesh::Point q2) {
	MyMesh::Normal v1 = q1 - p;
	MyMesh::Normal v2 = q2 - p;
	double cospij = OpenMesh::dot(v1.normalize(), v2.normalize());
	return acos(cospij);
}

double Parameterize5::crossproduct2D(OpenMesh::Vec3f v1, OpenMesh::Vec3f v2) {
	return v1[0] * v2[1] - v1[1] * v2[0];
}

void Parameterize5::calculateWpi(MyMesh::VertexIter v_prt_it, MyMesh::VertexIter v_ori_it, std::vector<MyMesh::Point>* vec_point_new, std::vector<float>* wp) {
	MyMesh::Point p = MyMesh::Point(0.0f);
	double angle = 0;
	for (int i = 0; i < vec_point_new->size(); i++) {
		angle = 0;
		int j = (i + 1) % vec_point_new->size();
		while (j != i) {
			int j_left = j == 0 ? vec_point_new->size() - 1 : j - 1;
			int j_right = (j + 1) % vec_point_new->size();
			angle += getAngle(p, vec_point_new->at(j_left), vec_point_new->at(j));
			if (angle >= M_PI) {
				if (!isTriangle(vec_point_new->at(i), vec_point_new->at(j), vec_point_new->at(j_left), p)) {
					OpenMesh::Vec3f  vpi = vec_point_new->at(i) - p;
					OpenMesh::Vec3f  vpj = vec_point_new->at(j) - p;
					OpenMesh::Vec3f  vpj_left = vec_point_new->at(j_left) - p;

					OpenMesh::Vec3f  vi1 = vec_point_new->at(j) - vec_point_new->at(i);
					OpenMesh::Vec3f  vi2 = vec_point_new->at(j_left) - vec_point_new->at(i);

					float si = (vpj%vpj_left).norm() / (vi1%vi2).norm();
					float sj_left = (vpj%vpi).norm() / (vi1%vi2).norm();
					float sj = (vpj_left%vpi).norm() / (vi1%vi2).norm();
				}
				if (!isTriangle(vec_point_new->at(i), vec_point_new->at(j), vec_point_new->at(j_left), p)) {
				}
				//if (isTriangle(vec_point_new->at(i), vec_point_new->at(j), vec_point_new->at(j_left), p)) {
					OpenMesh::Vec3f  vpi = vec_point_new->at(i) - p;
					OpenMesh::Vec3f  vpj = vec_point_new->at(j) - p;
					OpenMesh::Vec3f  vpj_left = vec_point_new->at(j_left) - p;

					OpenMesh::Vec3f  vi1 = vec_point_new->at(j) - vec_point_new->at(i);
					OpenMesh::Vec3f  vi2 = vec_point_new->at(j_left) - vec_point_new->at(i);

					wp->at(i) += (vpj%vpj_left).norm() / (vi1%vi2).norm();
					wp->at(j_left) += (vpj%vpi).norm() / (vi1%vi2).norm();
					wp->at(j) += (vpj_left%vpi).norm() / (vi1%vi2).norm();
					break;
				/*}
				else if (isTriangle(vec_point_new->at(i), vec_point_new->at(j), vec_point_new->at(j_right), p)) {
					OpenMesh::Vec3f  vpi = vec_point_new->at(i) - p;
					OpenMesh::Vec3f vpj = vec_point_new->at(j) - p;
					OpenMesh::Vec3f  vpj_right = vec_point_new->at(j_right) - p;
					OpenMesh::Vec3f  vi1 = vec_point_new->at(j) - vec_point_new->at(i);
					OpenMesh::Vec3f  vi2 = vec_point_new->at(j_right) - vec_point_new->at(i);

					wp->at(i) += (vpj%vpj_right).norm() / (vi1%vi2).norm();
					wp->at(j_right) += (vpj%vpi).norm() / (vi1%vi2).norm();
					wp->at(j) += (vpj_right%vpi).norm() / (vi1%vi2).norm();
					break;
				}*/

			}
			j = (j + 1) % vec_point_new->size();

		}
	}
}

void Parameterize5::setMatrixAb(int index_p, MyMesh::VertexIter v_prt_it, std::vector<float>* wp, std::vector<T>* coefficients) {
	int i = 0;
	int valence = wp->size();
	int m = prt_mesh->n_vertices();
	MyMesh::VertexVertexIter vv_it = prt_mesh->vv_iter(*v_prt_it);

	for (int i = 0; vv_it.is_valid(); vv_it++, i++) {
		float wpi = wp->at(i);
		if (prt_mesh->is_boundary(*vv_it)) {
			B_matrix_(index_p, 0) += wpi*(prt_mesh->point(*vv_it).data()[0]);
			B_matrix_(index_p, 1) += wpi*(prt_mesh->point(*vv_it).data()[1]);
			B_matrix_(index_p, 2) += wpi*(prt_mesh->point(*vv_it).data()[2]);
		}
		else {
			int index_i = getIndex(&prt_mesh->point(*vv_it));
			coefficients->push_back(T(index_p, index_i, -wpi));
		}
	}

	coefficients->push_back(T(index_p, index_p, valence));
}

void Parameterize5::setCoefficient(int index_p, MyMesh::VertexIter v_prt_it, std::vector<T>* coefficients) {
	MyMesh::VertexIter v_ori_it = v_prt_it;
	std::vector<OpenMesh::Vec3f> vec_point_new;
	mapToPlane(v_ori_it, &vec_point_new);
	std::vector<float> wp(vec_point_new.size());
	calculateWpi(v_prt_it, v_ori_it, &vec_point_new, &wp);
	setMatrixAb(index_p, v_prt_it, &wp, coefficients);
}

void Parameterize5::calculateAb() {
	std::vector<T> coefficients;
	MyMesh::VertexIter v_it, v_itend(prt_mesh->vertices_end());
	int i = 0;

	for (v_it = prt_mesh->vertices_begin(); v_it != v_itend; v_it++, i++) {
		if (!prt_mesh->is_boundary(*v_it)) {
			setCoefficient(i, v_it, &coefficients);
		}
		else {
			coefficients.push_back(T(i, i, 1));

			B_matrix_(i, 0) += prt_mesh->point(*v_it).data()[0];
			B_matrix_(i, 1) += prt_mesh->point(*v_it).data()[1];
			B_matrix_(i, 2) += prt_mesh->point(*v_it).data()[2];
		}
	}
	A_matrix_.setFromTriplets(coefficients.begin(), coefficients.end());
}



void Parameterize5::WriteNewPositions() {
	MyMesh::VertexIter v_it, v_itend(prt_mesh->vertices_end());
	int i = 0;
	for (v_it = prt_mesh->vertices_begin(), cog_it = cogs.begin();
		v_it != v_itend; v_it++, i++) {
		if (!prt_mesh->is_boundary(*v_it)) {
			prt_mesh->point(*v_it).data()[0] = X_matrix_(i, 0);
			prt_mesh->point(*v_it).data()[1] = X_matrix_(i, 1);
			prt_mesh->point(*v_it).data()[2] = X_matrix_(i, 2);
		}
	}
}
void Parameterize5::MakeNewPositions() {
	//Ax=b
	calculateAb();
	calculateXMatrix();
}

void Parameterize5::fixBoundary() {
	//get the boundary point
	HalfedgeHandle edge_it, init_it;
	bool firstEdgeFlag = true;
	int count_point_boundary = 0, count_point_pb = 0;
	double interval = 0;
	for (auto it = prt_mesh->halfedges_begin(); it != prt_mesh->halfedges_end(); ++it)
	{
		if (prt_mesh->is_boundary(it))
		{
			if (firstEdgeFlag) {
				edge_it = init_it = *it;
				firstEdgeFlag = false;
			}
			count_point_boundary++;
		}
	}
	
	count_point_pb = count_point_boundary / 4;
	if (count_point_boundary % 4 != 0) {
		count_point_pb++;
	}
	interval = 1 / (double)count_point_pb;

	prt_mesh->set_point(prt_mesh->from_vertex_handle(edge_it), MyMesh::Point(0.0f));
	edge_it = prt_mesh->next_halfedge_handle(edge_it);
	int col = 0, row = 0;
	for (int j = 1; j < count_point_boundary; j++) {
		row = j / count_point_pb;
		col = j%count_point_pb;
		if (row == 0) {
			prt_mesh->set_point(prt_mesh->from_vertex_handle(edge_it), MyMesh::Point(0, col*interval, 0));
		}
		else if (row == 1) {
			prt_mesh->set_point(prt_mesh->from_vertex_handle(edge_it), MyMesh::Point(col*interval, 1, 0));
		}
		else if (row == 2) {
			prt_mesh->set_point(prt_mesh->from_vertex_handle(edge_it), MyMesh::Point(1, 1 - col*interval, 0));
		}
		else if (row == 3) {
			prt_mesh->set_point(prt_mesh->from_vertex_handle(edge_it), MyMesh::Point(1 - col*interval, 0, 0));
		}
		edge_it = prt_mesh->next_halfedge_handle(edge_it);
	}
}

int Parameterize5::getIndex(MyMesh::Point* p) {
	int i = 0;
	MyMesh::VertexIter v_it, v_itend(prt_mesh->vertices_end());
	for (v_it = prt_mesh->vertices_begin(); v_it != v_itend; v_it++, i++) {
		if (*p == prt_mesh->point(*v_it)) {
			return i;
		}
	}
}

void Parameterize5::setTex(MyMesh* mesh_) {

	for (auto v_it = prt_mesh->vertices_begin(), v2_it = mesh_->vertices_begin(); v2_it != mesh_->vertices_end(); v_it++, v2_it++) {
		OpenMesh::Vec2f texcoord(prt_mesh->point(*v_it).data()[0], prt_mesh->point(*v_it).data()[1]);
		mesh_->set_texcoord2D(*v2_it, texcoord);
	}
}

bool Parameterize5::isTriangle(MyMesh::Point A, MyMesh::Point B, MyMesh::Point C, MyMesh::Point P) {
	OpenMesh::Vec3f v0 = C - A;
	OpenMesh::Vec3f v1 = B - A;
	OpenMesh::Vec3f v2 = P - A;
	float dot00 = v0 | v0;
	float dot01 = v0 | v1;
	float dot02 = v0 | v2;
	float dot11 = v1 | v1;
	float dot12 = v1 | v2;

	float inverDeno = 1 / (dot00*dot11 - dot01*dot01);
	float u = (dot02*dot11 - dot12*dot01)*inverDeno;
	if (u < -0.0001 || u>1) {
		return false;
	}
	float v = (dot00*dot12 - dot01*dot02)*inverDeno;
	if (v < -0.0001 || v>1) {
		return false;
	}
	return u + v <= 1;
}
