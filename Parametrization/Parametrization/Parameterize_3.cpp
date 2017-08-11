#include <math.h>
#include "Parameterize_3.h"
#include <iostream>
Parameterize_3::Parameterize_3(MyMesh *mesh) :Parameterize(mesh)
{
	Param = this;
	origin_mesh_ = *mesh;
	doParameterize();
}

Parameterize_3::~Parameterize_3()
{
}

/*
uj,1 = area(pi,p2,p3)/area(pj,p2,p3)
wij = -sum(uj,1+uj,2...)
wii = degree_i
*/
void Parameterize_3::setCoefficient(int index_p, MyMesh::VertexIter v_it, std::vector<T>* coefficients) {
	std::vector<MyMesh::Point> vec_point_new;//映射到平面的点坐标
	
	//将p及周围点映射到平面
	mapToPlane(v_it, &vec_point_new);
	//计算wpi参数
	std::vector<float> wp(vec_point_new.size());
	calculateWpi(vec_point_new, &wp);
	//设置A_matrix_ B_matrix_矩阵系数
	int degree = wp.size();
	bool flag = false;
	MyMesh::VertexVertexIter vv_it = ptr_mesh_->vv_iter(*v_it);
	int i = 0;
	int m = ptr_mesh_->n_vertices();
	Eigen::MatrixXd B_matrix_2 = Eigen::MatrixXd::Zero(m, 3);

	do {
		float wpi = wp.at(i);
		if (ptr_mesh_->is_boundary(*vv_it)) {
			B_matrix_(index_p, 0) += wpi*(ptr_mesh_->point(*vv_it).data()[0]);
			B_matrix_(index_p, 1) += wpi*(ptr_mesh_->point(*vv_it).data()[1]);
			B_matrix_(index_p, 2) += wpi*(ptr_mesh_->point(*vv_it).data()[2]);
		}
		else {
			int index_i = getIndex(&ptr_mesh_->point(*vv_it));
			coefficients->push_back(T(index_p, index_i, -wpi));
		}
		vv_it++; i++;
	} while (vv_it.is_valid());

	coefficients->push_back(T(index_p, index_p, degree));
}


void Parameterize_3::mapToPlane(MyMesh::VertexIter v_it, std::vector<MyMesh::Point> *vec_point_new) {
	MyMesh::Point p = ptr_mesh_->point(*v_it);
	double angle_sum = 0;
	//遍历p点的周围顶点计算角度和
	MyMesh::VertexVertexIter vv_it_begin = origin_mesh_.vv_iter(*v_it);
	MyMesh::VertexVertexIter vv_it_q1 = vv_it_begin, vv_it_q2 = vv_it_begin;
	vv_it_q2++;

	do {
		MyMesh::Point q1 = origin_mesh_.point(*vv_it_q1);
		MyMesh::Point q2 = !vv_it_q2.is_valid() ? origin_mesh_.point(*vv_it_begin) : origin_mesh_.point(*vv_it_q2);
		angle_sum += getAngle(p, q1, q2);
		vv_it_q1++; vv_it_q2++;

	} while (vv_it_q1.is_valid());

	float length = (p - origin_mesh_.point(*vv_it_begin)).norm();
	vec_point_new->push_back(MyMesh::Point(length, 0, 0));

	double angle = 0;
	double angle_new = 0;

	vv_it_q1 = vv_it_begin, vv_it_q2 = vv_it_begin;
	vv_it_q2++;

	do {
		MyMesh::Point q1 = origin_mesh_.point(*vv_it_q1);
		MyMesh::Point q2 = origin_mesh_.point(*vv_it_q2);

		angle += getAngle(p, q1, q2);
		angle_new = angle * 2 * M_PI / angle_sum;
		length = (p - q2).norm();

		vec_point_new->push_back(MyMesh::Point(length*cos(angle_new), length*sin(angle_new), 0));

		vv_it_q1++; vv_it_q2++;

	} while (vv_it_q2.is_valid());
}

void Parameterize_3::calculateWpi(std::vector<MyMesh::Point> vec_point_new, std::vector<float>* wp) {
	MyMesh::Point p = MyMesh::Point(0.0f);
	double angle = 0;
	for (int i = 0; i < vec_point_new.size(); i++) {
		angle = 0;
		int j = (i + 1) % vec_point_new.size();
		while (j != i) {
			int j_left = j == 0 ? vec_point_new.size() - 1 : j - 1;
			int j_right = (j + 1) % vec_point_new.size();
			angle += getAngle(p, vec_point_new.at(j_left), vec_point_new.at(j));
			if (angle >= M_PI) {

				//if (isTriangle(vec_point_new.at(i), vec_point_new.at(j), vec_point_new.at(j_left), p)) {
					OpenMesh::Vec3f  vpi = vec_point_new.at(i) - p;
					OpenMesh::Vec3f  vpj = vec_point_new.at(j) - p;
					OpenMesh::Vec3f  vpj_left = vec_point_new.at(j_left) - p;

					OpenMesh::Vec3f  vi1 = vec_point_new.at(j) - vec_point_new.at(i);
					OpenMesh::Vec3f  vi2 = vec_point_new.at(j_left) - vec_point_new.at(i);
				
					wp->at(i) += (vpj%vpj_left).norm() / (vi1%vi2).norm();
					wp->at(j_left) += (vpj%vpi).norm() / (vi1%vi2).norm();
					wp->at(j) += (vpj_left%vpi).norm() / (vi1%vi2).norm();
					break;
				/*}
				else if (isTriangle(vec_point_new.at(i), vec_point_new.at(j), vec_point_new.at(j_right), p)) {
					OpenMesh::Vec3f  vpi = vec_point_new.at(i) - p;
					OpenMesh::Vec3f vpj = vec_point_new.at(j) - p;
					OpenMesh::Vec3f  vpj_right = vec_point_new.at(j_right) - p;
					OpenMesh::Vec3f  vi1 = vec_point_new.at(j) - vec_point_new.at(i);
					OpenMesh::Vec3f  vi2 = vec_point_new.at(j_right) - vec_point_new.at(i);

					wp->at(i) += (vpj%vpj_right).norm() / (vi1%vi2).norm();
					wp->at(j_right) += (vpj%vpi).norm() / (vi1%vi2).norm();
					wp->at(j) += (vpj_right%vpi).norm() / (vi1%vi2).norm();
					break;
				}*/

			}
			j = (j + 1) % vec_point_new.size();

		}
	}
}



double Parameterize_3::getAngle(MyMesh::Point p, MyMesh::Point q1, MyMesh::Point q2) {
	MyMesh::Normal v1 = q1 - p;
	MyMesh::Normal v2 = q2 - p;
	double cospij = OpenMesh::dot(v1.normalize(), v2.normalize());
	return acos(cospij);
}

bool Parameterize_3::isTriangle(MyMesh::Point A, MyMesh::Point B, MyMesh::Point C, MyMesh::Point P) {
	MyMesh::Normal v0 = C - A;
	MyMesh::Normal v1 = B - A;
	MyMesh::Normal v2 = P - A;
	float dot00 = v0 | v0;
	float dot01 = v0 | v1;
	float dot02 = v0 | v2;
	float dot11 = v1 | v1;
	float dot12 = v1 | v2;

	float inverDeno = 1 / (dot00*dot11 - dot01*dot01);
	float u = (dot11*dot02 - dot01*dot12)*inverDeno;
	if (u < -0.0001 || u>1) {
		return false;
	}
	float v = (dot00*dot12 - dot01*dot02)*inverDeno;
	if (v < -0.0001 || v>1) {
		return false;
	}
	return u + v <= 1;
}

