#ifndef MINISURF_H_
#define MINISURF_H_


#include  <OpenMesh\Core\Mesh\TriMesh_ArrayKernelT.hh>

using namespace OpenMesh;
typedef TriMesh_ArrayKernelT<>  MyMesh;

class MiniSurf
{
	float step_factor_;
	float max_offset_;
	float currentOffset;
	MyMesh *ptr_mesh_;
	std::vector<MyMesh::Point> cogs;
	std::vector<MyMesh::Point>::iterator cog_it;

public:
	MiniSurf(MyMesh *mesh);
	~MiniSurf(void);

private:
	void Init(void);
	void MakeNewPositions(void);
	void WriteNewPositions(void);
};
#endif // !MINISURF_H_
