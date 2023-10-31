#include <cmath>
#include <limits>
#include <cstdlib>
#include <iostream>

#include "our_gl.h"

glm::mat4 view(0.f);
glm::mat4 viewport(0.f);
glm::mat4 projection(0.f);

Shader::~Shader() {}

void create_viewport(int w, int h) {
    int depth = 255;
    viewport[0][0] = w/2.f;
    viewport[1][1] = h/2.f;
    viewport[2][2] = depth/2.f;

    viewport[3][0] = w/2.f + 0.5f;
    viewport[3][1] = h/2.f + 0.5f;
    viewport[3][2] = depth/2.f + 0.5f;
    viewport[3][3] = 1.f;
}

void create_projection(float coeff) {
	projection = glm::mat4(1.0f);
	// projection[3][2] = coeff;
	projection[2][3] = -1.f/200.f;
}

void look_at(glm::vec3 eye, glm::vec3 center, glm::vec3 up) {
    glm::vec3 z = glm::normalize(eye - center);
    glm::vec3 x = glm::normalize(glm::cross(up, z));
    glm::vec3 y = glm::normalize(glm::cross(z, x));

    view = glm::mat4(1.0f);
    for (int i = 0; i < 3; i++) {
        view[i][0] = x[i];
        view[i][1] = y[i];
        view[i][2] = z[i];
    }
    view[3][0] = -glm::dot(x, eye);
    view[3][1] = -glm::dot(y, eye);
    view[3][2] = -glm::dot(z, eye);
}

glm::vec3 barycentric_coordinates(glm::vec3 v[3], glm::vec3 P) {
    glm::vec3 w = glm::cross(
        glm::vec3(v[2].x - v[0].x, v[1].x - v[0].x, v[0].x - P.x), 
        glm::vec3(v[2].y - v[0].y, v[1].y - v[0].y, v[0].y - P.y)
    );
    if (std::abs(w.z) < 1.f) return glm::vec3(-1.f, 1.f, 1.f);
    return glm::vec3(1.f - (w.x + w.y)/w.z, w.y/w.z, w.x/w.z);
}

void triangle(glm::vec4 *vertices, Shader &shader, TGAImage &image, TGAImage &zbuffer) {
    glm::vec2 bboxmin(std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
    glm::vec2 bboxmax(std::numeric_limits<float>::min(), std::numeric_limits<float>::min());
    glm::vec2 clamp(image.get_width()-1, image.get_height()-1);
	glm::vec3 v3[3];
    for (int i=0; i<3; i++) {
        bboxmin.x = std::max(0.f,     std::min(bboxmin.x, vertices[i].x));
        bboxmin.y = std::max(0.f,     std::min(bboxmin.y, vertices[i].y));

        bboxmax.x = std::min(clamp.x, std::max(bboxmax.x, vertices[i].x));
        bboxmax.y = std::min(clamp.y, std::max(bboxmax.y, vertices[i].y));

		// v3[i] = glm::vec3(vertices[i]);
		v3[i] = glm::vec3(vertices[i]/vertices[i].w);
		// v3[i] = glm::vec3((int)v3[i].x, (int)v3[i].y, (int)v3[i].z);
    }

    glm::vec3 P;
	TGAColor color;
    for (P.x = (int)bboxmin.x; P.x <= bboxmax.x; P.x++) {
        for (P.y = (int)bboxmin.y; P.y <= bboxmax.y; P.y++) {
            glm::vec3 bc  = barycentric_coordinates(v3, P);
			float z = vertices[0].z*bc.x + vertices[1].z*bc.y + vertices[2].z*bc.z;
			float w = vertices[0].w*bc.x + vertices[1].w*bc.y + vertices[2].w*bc.z;
			int frag_depth = std::max(0, std::min(255, int(z/w + 0.5)));
			// std::cout << zbuffer.get(P.x, P.y)[0] << "\n";
			if (bc.x < 0.f || bc.y < 0.f || bc.z < 0.f || zbuffer.get(P.x, P.y)[0] > frag_depth) continue;
			bool discard = shader.fragment(bc, color);
			if (!discard) {
				std::cout << (int)P.x << ", " << (int)P.y << "\n";
				zbuffer.set(P.x, P.y, TGAColor(frag_depth, frag_depth, frag_depth, 255));
				image.set(P.x, P.y, color);
			}
            // P.z = 0.f;
            // P.z += vertices[0].z*bc.x;
            // P.z += vertices[1].z*bc.y;
            // P.z += vertices[2].z*bc.z;
            // if (zbuffer[int(P.x+P.y*width)] < P.z) {
            //     zbuffer[int(P.x+P.y*width)] = P.z;
            //     image.set(P.x, P.y, color);
            // }
        }
    }
}
