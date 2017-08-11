#include "MiniSurf.h"

MiniSurf::MiniSurf(Mesh3D *pMesh)		//入口点
{
	step_factor_ = 0.1;
	max_offset_ = 0.0001;
	ptr_mesh_ = pMesh;
	Init();
}

MiniSurf::~MiniSurf(void)
{
}

void MiniSurf::Init(void)								//极小化的主体流程
{
	if (!ptr_mesh_->isValid())
	{
		std::cout << "Mesh is not valid!" << std::endl;
		return;
	}
	ptr_vertlist_ = ptr_mesh_->get_vertex_list();			//读取顶点列表
	new_positions_.resize(ptr_vertlist_->size());			//设置新坐标的缓存空间


	float currentOffset = 0;
	do
	{
		currentOffset = MakeNewPositions();				//计算一次所有顶点的新坐标
		WriteNewPositions();
	} while (currentOffset>max_offset_);					//达不到容许误差就再执行一次计算


	std::cout << "步长因子=" << step_factor_ << ",最大误差距离=" << max_offset_ << "." << "极小化完成." << std::endl;
	//至此，极小化完成
}

float MiniSurf::MakeNewPositions()
{
	float curOffset = 0.0;
	HE_vert *pVert;
	int i, j;

	for (i = 0; i<ptr_vertlist_->size(); i++)
	{
		pVert = ptr_vertlist_->at(i);
		if (pVert->isOnBoundary())						//不处理边界点
			continue;

		int degree = pVert->degree();
		point newPosition(0.0, 0.0, 0.0);
		point newDir;

		for (j = 0; j<degree; j++)
		{
			newPosition += ptr_mesh_->get_vertex(pVert->neighborIdx[j])->position();
		}
		newPosition /= (float)degree;					//计算所有邻节点的几何重心
		newDir = newPosition - pVert->position();
		new_positions_[i] = pVert->position() + step_factor_*newDir;	//偏移向量要乘上系数，0.1，算出新坐标
		if (newDir*newDir > curOffset*curOffset)
			curOffset = sqrt(newDir*newDir);			//更新所有节点到目标节点的最大距离
	}

	return curOffset;									//返回所有节点到各自目标点的距离的最大值
}

void MiniSurf::WriteNewPositions(void)					//写入一次所有顶点的新坐标
{
	for (int i = 0; i<ptr_vertlist_->size(); i++)
		if (!ptr_vertlist_->at(i)->isOnBoundary())		//不处理边界点
			ptr_vertlist_->at(i)->set_position(new_positions_[i]);
}