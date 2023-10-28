#ifndef MODEL_H
#define MODEL_H

#include <vector>

#include "geometry.h"

class Model {
public:
	Model(const char* filename);
	~Model();

	int nverts() const;
	int nfaces() const;
	Vec3f vert(int idx) const;
	std::vector<int> face(int idx) const;

private:
	std::vector<Vec3f> m_verts;
	std::vector<std::vector<int>> m_faces;
};

#endif
