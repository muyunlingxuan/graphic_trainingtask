#include "MiniSurf.h"

MiniSurf::MiniSurf(Mesh3D *pMesh)		//��ڵ�
{
	step_factor_ = 0.1;
	max_offset_ = 0.0001;
	ptr_mesh_ = pMesh;
	Init();
}

MiniSurf::~MiniSurf(void)
{
}

void MiniSurf::Init(void)								//��С������������
{
	if (!ptr_mesh_->isValid())
	{
		std::cout << "Mesh is not valid!" << std::endl;
		return;
	}
	ptr_vertlist_ = ptr_mesh_->get_vertex_list();			//��ȡ�����б�
	new_positions_.resize(ptr_vertlist_->size());			//����������Ļ���ռ�


	float currentOffset = 0;
	do
	{
		currentOffset = MakeNewPositions();				//����һ�����ж����������
		WriteNewPositions();
	} while (currentOffset>max_offset_);					//�ﲻ������������ִ��һ�μ���


	std::cout << "��������=" << step_factor_ << ",���������=" << max_offset_ << "." << "��С�����." << std::endl;
	//���ˣ���С�����
}

float MiniSurf::MakeNewPositions()
{
	float curOffset = 0.0;
	HE_vert *pVert;
	int i, j;

	for (i = 0; i<ptr_vertlist_->size(); i++)
	{
		pVert = ptr_vertlist_->at(i);
		if (pVert->isOnBoundary())						//������߽��
			continue;

		int degree = pVert->degree();
		point newPosition(0.0, 0.0, 0.0);
		point newDir;

		for (j = 0; j<degree; j++)
		{
			newPosition += ptr_mesh_->get_vertex(pVert->neighborIdx[j])->position();
		}
		newPosition /= (float)degree;					//���������ڽڵ�ļ�������
		newDir = newPosition - pVert->position();
		new_positions_[i] = pVert->position() + step_factor_*newDir;	//ƫ������Ҫ����ϵ����0.1�����������
		if (newDir*newDir > curOffset*curOffset)
			curOffset = sqrt(newDir*newDir);			//�������нڵ㵽Ŀ��ڵ��������
	}

	return curOffset;									//�������нڵ㵽����Ŀ���ľ�������ֵ
}

void MiniSurf::WriteNewPositions(void)					//д��һ�����ж����������
{
	for (int i = 0; i<ptr_vertlist_->size(); i++)
		if (!ptr_vertlist_->at(i)->isOnBoundary())		//������߽��
			ptr_vertlist_->at(i)->set_position(new_positions_[i]);
}