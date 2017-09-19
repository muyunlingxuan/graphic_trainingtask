#include "poissonMeshEdit.h"
#include <Eigen\src\SVD\JacobiSVD.h>


poissonMeshEdit::poissonMeshEdit(Mesh3D *mesh)
{
	ptr_mesh_ = mesh;
	count_points_ = (ptr_mesh_->get_vertex_list())->size();
//	set_coefficientA();//计算拉普拉斯算子
//	set_laplace_coor();//计算拉普拉斯坐标
}

poissonMeshEdit::~poissonMeshEdit()
{
}

void poissonMeshEdit::set_boundary(std::vector<int> indexs_editable, std::vector<int> indexs_fixed)
{
	set_coefficientA();//计算拉普拉斯算子
	set_laplace_coor();//计算拉普拉斯坐标
	set_bc_editable(indexs_editable);
	set_bc_fixed(indexs_fixed);
	set_coefficient_bc();//添加边界条件
	set_coefficient();//最终求解方程左侧矩阵
}

void poissonMeshEdit::Deformation()
{
//	shortFloyd();
	set_div_w();//计算变换后的散度
	set_value_bc();//添加边界点值
	get_deformation_mesh();//求解变换后坐标并写回原网格
}

/*
set editable points
*/
void poissonMeshEdit::set_bc_editable(std::vector<int> indexs)
{
	bc_editable_.Clear();
	const std::vector<HE_vert*>& verts = *(ptr_mesh_->get_vertex_list());
	for (int i = 0; i < indexs.size(); i++)
	{
		bc_editable_.Insert_indexs(indexs[i]);
		bc_editable_.Insert_points(verts.at(indexs[i]));
	}
	Calculate_local_frames(bc_editable_);
}

void poissonMeshEdit::set_bc_edited(std::vector<int> indexs)
{
	bc_edited_.Clear();
	const std::vector<HE_vert*>& verts = *(ptr_mesh_->get_vertex_list());
	for (int i = 0; i < indexs.size(); i++)
	{
		bc_edited_.Insert_indexs(indexs[i]);
		bc_edited_.Insert_points(verts.at(indexs[i]));
	}
	Calculate_local_frames(bc_edited_);
}

/*
set fixed point
*/
void poissonMeshEdit::set_bc_fixed(std::vector<int> indexs)
{
	bc_fixed_.Clear();
	const std::vector<HE_vert*>& verts = *(ptr_mesh_->get_vertex_list());
	for (int i = 0; i < indexs.size(); i++)
	{
		bc_fixed_.Insert_indexs(indexs[i]);
		bc_fixed_.Insert_points(verts.at(indexs[i]));
	}
	Calculate_local_frames(bc_fixed_);
}


void poissonMeshEdit::set_rotate_matrix(Eigen::Matrix3f matrix, std::vector<int> bc_edited,Vec3f center)
{
	center_ = center;
	bc_editable_.set_rotate_matrix_(matrix);
}


/*
calculate local frame
*/
void poissonMeshEdit::Calculate_local_frames(boundaryCondition bc)
{
	int size = bc.indexs()->size();
	for (int i = 0; i < size; i++)
	{
		HE_vert *pVert = bc.points()->at(i);
		HE_vert *qVert = ptr_mesh_->get_vertex(pVert->neighborIdx[0]);
		Vec3f nz = pVert->normal();
		Vec3f pq = pVert->position() - qVert->position();
		Vec3f nx = pq - (pq*nz)*nz;
		nx.normalize();
		Vec3f ny = nz ^ nx;
		ny.normalize();
		Eigen::Matrix3f matrix;
		matrix << nx.at(0), ny.at(0), nz.at(0),
			nx.at(1), ny.at(1), nz.at(1),
			nx.at(2), ny.at(2), nz.at(2);
		bc.Insert_frames(matrix);
	}
}

//void poissonMeshEdit::get_coefficientA()
//{
//	//std::vector<T> coefficients;
//	if (ptr_mesh_ == NULL)
//		return;
//	if (ptr_mesh_->num_of_vertex_list() == 0)
//	{
//		return;
//	}
//	std::vector<HE_vert*> verts = *(ptr_mesh_->get_vertex_list());
//	int num = verts.size();
//	coefficient_A_.resize(num, num);
//	
//	for (int i = 0; i < num; i++)
//	{
//		HE_vert *pVert = verts.at(i);
//		int degree = pVert->degree();
//		double wi = degree;
//		//计算拉普拉斯参数
//		for (int j = 0; j < degree; j++)
//		{
//			HE_vert *qVert = ptr_mesh_->get_vertex(pVert->neighborIdx[j]);
//			
//			double wij = 1;
//			coefficients.push_back(T(i, qVert->id(), -wij));
//		}
//		coefficients.push_back(T(i, i, wi));
//	}
//	coefficient_A_.setFromTriplets(coefficients.begin(), coefficients.end());
//	LU.compute(coefficient_A_);
//
//}

/*
根据拓扑结构计算所有点的拉普拉斯坐标
均值
L(pi) = sum(1/d * (pi - pj))
余切
L(pi) =sum( 1/(2*areai) * (cot aj + cot bj)*(pi - pj))
*/
void poissonMeshEdit::set_coefficientA()
{
	std::vector<T> coefficients;
	if (ptr_mesh_ == NULL)
		return;
	if (ptr_mesh_->num_of_vertex_list() == 0)
	{
		return;
	}
	std::vector<HE_vert*> verts = *(ptr_mesh_->get_vertex_list());
	coefficient_A_.resize(count_points_, count_points_);
	for (int i = 0; i < count_points_; i++)
	{
		HE_vert *pVert = verts.at(i);
		int degree = pVert->degree();
		double wi = 0;
		//求pi点周围三角面片面积
		double area = 0;
		if (pVert->isOnBoundary())
		{
			coefficients.push_back(T(i, i, 1));
			continue;
		}
		for (int j = 0; j < degree; j++)
		{
			int index_qVert_right = (j + 1) % degree;

			HE_vert *qVert = ptr_mesh_->get_vertex(pVert->neighborIdx[j]);
			HE_vert *qVert_right = ptr_mesh_->get_vertex(pVert->neighborIdx[index_qVert_right]);

			Vec3f vec_l = pVert->position() - qVert->position();
			Vec3f vec_r = pVert->position() - qVert_right->position();
			area += (vec_l^vec_r).length() / 2;
		}
		//计算拉普拉斯参数
		for (int j = 0; j < degree; j++)
		{
			int index_qVert_left = (j - 1 + degree) % degree;
			int index_qVert_right = (j + 1) % degree;

			HE_vert *qVert = ptr_mesh_->get_vertex(pVert->neighborIdx[j]);
			HE_vert *qVert_left = ptr_mesh_->get_vertex(pVert->neighborIdx[index_qVert_left]);
			HE_vert *qVert_right = ptr_mesh_->get_vertex(pVert->neighborIdx[index_qVert_right]);

			double wij = area * ComputeLaplaceWeight(pVert, qVert, qVert_left, qVert_right);
			coefficients.push_back(T(i, qVert->id(), -wij));
			wi += wij;
		}
		coefficients.push_back(T(i, i, wi));
	}
	coefficient_A_.setFromTriplets(coefficients.begin(), coefficients.end());
	//LU.compute(coefficient_A_);
	
}

void poissonMeshEdit::set_laplace_coor()
{
	std::vector<HE_vert*> verts = *(ptr_mesh_->get_vertex_list());
	Eigen::MatrixXd positions = Eigen::MatrixXd::Zero(count_points_, 3);
	for (int i = 0; i < count_points_; i++)
	{
		Vec3f position = (verts.at(i))->position();
		positions(i, 0) = position.at(0);
		positions(i, 1) = position.at(1);
		positions(i, 2) = position.at(2);
	}
	laplace_coor_ = coefficient_A_*positions;
	value_right_ = laplace_coor_;
}

double poissonMeshEdit::ComputeLaplaceWeight(HE_vert *p, HE_vert *q, HE_vert *q_left, HE_vert *q_right)
{
	Vec3f l_v = p->position() - q_left->position();
	Vec3f l_m = q->position() - q_left->position();
	Vec3f r_v = p->position() - q_right->position();
	Vec3f r_m = q->position() - q_right->position();

	double cot1 = (l_v*l_m) / ((l_v^l_m).length());
	double cot2 = (r_v*r_m) / ((r_v^r_m).length());

	double result = 0.5*(cot1 + cot2);
	return result;
}

void poissonMeshEdit::set_coefficient()
{
	coefficient_.makeCompressed();
	/*std::cout << "==========coefficient_A_===============================" << std::endl;
	std::cout << coefficient_A_ << std::endl;
	std::cout << "==========coefficient_==============================="<< std::endl;
	std::cout << coefficient_ << std::endl;*/
	//svd = Eigen::JacobiSVD<Eigen::MatrixXd>(coefficient_, Eigen::ComputeThinU | Eigen::ComputeThinV);
	LU.compute(coefficient_);
}

/*
所有内部点 反向距离加权 求转换矩阵
*/
Eigen::Matrix4f poissonMeshEdit::Propagate_transform_matrix_to_pVert(HE_vert *pVert)
{
	double distance_to_bc_editable = get_min_distance(pVert,bc_editable_);
	if (distance_to_bc_editable < 0.00001)
		return bc_editable_.transform_matrix();

	double distance_to_bc_fixed = get_min_distance(pVert, bc_fixed_);
	if (distance_to_bc_fixed < 0.00001)
		return bc_fixed_.transform_matrix();

	double reverse_distance_to_bc_editable = 1 / distance_to_bc_editable;
	double reverse_distance_to_bc_fixed = 1 / distance_to_bc_fixed;

	Eigen::Matrix4f transform = (reverse_distance_to_bc_editable*bc_editable_.transform_matrix() +
								 reverse_distance_to_bc_fixed*bc_fixed_.transform_matrix()) /
								(reverse_distance_to_bc_editable + reverse_distance_to_bc_fixed);

	
	return transform;
}

double poissonMeshEdit::get_min_distance(HE_vert *pVert, boundaryCondition bc)
{
	double distance = DBL_MAX;
	Vec3f position_p = pVert->position();
	std::vector<HE_vert*> *points = bc.points();
	for (int i = 0; i < points->size(); i++)
	{
		Vec3f position_q = points->at(i)->position();
		double distance_pq = (position_p - position_q).length();
		distance = distance_pq < distance ? distance_pq : distance;
	}
	/*int pId = pVert->id();
	std::vector<int> *indexs = bc.indexs();
	for (int i = 0; i < indexs->size(); i++)
	{
		int qId = indexs->at(i);
		distance = distance > shortPath(pId, qId) ? shortPath(pId, qId) : distance;
	}*/
	return distance;
}

//图的Floyd的最短路径（即点到点的最后路径）
void poissonMeshEdit::shortFloyd()
{
	shortPath = Eigen::MatrixXd::Zero(count_points_, count_points_);
	shortPosition = Eigen::MatrixXd::Zero(count_points_, count_points_);

	for (int i = 0; i<count_points_; i++)
		for (int j = 0; j<count_points_; j++)
		{
			shortPosition(i,j) = j;
			if (ptr_mesh_->isNeighbors(ptr_mesh_->get_vertex(i), ptr_mesh_->get_vertex(j)))
			{
				shortPath(i, j) = 1;
			}
			else {
				shortPath(i, j) = INFINY;
			}
		}
	for (int k = 0; k<count_points_; k++)
		for (int i = 0; i<count_points_; i++)
			for (int j = 0; j<count_points_; j++)
			{
				if (shortPath(i, j)>shortPath(i, k) + shortPath(k,j))
				{
					shortPath(i, j) = shortPath(i, k) + shortPath(k, j);
					shortPosition(i, j) = shortPosition(i, k);
				}
			}

}


void poissonMeshEdit::set_div_w()
{
	std::vector<HE_vert*> verts = *(ptr_mesh_->get_vertex_list());
	for (int i = 0; i < count_points_; i++)
	{
		Eigen::Matrix4f transform = Propagate_transform_matrix_to_pVert(verts.at(i));
		/*Eigen::Vector4f laplace_coor_i;
		laplace_coor_i << value_right_(i, 0),
			value_right_(i, 1),
			value_right_(i, 2),
			1;
		
		Eigen::Vector4f laplace_coor_i_new = transform * laplace_coor_i;
		value_right_.row(i) << laplace_coor_i_new(0),
			laplace_coor_i_new(1),
			laplace_coor_i_new(2);*/

		Eigen::Matrix3f transform2 = transform.block(0, 0, 3, 3);
		Eigen::Vector3f laplace_coor_i, center;
		center << center_.at(0), center_.at(1), center_.at(2);
		laplace_coor_i << value_right_(i, 0),
			value_right_(i, 1),
			value_right_(i, 2);

		Eigen::Vector3f laplace_coor_i_new = transform2 * laplace_coor_i;// -transform2*center + center;
		value_right_.row(i) << laplace_coor_i_new(0),
			laplace_coor_i_new(1),
			laplace_coor_i_new(2);
	}
	//std::cout << "==================value_right_====after=edit=====================" << std::endl;
	//std::cout << value_right_ << std::endl;
}

void poissonMeshEdit::get_deformation_mesh()
{
	Eigen::MatrixXd result = LU.solve(value_right_);

	/*std::cout << "==================result==========================" << std::endl;
	std::cout << result << std::endl;*/

	std::vector<HE_vert*> *verts = ptr_mesh_->get_vertex_list();
	for (int i = 0; i < count_points_; i++)
	{
		Vec3f point = Vec3f(result(i,0), result(i, 1), result(i, 2));
		verts->at(i)->position() = point;
	}
	Vec3f position = (bc_editable_.points())->at(0)->position();
	std::cout << "==================position==========================" << std::endl;
	std::cout << position.at(0)<<","<<position.at(1)<<","<<position.at(2)<< std::endl;
}

void poissonMeshEdit::set_coefficient_bc() {
	/*int rows = bc_fixed_.points()->size();
	coefficient_bc_ = Eigen::MatrixXd::Zero(rows, count_points_);
	for (int i = 0; i < rows; i++) {
		coefficient_bc_(i, bc_fixed_.indexs()->at(i)) = 1;
	}
	std::cout << "==================coefficient_bc_====after======================" << std::endl;
	std::cout << coefficient_bc_ << std::endl;*/
	coefficient_ = coefficient_A_;
	int rows = bc_fixed_.points()->size();
	for (int i = 0; i < rows; i++)
	{
		int index = bc_fixed_.indexs()->at(i);
		HE_vert *vert = bc_fixed_.points()->at(i);
		int degree = vert->degree();
		for (int j = 0; j < degree; j++)
		{
			HE_vert *qVert = ptr_mesh_->get_vertex(vert->neighborIdx[j]);
			coefficient_.coeffRef(index, qVert->id()) = 0;
		}
		coefficient_.coeffRef(index, index) = 1;
	}

	rows = bc_editable_.points()->size();
	for (int i = 0; i < rows; i++)
	{
		int index = bc_editable_.indexs()->at(i);
		HE_vert *vert = bc_editable_.points()->at(i);
		int degree = vert->degree();
		for (int j = 0; j < degree; j++)
		{
			HE_vert *qVert = ptr_mesh_->get_vertex(vert->neighborIdx[j]);
			coefficient_.coeffRef(index, qVert->id()) = 0;
		}
		coefficient_.coeffRef(index, index) = 1;
	}
}

void poissonMeshEdit::set_value_bc() {
	/*int rows = bc_fixed_.points()->size();
	value_bc_ = Eigen::MatrixXd::Zero(count_points_, 3);
	for (int i = 0; i < rows; i++) {
		HE_vert *vert = bc_fixed_.points()->at(i);
		value_bc_.row(bc_fixed_.indexs()->at(i))<< vert->position().at(0), vert->position().at(1), vert->position().at(2);
	}
	std::cout << "==================value_bc_====after======================" << std::endl;
	std::cout << value_bc_ << std::endl;*/
	int rows = bc_fixed_.points()->size();
	for (int i = 0; i < rows; i++)
	{
		int index = bc_fixed_.indexs()->at(i);
		HE_vert *vert = bc_fixed_.points()->at(i);
		value_right_.row(index) << vert->position().at(0), vert->position().at(1), vert->position().at(2);
	}

	rows = bc_editable_.points()->size();
	for (int i = 0; i < rows; i++)
	{
		int index = bc_editable_.indexs()->at(i);
		HE_vert *vert = bc_editable_.points()->at(i);

		Eigen::Matrix4f transform = bc_editable_.transform_matrix();
		Eigen::Vector4f position_vert;
		position_vert << vert->position().at(0),
			vert->position().at(1),
			vert->position().at(2),
			1;

		Eigen::Vector4f position_vert_new = transform * position_vert;
		value_right_.row(index) << position_vert_new(0),
			position_vert_new(1),
			position_vert_new(2);
	}

	//std::cout << "==================laplace_coor_==========================" << std::endl;
	//std::cout << laplace_coor_ << std::endl;
	//std::cout << "==================value_right_==========================" << std::endl;
	//std::cout << value_right_ << std::endl;
}

