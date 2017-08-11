#pragma once

#include "HE_mesh\Mesh3D.h"

class MiniSurf
{
	float step_factor_;						
	float max_offset_;						
	Mesh3D *ptr_mesh_;
	std::vector<HE_vert* > *ptr_vertlist_;	
	std::vector<point> new_positions_;		

public:
	MiniSurf(Mesh3D *mesh);
	~MiniSurf(void);

private:
	void Init(void);						
	float MakeNewPositions(void);			
	void WriteNewPositions(void);			
};

