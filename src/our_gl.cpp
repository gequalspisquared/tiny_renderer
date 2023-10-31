#include "our_gl.h"

Shader::~Shader() {}

glm::mat4 model_view(1.f);
glm::mat4 projection(0.f);
glm::mat4 viewport(0.f);

void look_at(glm::vec3 eye, glm::vec3 center, glm::vec3 up) {
    glm::vec3 z = glm::normalize(eye - center);
    glm::vec3 x = glm::normalize(glm::cross(up, z));
    glm::vec3 y = glm::normalize(glm::cross(z, x));

    model_view = glm::mat4(1.0f);
    for (int i = 0; i < 3; i++) {
        model_view[i][0] = x[i];
        model_view[i][1] = y[i];
        model_view[i][2] = z[i];
    }
    model_view[3][0] = -glm::dot(x, eye);
    model_view[3][1] = -glm::dot(x, eye);
    model_view[3][2] = -glm::dot(z, eye);
}

void create_projection(const glm::vec3 &eye, const glm::vec3 &center) {
	projection = glm::mat4(1.0f);
	projection[2][3] = -1.f/(eye - center).length();
}

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

glm::vec3 barycentric_coordinates(glm::vec3 v[3], glm::vec3 P) {
    glm::vec3 w = glm::cross(
        glm::vec3(v[2].x - v[0].x, v[1].x - v[0].x, v[0].x - P.x), 
        glm::vec3(v[2].y - v[0].y, v[1].y - v[0].y, v[0].y - P.y)
    );
    if (std::abs(w.z) < 1.f) return glm::vec3(-1.f, 1.f, 1.f);
    return glm::vec3(1.f - (w.x + w.y)/w.z, w.y/w.z, w.x/w.z);
}

void triangle(glm::vec4 vertices[3], Shader &shader, TGAImage& image, TGAImage& zbuffer) {
    glm::vec2 bboxmin(std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
    glm::vec2 bboxmax(std::numeric_limits<float>::min(), std::numeric_limits<float>::min());
    glm::vec2 clamp(image.get_width()-1, image.get_height()-1);
	glm::vec3 v[3];
    for (int i=0; i<3; i++) {
		v[i] = glm::vec3(
			(int)(vertices[i].x/vertices[i].w),
			(int)(vertices[i].y/vertices[i].w),
			(int)(vertices[i].z/vertices[i].w)
		);

        bboxmin.x = std::max(0.f,     std::min(bboxmin.x, v[i].x));
        bboxmin.y = std::max(0.f,     std::min(bboxmin.y, v[i].y));

        bboxmax.x = std::min(clamp.x, std::max(bboxmax.x, v[i].x));
        bboxmax.y = std::min(clamp.y, std::max(bboxmax.y, v[i].y));
    }

    glm::vec3 P;
    for (P.x = (int)bboxmin.x; P.x <= (int)bboxmax.x; P.x++) {
        for (P.y = (int)bboxmin.y; P.y <= (int)bboxmax.y; P.y++) {
            glm::vec3 bc  = barycentric_coordinates(v, P);
            if (bc.x < 0.f || bc.y < 0.f || bc.z < 0.f) continue;
			float z = vertices[0].z*bc.x + vertices[1].z*bc.y + vertices[2].z*bc.z;
			float w = vertices[0].w*bc.x + vertices[1].w*bc.y + vertices[2].w*bc.z;

			int frag_depth = std::max(0, std::min(255, (int)(z/w + 0.5f)));
			// int frag_depth = std::max(0, std::min(255, (int)(z + 0.5f)));
			if (zbuffer.get(P.x, P.y)[0] > frag_depth) continue;
			TGAColor color;
			bool discard = shader.fragment(bc, color);
			if (!discard) {
				zbuffer.set(P.x, P.y, TGAColor(frag_depth));
				image.set(P.x, P.y, color);
			}
            // P.z = 0.f;

            // P.z += vertices[0].z/vertices[0].w*bc.x;
            // P.z += vertices[1].z/vertices[1].w*bc.y;
            // P.z += vertices[2].z/vertices[2].w*bc.z;
            // P.z += v[0].z*bc.x;
            // P.z += v[1].z*bc.y;
            // P.z += v[2].z*bc.z;
            // P.z += vertices[0].z*bc.x;
            // P.z += vertices[1].z*bc.y;
            // P.z += vertices[2].z*bc.z;
			// std::cout << P.z << "\n";
   //          if (zbuffer.get((int)P.x, (int)P.y)[0] < P.z) {
   //              zbuffer.set((int)P.x, (int)P.y, P.z);
			//
			// 	TGAColor color = TGAColor(255, 255, 255);
			// 	bool discard = shader.fragment(bc, color);
   //              // color = white;
   //              image.set(P.x, P.y, color);
   //          }
        }
    }
}
