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
		} else if (!line.compare(0, 2, "f ")) {
			std::vector<int> f;
			int itrash, idx;
			iss >> trash;
			while (iss >> idx >> trash >> itrash >> trash >> itrash) {
				idx --;
				f.push_back(idx);
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

int Model::nfaces() const {
	return m_faces.size();
}

Vec3f Model::vert(int idx) const {
	return m_verts[idx];
}

std::vector<int> Model::face(int idx) const {
	return m_faces[idx];
}
