#ifndef MODEL_H
#define MODEL_H

#include <vector>

#include "geometry.h"

class Model {
public:
	Model(const char* filename);
	~Model();

	int nverts();
	int nfaces();
	Vec3f vert(int idx);
	std::vector<int> face(int idx);

private:
	std::vector<Vec3f> m_verts;
	std::vector<std::vector<int>> m_faces;
};

#endif
