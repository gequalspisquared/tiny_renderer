#ifndef MODEL_H
#define MODEL_H

#include <vector>

#include "geometry.h"

class Model {
public:
	Model(const char* filename);
	~Model();

	int nverts() const;
	int nuvs() const;
	int nnorms() const;
	int nfaces() const;
	Vec3f vert(int idx) const;
	Vec3f uv(int idx) const;
	Vec3f norm(int idx) const;
	std::vector<int> face(int idx) const;

private:
	std::vector<Vec3f> m_verts;
	std::vector<Vec3f> m_uvs;
	std::vector<Vec3f> m_norms;
	std::vector<std::vector<int>> m_faces;
};

#endif
