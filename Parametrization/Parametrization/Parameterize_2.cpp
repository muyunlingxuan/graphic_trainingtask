#include "Parameterize_2.h"



Parameterize_2::Parameterize_2(MyMesh *mesh) :Parameterize(mesh)
{
	Param = this;
	doParameterize();
}


Parameterize_2::~Parameterize_2()
{
}

/*
wij = 1/|pi-pj|
aij = -wij
aii = sum(wij)
b = wij j-±ß½çµã
*/
void Parameterize_2::setCoefficient(int i, MyMesh::VertexIter v_it, std::vector<T>* coefficients) {
	MyMesh::VertexVertexIter vv_it;
	float wij = 0;
	float wi = 0;
	for (vv_it = ptr_mesh_->vv_iter(*v_it); vv_it.is_valid(); vv_it++) {
		MyMesh::Point pi = ptr_mesh_->point(*v_it);
		MyMesh::Point pj = ptr_mesh_->point(*vv_it);
		MyMesh::Point pij = pi - pj;
		wij = pij.data()[0] * pij.data()[0] + pij.data()[1] * pij.data()[1] + pij.data()[2] * pij.data()[2];
		wij = sqrt(wij);
		wi += wij;
		if (ptr_mesh_->is_boundary(*vv_it)) {
			B_matrix_(i, 0) += wij*ptr_mesh_->point(*vv_it).data()[0];
			B_matrix_(i, 1) += wij*ptr_mesh_->point(*vv_it).data()[1];
			B_matrix_(i, 2) += wij*ptr_mesh_->point(*vv_it).data()[2];
		}
		else {
			int j = getIndex(&ptr_mesh_->point(*vv_it));
			coefficients->push_back(T(i, j, -wij));
		}
	}
	coefficients->push_back(T(i, i, wi));
}
