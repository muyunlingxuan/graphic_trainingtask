#include "MiniSurf.h"
#include <iostream>

MiniSurf::MiniSurf(MyMesh *pMesh)		//��ڵ�
{
	ptr_mesh_ = pMesh;
	Init();
	do
	{
		cogs.clear();
		MakeNewPositions();
		WriteNewPositions();
	} while (currentOffset>max_offset_);
	std::cout << "��������=" << step_factor_ << ",���������=" << max_offset_ << "." << "��С�����." << std::endl;
}

MiniSurf::~MiniSurf(void)
{
}

void MiniSurf::Init(void)								//��С������������
{
	step_factor_ = 0.1;
	max_offset_ = 0.00001;
	currentOffset = 0;
	cogs.reserve(ptr_mesh_->n_vertices());
}

void MiniSurf::MakeNewPositions()
{
	MyMesh::VertexIter v_it, v_itend(ptr_mesh_->vertices_end());
	MyMesh::VertexVertexIter vv_it;
	MyMesh::Point cog;
	MyMesh::Scalar valence;

	for (v_it = ptr_mesh_->vertices_begin(); v_it != v_itend; ++v_it) {
		cog.vectorize(0.0f);
		valence = 0.0;
		//position_Pv_it = sum(Pvv_it)/count(Pvv_it)
		for (vv_it = ptr_mesh_->vv_iter(*v_it); vv_it.is_valid(); ++vv_it) {
			cog += ptr_mesh_->point(*vv_it);
			valence++;
		}
		cogs.push_back(cog / valence);
	}							
}

void MiniSurf::WriteNewPositions(void)					//д��һ�����ж����������
{
	currentOffset = 0.0;
	MyMesh::VertexIter v_it, v_itend(ptr_mesh_->vertices_end());
	for (v_it = ptr_mesh_->vertices_begin(), cog_it = cogs.begin();
		v_it != v_itend; ++v_it, ++cog_it) {
		if (!ptr_mesh_->is_boundary(*v_it)) {
			MyMesh::Point newDir = *cog_it - ptr_mesh_->point(*v_it);
			ptr_mesh_->set_point(*v_it, *cog_it);
			float offset = newDir.data()[0] * newDir.data()[0] +
							newDir.data()[1] * newDir.data()[1] +
							newDir.data()[2] * newDir.data()[2];
			if (offset > currentOffset*currentOffset) {
				currentOffset = sqrt(offset*offset);
			}
		}
	}
}