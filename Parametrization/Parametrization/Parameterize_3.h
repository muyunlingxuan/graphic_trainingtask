#pragma once
#ifndef PARAMETERIZE3_H_
#define PARAMETERIZE3_H_
#include "Parameterize.h"
class Parameterize_3 :public Parameterize
{
public:
	Parameterize_3(MyMesh* mesh_);
	~Parameterize_3();
	virtual void setCoefficient(int i, MyMesh::VertexIter v_it, std::vector<T> *coefficients);

private:
	void mapToPlane(MyMesh::VertexIter v_it, std::vector<MyMesh::Point>* vec_point_new);
	void calculateWpi(std::vector<MyMesh::Point> vec_point_new, std::vector<float>* wp);
	double getAngle(MyMesh::Point p, MyMesh::Point q1, MyMesh::Point q2);
	bool isTriangle(MyMesh::Point A, MyMesh::Point B, MyMesh::Point C, MyMesh::Point P);

private:
	MyMesh origin_mesh_;
};

#endif // !PARAMETERIZE3_H_