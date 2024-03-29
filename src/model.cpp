#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>

#include "model.h"

Model::Model(const char *filename) : m_verts() , m_faces() {
	std::ifstream in;

	in.open(filename, std::ifstream::in);
	if (in.fail()) return;

	std::string line;
	while (!in.eof()) {
		std::getline(in, line);
		std::istringstream iss(line.c_str());
		char trash;

		if (!line.compare(0, 2, "v ")) {
			iss >> trash;
			glm::vec3 v;
			for (int i = 0; i < 3; i++) iss >> v[i];
			m_verts.push_back(v);
		} else if (!line.compare(0, 3, "vt ")) {
			iss >> trash;
			iss >> trash;
			glm::vec3 uv;
			for (int i = 0; i < 3; i++) iss >> uv[i];
			m_uvs.push_back(uv);
		} else if (!line.compare(0, 3, "vn ")) {
			iss >> trash;
			iss >> trash;
			glm::vec3 norm;
			for (int i = 0; i < 3; i++) iss >> norm[i];
			m_norms.push_back(norm);
		} else if (!line.compare(0, 2, "f ")) {
			std::vector<int> f, uv;
			int idx, uvidx, normidx;
			iss >> trash;
			while (iss >> idx >> trash >> uvidx >> trash >> normidx) {
				idx--;
				uvidx--;
				normidx--;
				f.push_back(idx);
				f.push_back(uvidx);
				f.push_back(normidx);
			}
			m_faces.push_back(f);
		}
	}

	std::cerr << "# v# " << m_verts.size() << " f# " << m_faces.size() << std::endl;
}

Model::~Model() {
}

int Model::nverts() const {
	return m_verts.size();
}

int Model::nuvs() const {
	return m_uvs.size();
}

int Model::nnorms() const {
	return m_norms.size();
}

int Model::nfaces() const {
	return m_faces.size();
}

glm::vec3 Model::vert(int idx) const {
	return m_verts[idx];
}

glm::vec3 Model::uv(int idx) const {
	return m_uvs[idx];
}

glm::vec3 Model::norm(int idx) const {
	return m_norms[idx];
}

std::vector<int> Model::face(int idx) const {
	return m_faces[idx];
}

void Model::set_diffuse_texture(TGAImage &texture) {
	m_diffuse_texture = texture;
	m_has_diffuse_texture = true;
}

void Model::set_normal_texture(TGAImage &texture) {
	m_normal_texture = texture;
	m_has_normal_texture = true;
}

void Model::set_specular_texture(TGAImage &texture) {
	m_specular_texture = texture;
	m_has_specular_texture = true;
}

TGAColor Model::diffuse_color(glm::vec2& uv) {
	return m_diffuse_texture.get(
		(int)(uv.x*m_diffuse_texture.get_width()),
		(int)(uv.y*m_diffuse_texture.get_height())
	);
}

glm::vec4 Model::normal_from_texture(glm::vec2& uv) {
	TGAColor norm = m_normal_texture.get(
		(int)(uv.x*m_normal_texture.get_width()),
		(int)(uv.y*m_normal_texture.get_height())
	);
	glm::vec4 res(1.f);
	for (int i = 0; i < 3; i++) {
		res[2-i] = (float)norm[i]/255.f*2.f - 1.f;
	}
	return res;
}

float Model::specular(glm::vec2& uv) {
	TGAColor specular = m_specular_texture.get(
		(int)(uv.x*m_normal_texture.get_width()),
		(int)(uv.y*m_normal_texture.get_height())
	);
	return specular[0];
}
