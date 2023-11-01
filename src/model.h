#ifndef MODEL_H
#define MODEL_H

#include <vector>

// #include "geometry.h"
#include "tgaimage.h"
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

	void set_diffuse_texture(TGAImage& texture);
	void set_normal_texture(TGAImage& texture);
	void set_specular_texture(TGAImage& texture);
	TGAColor diffuse_color(glm::vec2& uv);
	glm::vec4 normal_from_texture(glm::vec2& uv);
	float specular(glm::vec2& uv);

private:
	std::vector<glm::vec3> m_verts;
	std::vector<glm::vec3> m_uvs;
	std::vector<glm::vec3> m_norms;
	std::vector<std::vector<int>> m_faces;
	TGAImage m_diffuse_texture;
	TGAImage m_normal_texture;
	TGAImage m_specular_texture;
	bool m_has_diffuse_texture = false;
	bool m_has_normal_texture = false;
	bool m_has_specular_texture = false;
};

#endif
