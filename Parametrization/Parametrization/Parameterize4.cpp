#include <math.h>
#include "Parameterize4.h"
#include <iostream>


Parameterize4::Parameterize4(MyMesh *mesh_)
{
	ori_mesh = *mesh_;
	prt_mesh = mesh_;
	int m = prt_mesh->n_vertices();
	cogs.reserve(m);
	A_matrix_.resize(m, m);
	B_matrix_ = Eigen::MatrixXd::Zero(m, 3);

	//¹Ì¶¨µã
	fixBoundary();
	MakeNewPositions();
	WriteNewPositions();
}


Parameterize4::~Parameterize4()
{
}

void Parameterize4::calculateXMatrix() {
	Eigen::SparseQR<Eigen::SparseMatrix<double>, Eigen::COLAMDOrdering<int>> QR;
	QR.compute(A_matrix_);
	X_matrix_ = QR.solve(B_matrix_);
}

void Parameterize4::mapToPlane(MyMesh::VertexIter v_ori_it, std::vector<OpenMesh::Vec2f>* vec_point_2D){
	MyMesh::VertexVertexIter q1_it(ori_mesh.vv_iter(*v_ori_it)), q1_init_it(ori_mesh.vv_iter(*v_ori_it)), q2_it(ori_mesh.vv_iter(*v_ori_it)), q_it = ori_mesh.vv_iter(*v_ori_it);
	double total_angle = 0.0;
	double angle = 0.0;
	std::vector<double> vec_angle, lengths;
	MyMesh::Point p(ori_mesh.point(*v_ori_it)), q1, q2;
	OpenMesh::Vec3f pq1,pq2;
	do {
		q2_it++;
		q1 = ori_mesh.point(*q1_it);
		q2 = !q2_it.is_valid() ? ori_mesh.point(*q1_init_it) : ori_mesh.point(*q2_it);
		
		pq1 = q1 - p;
		pq2 = q2 - p;
		lengths.push_back(pq1.norm());
		angle = std::acos(OpenMesh::dot(pq1.normalize(), pq2.normalize()));
		vec_angle.push_back(angle);
		total_angle = total_angle + angle;
		q1_it++;
	} while (q1_it.is_valid());

	for (int i = 0; i < vec_angle.size(); i++) {
		vec_angle[i] = vec_angle[i] * 2 * M_PI / total_angle;
	}
	q_it++;
	double accu_angle = vec_angle[0];
	vec_point_2D->push_back(OpenMesh::Vec2f(lengths[0], 0));
	for (int i = 1; q_it.is_valid(); q_it++, i++) {
		OpenMesh::Vec2f position_q(lengths[i] * std::cos(accu_angle), lengths[i] * std::sin(accu_angle));
		accu_angle += vec_angle[i];
		vec_point_2D->push_back(position_q);
	}
	for (int i = 0; i < vec_point_2D->size(); i++) {
		std::cout << vec_point_2D->at(i)[0] << ","<<vec_point_2D->at(i)[1] << std::endl;
	}
}
double Parameterize4::crossproduct2D(OpenMesh::Vec2f v1, OpenMesh::Vec2f v2) {
	return v1[0] * v2[1] - v1[1] * v2[0];
}

void Parameterize4::calculateWpi(MyMesh::VertexIter v_prt_it, MyMesh::VertexIter v_ori_it, std::vector<OpenMesh::Vec2f>* vec_point_2D, std::vector<float>* wp){
	OpenMesh::Vec2f p_init, p, p1, p2, p3, pp1, pp2;
	p = OpenMesh::Vec2f(0., 0.);
	int valence = vec_point_2D->size();
	for (int i = 0; i < valence; i++) {
		p1 = vec_point_2D->at(i);
		int j = (i + 1) % valence;
		for (; j != i; j = (j + 1) % valence) {
			p2 = vec_point_2D->at(j);
			p3 = vec_point_2D->at((j + 1) % valence);
			double cross1 = crossproduct2D(p2 - p, p - p1), cross2 = crossproduct2D(p3 - p, p - p1);
			if (cross1*cross2 <= 0.000001) {	// hit
				break;
			}
		}
		double	areap1p2p3 = std::abs(crossproduct2D(p2 - p1, p3 - p1)),
			areapp2p3 = std::abs(crossproduct2D(p2 - p, p3 - p)),
			areap1pp3 = std::abs(crossproduct2D(p1 - p, p3 - p)),
			areap1p2p = std::abs(crossproduct2D(p1 - p, p2 - p));
		int ind1, ind2, ind3;
		wp->at(i) += areapp2p3 / areap1p2p3;
		wp->at(j) += areap1pp3 / areap1p2p3;

		wp->at((j + 1) % valence) += areap1p2p / areap1p2p3;
	}
}

void Parameterize4::setMatrixAb(int index_p, MyMesh::VertexIter v_prt_it, std::vector<float>* wp, std::vector<T>* coefficients) {
	int i = 0;
	int valence = wp->size();
	int m = prt_mesh->n_vertices();
	MyMesh::VertexVertexIter vv_it = prt_mesh->vv_iter(*v_prt_it);
	
	for(int i=0; vv_it.is_valid(); vv_it++,i++){
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

void Parameterize4::setCoefficient(int index_p, MyMesh::VertexIter v_prt_it, MyMesh::VertexIter v_ori_it, std::vector<T>* coefficients) {
	std::vector<OpenMesh::Vec2f> vec_point_2D;
	mapToPlane(v_ori_it,&vec_point_2D);
	std::vector<float> wp(vec_point_2D.size());
	calculateWpi(v_prt_it, v_ori_it,&vec_point_2D, &wp);
	setMatrixAb(index_p, v_prt_it,&wp,coefficients);
}

void Parameterize4::calculateAb() {
	std::vector<T> coefficients;
	MyMesh::VertexIter v_prt_it(prt_mesh->vertices_begin()), v_prt_itend(prt_mesh->vertices_end());
	MyMesh::VertexIter v_ori_it(ori_mesh.vertices_begin()), v_ori_itend(ori_mesh.vertices_end());
	
	int i = 0;
	for (; v_prt_it != v_prt_itend; v_prt_it++, v_ori_it++,i++) {
		if (!prt_mesh->is_boundary(*v_prt_it)) {
			setCoefficient(i, v_prt_it, v_ori_it, &coefficients);
		}
		else {
			coefficients.push_back(T(i, i, 1));

			B_matrix_(i, 0) = prt_mesh->point(*v_prt_it).data()[0];
			B_matrix_(i, 1) = prt_mesh->point(*v_prt_it).data()[1];
			B_matrix_(i, 2) = prt_mesh->point(*v_prt_it).data()[2];
		}
	}
	A_matrix_.setFromTriplets(coefficients.begin(), coefficients.end());

}



void Parameterize4::WriteNewPositions() {
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
void Parameterize4::MakeNewPositions() {
	//Ax=b
	calculateAb();
	calculateXMatrix();
}

void Parameterize4::fixBoundary() {
	unsigned int				N, B = 0;
	MyMesh::EdgeIter			e_it, e_beg(prt_mesh->edges_begin()), e_end(prt_mesh->edges_end()), last_boarder_edge;
	MyMesh::HalfedgeHandle		heh, heh_init;
	{
		for (e_it = e_beg; e_it != e_end; e_it++) {
			if (prt_mesh->is_boundary(*e_it)) {
				B++;
				last_boarder_edge = e_it;
			}
		}
		heh = prt_mesh->halfedge_handle(last_boarder_edge, 0);
		if (!prt_mesh->is_boundary(heh)) {
			heh = prt_mesh->opposite_halfedge_handle(heh);
		}
		if (!prt_mesh->is_boundary(heh)) {
			bool error = true;
		}
	}

	// map the boarder of the mesh onto the edges of a 2D unit square
	{
		double dist = 0, step = 4. / B; // dist: total distance, step: how wide is the gap between the points
		int counter = 0;
		heh_init = heh;
		while (true) {
			if (heh == heh_init) { counter++; }
			if (counter == 2) { break; }
			if (!prt_mesh->is_boundary(heh)) {
				bool error = true;
			}

			OpenMesh::Vec3f newPos;
			if (dist < 1) {
				newPos = OpenMesh::Vec3f(dist, 0., 0.);
			}
			else if (dist < 2) {
				newPos = OpenMesh::Vec3f(1., dist - 1, 0.);
			}
			else if (dist < 3) {
				newPos = OpenMesh::Vec3f(3 - dist, 1., 0.);
			}
			else if (dist < 4) {
				newPos = OpenMesh::Vec3f(0, 4 - dist, 0.);
			}
			OpenMesh::VertexHandle vh = prt_mesh->from_vertex_handle(heh);
			prt_mesh->set_point(vh, newPos);
			dist += step;
			heh = prt_mesh->next_halfedge_handle(heh);
		}
	}
}

int Parameterize4::getIndex(MyMesh::Point* p) {
	int i = 0;
	MyMesh::VertexIter v_it, v_itend(prt_mesh->vertices_end());
	for (v_it = prt_mesh->vertices_begin(); v_it != v_itend; v_it++, i++) {
		if (*p == prt_mesh->point(*v_it)) {
			return i;
		}
	}
}

void Parameterize4::setTex(MyMesh* mesh_) {

	for (auto v_it = prt_mesh->vertices_begin(), v2_it = mesh_->vertices_begin(); v2_it != mesh_->vertices_end(); v_it++, v2_it++) {
		OpenMesh::Vec2f texcoord(prt_mesh->point(*v_it).data()[0], prt_mesh->point(*v_it).data()[1]);
		mesh_->set_texcoord2D(*v2_it, texcoord);
	}
}
