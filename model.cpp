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
			Vec3f v;
			for (int i = 0; i < 3; i++) iss >> v.raw[i];
			m_verts.push_back(v);
		} else if (!line.compare(0, 3, "vt ")) {
			iss >> trash;
			iss >> trash;
			Vec3f uv;
			for (int i = 0; i < 3; i++) iss >> uv.raw[i];
			m_uvs.push_back(uv);
		} else if (!line.compare(0, 3, "vn ")) {
			iss >> trash;
			iss >> trash;
			Vec3f norm;
			for (int i = 0; i < 3; i++) iss >> norm.raw[i];
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

Vec3f Model::vert(int idx) const {
	return m_verts[idx];
}

Vec3f Model::uv(int idx) const {
	return m_uvs[idx];
}

Vec3f Model::norm(int idx) const {
	return m_norms[idx];
}

std::vector<int> Model::face(int idx) const {
	return m_faces[idx];
}
