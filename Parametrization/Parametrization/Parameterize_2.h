#pragma once
#ifndef PARAMETERIZE2_H_
#define PARAMETERIZE2_H_
#include "Parameterize.h"

class Parameterize_2 :public Parameterize
{
public:
	Parameterize_2(MyMesh *mesh_);
	~Parameterize_2();
	virtual void setCoefficient(int i, MyMesh::VertexIter v_it, std::vector<T> *coefficients);
};

#endif // !PARAMETERIZE2_H_