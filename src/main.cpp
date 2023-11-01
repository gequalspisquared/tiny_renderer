#include <vector>
#include <iostream>

#include "glm/geometric.hpp"
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
	glm::mat3x2 varying_uv;
	glm::mat4 uniform_M;
	glm::mat4 uniform_MIT;

	virtual glm::vec4 vertex(int iface, int nthvert) {
		std::vector<int> face = model->face(iface);

		glm::vec3 norm = model->norm(face[nthvert + 2]);
		varying_intensity[nthvert/3] = std::max(0.f, glm::dot(norm, light_dir));

		glm::vec3 curr_uv = model->uv(face[nthvert + 1]);
		varying_uv[nthvert/3] = {curr_uv.x, curr_uv.y};

		uniform_M = projection*model_view;
		uniform_MIT = glm::inverse(glm::transpose(uniform_M));

		glm::vec3 v = model->vert(face[nthvert]);
		glm::vec4 gl_Vertex(v.x, v.y, v.z, 1.f);
		gl_Vertex = viewport*projection*model_view*gl_Vertex;
		return gl_Vertex;
	}

	virtual bool fragment(glm::vec3 bc, TGAColor &color) {
		glm::vec2 uv = (varying_uv * bc);
		glm::vec3 normal = glm::normalize(glm::vec3(uniform_MIT*model->normal_from_texture(uv)));
		glm::vec4 l = glm::vec4(light_dir.x, light_dir.y, light_dir.z, 0.f);
		glm::vec3 light  = glm::normalize(glm::vec3(uniform_M*l));
		glm::vec3 r = glm::normalize(normal*(glm::dot(normal, light)*2.f) - light);
		float spec = pow(std::max(r.z, 0.0f), model->specular(uv));
		float intensity = std::max(0.f, glm::dot(normal, light));
		if (intensity < 0.f) {
			return true;
		}
		color = model->diffuse_color(uv);
		for (int i = 0; i < 3; i++) {
			color[i] = std::min<float>(5 + color[i]*(intensity + 0.6*spec), 255);
		}

		return false;
	}
};

int main(int argc, char** argv) {
	if (argc == 2) {
		model = new Model(argv[1]);
	} else {
		model = new Model("african_head.obj");
	}
	TGAImage texture;
	if (!texture.read_tga_file("african_head_diffuse.tga")) {
		std::cout << "Failed to load diffuse texture\n";
		delete model;
		return 1;
	}
	texture.flip_vertically();
	model->set_diffuse_texture(texture);

	TGAImage normal_map;
	if (!normal_map.read_tga_file("african_head_nm.tga")) {
		std::cout << "Failed to load diffuse texture\n";
		delete model;
		return 1;
	}
	normal_map.flip_vertically();
	model->set_normal_texture(normal_map);

	TGAImage specular;
	if (!specular.read_tga_file("african_head_nm.tga")) {
		std::cout << "Failed to load diffuse texture\n";
		delete model;
		return 1;
	}
	specular.flip_vertically();
	model->set_specular_texture(normal_map);

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
