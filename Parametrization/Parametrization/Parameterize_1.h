#pragma once
#ifndef PARAMETERIZE1_H_
#define PARAMETERIZE1_H_
#include "Parameterize.h"

class Parameterize_1:public Parameterize
{
public:
	Parameterize_1(MyMesh *mesh_);
	~Parameterize_1();
	virtual void setCoefficient(int i, MyMesh::VertexIter v_it, std::vector<T> *coefficients);
};

#endif // !PARAMETERIZE1_H_