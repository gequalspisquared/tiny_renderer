#ifndef HEADER_H
#define HEADER_H

#include "tgaimage.h"
#include "glm/glm.hpp"

extern glm::mat4 view;
extern glm::mat4 viewport;
extern glm::mat4 projection;

void create_viewport(int w, int h);
void create_projection(float coeff = 0.f); // coeff = -1/c 
void look_at(glm::vec3 eye, glm::vec3 center, glm::vec3 up);

struct Shader {
    virtual ~Shader();
    virtual glm::vec4 vertex(int iface, int nthvert) = 0;
    virtual bool fragment(glm::vec3 bar, TGAColor &color) = 0;
};

void triangle(glm::vec4 *vertices, Shader &shader, TGAImage &image, TGAImage &zbuffer);

#endif
