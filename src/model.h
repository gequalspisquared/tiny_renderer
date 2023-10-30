#ifndef MODEL_H
#define MODEL_H

#include <vector>

// #include "geometry.h"
#include "glm/glm.hpp"

class Model {
public:
	Model(const char* filename);
	~Model();

	int nverts() const;
	int nuvs() const;
	int nnorms() const;
	int nfaces() const;
	glm::vec3 vert(int idx) const;
	glm::vec3 uv(int idx) const;
	glm::vec3 norm(int idx) const;
	std::vector<int> face(int idx) const;

private:
	std::vector<glm::vec3> m_verts;
	std::vector<glm::vec3> m_uvs;
	std::vector<glm::vec3> m_norms;
	std::vector<std::vector<int>> m_faces;
};

#endif
