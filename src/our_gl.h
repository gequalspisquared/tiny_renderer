#ifndef OUR_GL_H
#define OUR_GL_H

#include "tgaimage.h"
#include "model.h"

extern glm::mat4 model_view;
extern glm::mat4 projection;
extern glm::mat4 viewport;

void look_at(glm::vec3 eye, glm::vec3 center, glm::vec3 up);
void create_projection(const glm::vec3 &eye, const glm::vec3 &center);
void create_viewport(int w, int h);

struct Shader {
    virtual ~Shader();
    virtual glm::vec4 vertex(int iface, int nthvert) = 0;
    virtual bool fragment(glm::vec3 bar, TGAColor &color) = 0;
};

void triangle(glm::vec4 verts[3], Shader &shader, TGAImage &image, TGAImage &zbuffer);

#endif
