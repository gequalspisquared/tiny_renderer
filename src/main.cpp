#include <vector>
#include <iostream>

#include "tgaimage.h"
#include "model.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "our_gl.h"

Model *model = nullptr;
const int width = 800;
const int height = 800;

glm::vec3 light_dir(1.f, 1.f, 1.f);
glm::vec3 eye(0.3f, 0.3f, 1.f);
glm::vec3 center(0.f, 0.f, 0.f);
glm::vec3 up(0.f, 1.f, 0.f);

struct GouraudShader : public Shader {
	glm::vec3 varying_intensity;

	virtual glm::vec4 vertex(int iface, int nthvert) {
		std::vector<int> face = model->face(iface);
		glm::vec3 norm = model->norm(face[nthvert + 2]);
		varying_intensity[nthvert/3] = std::max(0.f, glm::dot(norm, light_dir));
		glm::vec3 v = model->vert(face[nthvert]);
		glm::vec4 gl_Vertex(v.x, v.y, v.z, 1.f);
		gl_Vertex = viewport*projection*model_view*gl_Vertex;
		return gl_Vertex;
	}

	virtual bool fragment(glm::vec3 bc, TGAColor &color) {
		float intensity = glm::dot(varying_intensity, bc);
		if (intensity < 0.f) {
			return true;
		}
		color = TGAColor(255, 255, 255)*intensity;
		return false;
	}
};

int main(int argc, char** argv) {
	if (argc == 2) {
		model = new Model(argv[1]);
	} else {
		model = new Model("african_head.obj");
	}

	look_at(eye, center, up);
	create_projection(eye, center);
	create_viewport(width, height);
	light_dir = glm::normalize(light_dir);

	TGAImage image(width, height, TGAImage::RGB);
	TGAImage zbuffer(width, height, TGAImage::GRAYSCALE);

	GouraudShader shader;
	for (int iface = 0; iface < model->nfaces(); iface++) {
		glm::vec4 screen_coords[3];
		for (int j = 0; j < 3; j++) {
			screen_coords[j] = shader.vertex(iface, 3*j + 0);
		}

		triangle(screen_coords, shader, image, zbuffer);
	}

	image.flip_vertically();
	zbuffer.flip_vertically();
	image.write_tga_file("output.tga");
	zbuffer.write_tga_file("zbuffer.tga");

	delete model;
	return 0;
}
