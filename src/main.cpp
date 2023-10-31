#include <limits>
#include <vector>
#include <iostream>
#include <cstdlib>

#include "glm/ext/matrix_transform.hpp"
#include "tgaimage.h"
// #include "geometry.h"
#include "glm/glm.hpp"
#include "glm/gtx/string_cast.hpp"
#include "model.h"

const int width = 800;
const int height = 800;

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red   = TGAColor(255, 0,   0,   255);
const TGAColor green = TGAColor(0,   255, 0,   255);
const TGAColor blue  = TGAColor(0,   0,   255, 255);

glm::vec3 camera(1.f, 1.f, 3.f);

void line(int x0, int y0, int x1, int y1, TGAImage &image, const TGAColor &color) {
    bool steep = false;
    if (std::abs(x0 - x1) < std::abs(y0 - y1)) { // if the line is steep,
        std::swap(x0, y0);                       // transpose the image
        std::swap(x1, y1);
        steep = true;
    }

    if (x0 > x1) { // make it left to right
        std::swap(x0, x1);
        std::swap(y0, y1);
    }

    int dx = x1 - x0;
    int dy = y1 - y0;

    int derror2 = std::abs(dy)*2;
    int error2 = 0;
    int y = y0;

    for (int x = x0; x <= x1; x++) {
        if (steep) {
            image.set(y, x, color); // de-transpose the image
        } else {
            image.set(x, y, color);
        }

        error2 += derror2;
        if (error2 > dx) {
            y += (y1 > y0 ? 1 : -1);
            error2 -= dx*2;
        }
    }
}

void fill_border(TGAImage &image, TGAColor color, int thickness = 1) {
    int width = image.get_width();
    int height = image.get_height();

    for (int t = 0; t < thickness; t++) {
        for (int x = t; x < width - t; x++) {
            image.set(x, t, color);
            image.set(x, height - 1 - t, color);
        }

        for (int y = t; y < height - 1 - t; y++) {
            image.set(t, y, color);
            image.set(width - 1 - t, y, color);
        }
    }
}

glm::vec3 barycentric_coordinates(glm::ivec2 v[3], glm::ivec2 P) {
    glm::vec3 w = glm::cross(
        glm::vec3(v[2].x - v[0].x, v[1].x - v[0].x, v[0].x - P.x), 
        glm::vec3(v[2].y - v[0].y, v[1].y - v[0].y, v[0].y - P.y)
    );
    if (std::abs(w.z) < 1.f) return glm::vec3(-1.f, 1.f, 1.f);
    return glm::vec3(1.f - (w.x + w.y)/w.z, w.y/w.z, w.x/w.z);
}

glm::vec3 barycentric_coordinates(glm::vec3 v[3], glm::vec3 P) {
    glm::vec3 w = glm::cross(
        glm::vec3(v[2].x - v[0].x, v[1].x - v[0].x, v[0].x - P.x), 
        glm::vec3(v[2].y - v[0].y, v[1].y - v[0].y, v[0].y - P.y)
    );
    if (std::abs(w.z) < 1.f) return glm::vec3(-1.f, 1.f, 1.f);
    return glm::vec3(1.f - (w.x + w.y)/w.z, w.y/w.z, w.x/w.z);
}

void triangle(glm::ivec2 vertices[3], TGAImage& image, const TGAColor& color, bool wireframe = false) {
    glm::ivec2 bboxmin(image.get_width() - 1, image.get_height() - 1);
    glm::ivec2 bboxmax(0, 0);
    glm::ivec2 clamp(image.get_width() - 1, image.get_height() - 1);
    for (int i = 0; i < 3; i++) {
        bboxmin.x = std::max(0, std::min(bboxmin.x, vertices[i].x));
        bboxmin.y = std::max(0, std::min(bboxmin.y, vertices[i].y));

        bboxmax.x = std::min(clamp.x, std::max(bboxmax.x, vertices[i].x));
        bboxmax.y = std::min(clamp.y, std::max(bboxmax.y, vertices[i].y));
    }

    glm::ivec2 P;
    for (P.x = bboxmin.x; P.x <= bboxmax.x; P.x++) {
        for (P.y = bboxmin.y; P.y <= bboxmax.y; P.y++) {
            glm::vec3 bc = barycentric_coordinates(vertices, P);
            if (bc.x < 0.f || bc.y < 0.f || bc.z < 0.f) {
                continue;
            }
            image.set(P.x, P.y, color);
        }
    }
}

void triangle(glm::vec3 vertices[3], float *zbuffer, TGAImage& image, const TGAColor& color) {
    glm::vec2 bboxmin(std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
    glm::vec2 bboxmax(std::numeric_limits<float>::min(), std::numeric_limits<float>::min());
    glm::vec2 clamp(image.get_width()-1, image.get_height()-1);
    for (int i=0; i<3; i++) {
        bboxmin.x = std::max(0.f,     std::min(bboxmin.x, vertices[i].x));
        bboxmin.y = std::max(0.f,     std::min(bboxmin.y, vertices[i].y));

        bboxmax.x = std::min(clamp.x, std::max(bboxmax.x, vertices[i].x));
        bboxmax.y = std::min(clamp.y, std::max(bboxmax.y, vertices[i].y));
    }

    glm::vec3 P;
    for (P.x=bboxmin.x; P.x<=bboxmax.x; P.x++) {
        for (P.y=bboxmin.y; P.y<=bboxmax.y; P.y++) {
            glm::vec3 bc  = barycentric_coordinates(vertices, P);
            if (bc.x < 0.f || bc.y < 0.f || bc.z < 0.f) continue;
            P.z = 0.f;
            P.z += vertices[0].z*bc.x;
            P.z += vertices[1].z*bc.y;
            P.z += vertices[2].z*bc.z;
            if (zbuffer[int(P.x+P.y*width)] < P.z) {
                zbuffer[int(P.x+P.y*width)] = P.z;
                image.set(P.x, P.y, color);
            }
        }
    }
}

void textured_triangle(glm::vec3 vertices[3], glm::vec3 uvs[3], float *zbuffer, TGAImage& image, TGAImage& texture) {
    glm::vec2 bboxmin(std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
    glm::vec2 bboxmax(std::numeric_limits<float>::min(), std::numeric_limits<float>::min());
    glm::vec2 clamp(image.get_width()-1, image.get_height()-1);
    for (int i=0; i<3; i++) {
        bboxmin.x = std::max(0.f,     std::min(bboxmin.x, vertices[i].x));
        bboxmin.y = std::max(0.f,     std::min(bboxmin.y, vertices[i].y));

        bboxmax.x = std::min(clamp.x, std::max(bboxmax.x, vertices[i].x));
        bboxmax.y = std::min(clamp.y, std::max(bboxmax.y, vertices[i].y));
    }

    glm::vec3 P;
    for (P.x=bboxmin.x; P.x<=bboxmax.x; P.x++) {
        for (P.y=bboxmin.y; P.y<=bboxmax.y; P.y++) {
            glm::vec3 bc  = barycentric_coordinates(vertices, P);
            if (bc.x < 0.f || bc.y < 0.f || bc.z < 0.f) continue;
            P.z = 0.f;

            P.z += vertices[0].z*bc.x;
            P.z += vertices[1].z*bc.y;
            P.z += vertices[2].z*bc.z;
            if (zbuffer[int(P.x+P.y*width)] < P.z) {
                zbuffer[int(P.x+P.y*width)] = P.z;

                glm::vec3 texcoords = glm::vec3(
                    (int)((uvs[0].x*bc.x + uvs[1].x*bc.y + uvs[2].x*bc.z)*texture.get_width()),
                    (int)((uvs[0].y*bc.x + uvs[1].y*bc.y + uvs[2].y*bc.z)*texture.get_height()),
                    0.f
                );

                TGAColor color = texture.get(texcoords.x, texcoords.y);
                image.set(P.x, P.y, color);
            }
        }
    }
}

void textured_lighted_triangle(glm::vec3 vertices[3], glm::vec3 uvs[3], float *zbuffer, TGAImage& image, TGAImage& texture, float intensity) {
    glm::vec2 bboxmin(std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
    glm::vec2 bboxmax(std::numeric_limits<float>::min(), std::numeric_limits<float>::min());
    glm::vec2 clamp(image.get_width()-1, image.get_height()-1);
    for (int i=0; i<3; i++) {
        bboxmin.x = std::max(0.f,     std::min(bboxmin.x, vertices[i].x));
        bboxmin.y = std::max(0.f,     std::min(bboxmin.y, vertices[i].y));

        bboxmax.x = std::min(clamp.x, std::max(bboxmax.x, vertices[i].x));
        bboxmax.y = std::min(clamp.y, std::max(bboxmax.y, vertices[i].y));
    }

    glm::vec3 P;
    for (P.x=bboxmin.x; P.x<=bboxmax.x; P.x++) {
        for (P.y=bboxmin.y; P.y<=bboxmax.y; P.y++) {
            glm::vec3 bc  = barycentric_coordinates(vertices, P);
            if (bc.x < 0.f || bc.y < 0.f || bc.z < 0.f) continue;
            P.z = 0.f;

            P.z += vertices[0].z*bc.x;
            P.z += vertices[1].z*bc.y;
            P.z += vertices[2].z*bc.z;
            if (zbuffer[int(P.x+P.y*width)] < P.z) {
                zbuffer[int(P.x+P.y*width)] = P.z;

                glm::vec3 texcoords = glm::vec3(
                    (int)((uvs[0].x*bc.x + uvs[1].x*bc.y + uvs[2].x*bc.z)*texture.get_width()),
                    (int)((uvs[0].y*bc.x + uvs[1].y*bc.y + uvs[2].y*bc.z)*texture.get_height()),
                    0.f
                );

                TGAColor color = texture.get(texcoords.x, texcoords.y);
                color.r *= intensity;
                color.g *= intensity;
                color.b *= intensity;
                // color = white;
                image.set(P.x, P.y, color);
            }
        }
    }
}

void textured_gouraud_triangle(glm::vec3 vertices[3], glm::vec3 uvs[3], glm::vec3 norms[3], float *zbuffer, TGAImage& image, TGAImage& texture, glm::vec3 light_dir) {
    glm::vec2 bboxmin(std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
    glm::vec2 bboxmax(std::numeric_limits<float>::min(), std::numeric_limits<float>::min());
    glm::vec2 clamp(image.get_width()-1, image.get_height()-1);
    for (int i=0; i<3; i++) {
        bboxmin.x = std::max(0.f,     std::min(bboxmin.x, vertices[i].x));
        bboxmin.y = std::max(0.f,     std::min(bboxmin.y, vertices[i].y));

        bboxmax.x = std::min(clamp.x, std::max(bboxmax.x, vertices[i].x));
        bboxmax.y = std::min(clamp.y, std::max(bboxmax.y, vertices[i].y));
    }

    glm::vec3 P;
    for (P.x=bboxmin.x; P.x<=bboxmax.x; P.x++) {
        for (P.y=bboxmin.y; P.y<=bboxmax.y; P.y++) {
            glm::vec3 bc  = barycentric_coordinates(vertices, P);
            if (bc.x < 0.f || bc.y < 0.f || bc.z < 0.f) continue;
            P.z = 0.f;

            P.z += vertices[0].z*bc.x;
            P.z += vertices[1].z*bc.y;
            P.z += vertices[2].z*bc.z;
            if (zbuffer[int(P.x+P.y*width)] < P.z) {
                zbuffer[int(P.x+P.y*width)] = P.z;

                glm::vec3 texcoords = glm::vec3(
                    (int)((uvs[0].x*bc.x + uvs[1].x*bc.y + uvs[2].x*bc.z)*texture.get_width()),
                    (int)((uvs[0].y*bc.x + uvs[1].y*bc.y + uvs[2].y*bc.z)*texture.get_height()),
                    0.f
                );

                TGAColor color = texture.get(texcoords.x, texcoords.y);

                glm::vec3 normal = glm::vec3(
                    norms[0].x*bc.x + norms[1].x*bc.y + norms[2].x*bc.z,
                    norms[0].y*bc.x + norms[1].y*bc.y + norms[2].y*bc.z,
                    norms[0].z*bc.x + norms[1].z*bc.y + norms[2].z*bc.z
                );
                normal = glm::normalize(normal);
                float intensity = -glm::dot(normal, light_dir);

                if (intensity > 0) {
                    color.r *= intensity;
                    color.g *= intensity;
                    color.b *= intensity;
                } else {
                    continue;
                }
                // color = white;
                image.set(P.x, P.y, color);
            }
        }
    }
}

void draw_model_random_colors(const Model& model, TGAImage& image) {
    for (int i = 0; i < model.nfaces(); i++) {
        std::vector<int> face = model.face(i);
        glm::ivec2 screen_coords[3];
        for (int j = 0; j < 3; j++) {
            glm::vec3 world_coords = model.vert(face[3*j + 0]);
            screen_coords[j] = glm::ivec2((world_coords.x+1.)*width/2., (world_coords.y+1.)*height/2.);
        }
        TGAColor color(std::rand()%255, std::rand()%255, std::rand()%255, 255);
        triangle(screen_coords, image, color);
    }
}

void draw_model_lighted(const Model& model, TGAImage& image, glm::vec3 light_dir) {
    for (int i = 0; i < model.nfaces(); i++) {
        std::vector<int> face = model.face(i);
        glm::ivec2 screen_coords[3];
        glm::vec3 world_coords[3];
        for (int j = 0; j < 3; j++) {
            glm::vec3 v = model.vert(face[3*j + 0]);
            screen_coords[j] = glm::ivec2((v.x+1.)*width/2., (v.y+1.)*height/2.);
            world_coords[j] = v;
        }
        glm::vec3 normal = glm::cross(
            world_coords[2] - world_coords[0], 
            world_coords[1]-world_coords[0]
        );
        normal = glm::normalize(normal);
        float intensity = glm::dot(normal, light_dir);
        if (intensity > 0) {
            TGAColor color(intensity*255, intensity*255, intensity*255, 255);
            triangle(screen_coords, image, color);
        }
    }
}

void draw_model_lighted_zbuffer(const Model& model, float *zbuffer, TGAImage& image, glm::vec3 light_dir) {
    for (int i = 0; i < model.nfaces(); i++) {
        std::vector<int> face = model.face(i);
        glm::vec3 screen_coords[3];
        glm::vec3 world_coords[3];
        for (int j = 0; j < 3; j++) {
            glm::vec3 v = model.vert(face[3*j + 0]);
            screen_coords[j] = glm::vec3(
                (int)((v.x+1.)*width/2. + 0.5f), 
                (int)((v.y+1.)*height/2. + 0.5f), 
            v.z);
            world_coords[j] = v;
        }
        glm::vec3 normal = glm::cross(
            world_coords[2] - world_coords[0],
            world_coords[1]-world_coords[0]
        );
        normal = glm::normalize(normal);
        float intensity = glm::dot(normal, light_dir);
        if (intensity > 0) {
            TGAColor color(intensity*255, intensity*255, intensity*255, 255);
            triangle(screen_coords, zbuffer, image, color);
        }
    }
}

void draw_model_zbuffer_textured(const Model& model, float *zbuffer, TGAImage& image, TGAImage& texture) {
    for (int i = 0; i < model.nfaces(); i++) {
        std::vector<int> face = model.face(i);
        glm::vec3 screen_coords[3];
        glm::vec3 uvs[3];
        glm::vec3 world_coords[3];
        for (int j = 0; j < 3; j++) {
            glm::vec3 v = model.vert(face[3*j + 0]);
            screen_coords[j] = glm::vec3(
                (int)((v.x+1.)*width/2. + 0.5f), 
                (int)((v.y+1.)*height/2. + 0.5f), 
            v.z);
            uvs[j] = model.uv(face[3*j + 1]);
            world_coords[j] = v;
        }

        textured_triangle(screen_coords, uvs, zbuffer, image, texture);
    }
}

void draw_model_zbuffer_textured_lighted(const Model& model, float *zbuffer, TGAImage& image, TGAImage& texture, glm::vec3 light_dir) {
    for (int i = 0; i < model.nfaces(); i++) {
        std::vector<int> face = model.face(i);
        glm::vec3 screen_coords[3];
        glm::vec3 uvs[3];
        glm::vec3 world_coords[3];
        for (int j = 0; j < 3; j++) {
            glm::vec3 v = model.vert(face[3*j + 0]);
            screen_coords[j] = glm::vec3(
                (int)((v.x+1.)*width/2. + 0.5f), 
                (int)((v.y+1.)*height/2. + 0.5f), 
                v.z);
            uvs[j] = model.uv(face[3*j + 1]);
            world_coords[j] = v;
        }

        glm::vec3 normal = glm::cross(
            world_coords[2] - world_coords[0],
            world_coords[1] - world_coords[0]
        );
        normal = glm::normalize(normal);
        float intensity = glm::dot(normal, light_dir);
        if (intensity > 0) {
            TGAColor color(intensity*255, intensity*255, intensity*255, 255);
            textured_lighted_triangle(screen_coords, uvs, zbuffer, image, texture, intensity);
        }
    }
}

glm::mat4 Viewport(int x, int y, int w, int h) {
    glm::mat4 m = glm::mat4(1.f);
    int depth = 255;
    m[0][3] = x+w/2.f;
    m[1][3] = y+h/2.f;
    m[2][3] = depth/2.f;

    m[0][0] = w/2.f;
    m[1][1] = h/2.f;
    m[2][2] = depth/2.f;
    return m;
}

void draw_model_zbuffer_textured_lighted_perspective(const Model& model, float *zbuffer, TGAImage& image, TGAImage& texture, glm::vec3 light_dir) {
    float camera_dist = 3.0f;
    glm::mat4 perspective = glm::mat4(1.0f);
    perspective[2][3] = -1/camera_dist;
    glm::mat4 viewport = glm::mat4(0.f);
    viewport[0][0] = width/2.f;
    viewport[1][1] = height/2.f;
    viewport[2][2] = 255/2.f;

    viewport[3][0] = width/2.f + 0.5f;
    viewport[3][1] = height/2.f + 0.5f;
    viewport[3][2] = 255/2.f + 0.5f;
    viewport[3][3] = 1.f;
    for (int i = 0; i < model.nfaces(); i++) {
        std::vector<int> face = model.face(i);
        glm::vec3 screen_coords[3];
        glm::vec3 uvs[3];
        glm::vec3 world_coords[3];
        for (int j = 0; j < 3; j++) {
            glm::vec3 v = model.vert(face[3*j + 0]);
            screen_coords[j] = v;
            uvs[j] = model.uv(face[3*j + 1]);
            world_coords[j] = v;
        }

        glm::vec3 normal = glm::cross(
            world_coords[2] - world_coords[0],
            world_coords[1] - world_coords[0]
        );
        normal = glm::normalize(normal);
        float intensity = glm::dot(normal, light_dir);
        if (intensity > 0) {

            // transformations
            for (int j = 0; j < 3; j++) {
                glm::vec4 v4(screen_coords[j].x, screen_coords[j].y, screen_coords[j].z, 1.f);
                v4 = viewport*perspective*v4;
                screen_coords[j].x = (int)(v4.x/v4.w);
                screen_coords[j].y = (int)(v4.y/v4.w);
                screen_coords[j].z = v4.z/v4.w;
            }

            TGAColor color(intensity*255, intensity*255, intensity*255, 255);
            textured_lighted_triangle(screen_coords, uvs, zbuffer, image, texture, intensity);
        }
    }
}

void draw_model_zbuffer_textured_gouraud_perspective(const Model& model, float *zbuffer, TGAImage& image, TGAImage& texture, glm::vec3 light_dir) {
    float camera_dist = 3.0f;
    glm::mat4 perspective = glm::mat4(1.0f);
    perspective[2][3] = -1/camera_dist;
    glm::mat4 viewport = glm::mat4(0.f);
    viewport[0][0] = width/2.f;
    viewport[1][1] = height/2.f;
    viewport[2][2] = 255/2.f;

    viewport[3][0] = width/2.f + 0.5f;
    viewport[3][1] = height/2.f + 0.5f;
    viewport[3][2] = 255/2.f + 0.5f;
    viewport[3][3] = 1.f;
    for (int i = 0; i < model.nfaces(); i++) {
        std::vector<int> face = model.face(i);
        glm::vec3 screen_coords[3];
        glm::vec3 uvs[3];
        glm::vec3 norms[3];
        glm::vec3 world_coords[3];
        for (int j = 0; j < 3; j++) {
            glm::vec3 v = model.vert(face[3*j + 0]);
            screen_coords[j] = v;
            uvs[j] = model.uv(face[3*j + 1]);
            norms[j] = model.norm(face[3*j + 2]);
            world_coords[j] = v;
        }

        glm::vec3 normal = glm::cross(
            world_coords[2] - world_coords[0],
            world_coords[1] - world_coords[0]
        );
        normal = glm::normalize(normal);
        float intensity = glm::dot(normal, light_dir);
        if (intensity > 0) {

            // transformations
            for (int j = 0; j < 3; j++) {
                glm::vec4 v4(screen_coords[j].x, screen_coords[j].y, screen_coords[j].z, 1.f);
                v4 = viewport*perspective*v4;
                screen_coords[j].x = (int)(v4.x/v4.w);
                screen_coords[j].y = (int)(v4.y/v4.w);
                screen_coords[j].z = (int)(v4.z/v4.w);
            }

            textured_gouraud_triangle(screen_coords, uvs, norms, zbuffer, image, texture, light_dir);
        }
    }
}

glm::mat4 look_at(glm::vec3 eye, glm::vec3 center, glm::vec3 up) {
    glm::vec3 z = glm::normalize(eye - center);
    glm::vec3 x = glm::normalize(glm::cross(up, z));
    glm::vec3 y = glm::normalize(glm::cross(z, x));

    glm::mat4 inverse = glm::mat4(1.0f);
    glm::mat4 transpose = glm::mat4(1.0f);
    glm::mat4 res = glm::mat4(1.0f);
    // for (int i = 0; i < 3; i++) {
    //     inverse[0][i] = x[i];
    //     inverse[1][i] = y[i];
    //     inverse[2][i] = z[i];
    //     transpose[i][3] = -center[i];
    // }
    for (int i = 0; i < 3; i++) {
        res[i][0] = x[i];
        res[i][1] = y[i];
        res[i][2] = z[i];
        // res[3][i] = -eye[i];
    }
    res[3][0] = -glm::dot(x, eye);
    res[3][1] = -glm::dot(x, eye);
    res[3][2] = -glm::dot(z, eye);
    // return inverse*transpose;
    return res;
}

void draw_model_camera(const Model& model, float *zbuffer, TGAImage& image, TGAImage& texture, glm::vec3 light_dir) {
    float camera_dist = 3.0f;
    // glm::mat4 view = look_at(camera, glm::vec3(0.f), glm::vec3(0.f, 1.f, 0.f));
    glm::mat4 view = glm::lookAt(camera, glm::vec3(0.f), glm::vec3(0.f, 1.f, 0.f));
    glm::mat4 perspective = glm::mat4(1.0f);
    perspective[2][3] = -1.f/(camera).length();
    glm::mat4 viewport = glm::mat4(0.f);
    viewport[0][0] = width/2.f;
    viewport[1][1] = height/2.f;
    viewport[2][2] = 255/2.f;

    viewport[3][0] = width/2.f + 0.5f;
    viewport[3][1] = height/2.f + 0.5f;
    viewport[3][2] = 255/2.f + 0.5f;
    viewport[3][3] = 1.f;
    for (int i = 0; i < model.nfaces(); i++) {
        std::vector<int> face = model.face(i);
        glm::vec3 screen_coords[3];
        glm::vec3 uvs[3];
        glm::vec3 norms[3];
        glm::vec3 world_coords[3];
        for (int j = 0; j < 3; j++) {
            glm::vec3 v = model.vert(face[3*j + 0]);
            screen_coords[j] = v;
            uvs[j] = model.uv(face[3*j + 1]);
            norms[j] = model.norm(face[3*j + 2]);
            world_coords[j] = v;
        }

        glm::vec3 normal = glm::cross(
            world_coords[2] - world_coords[0],
            world_coords[1] - world_coords[0]
        );
        normal = glm::normalize(normal);
        float intensity = glm::dot(normal, light_dir);
        if (intensity > 0) {

            // transformations
            for (int j = 0; j < 3; j++) {
                glm::vec4 v4(screen_coords[j].x, screen_coords[j].y, screen_coords[j].z, 1.f);
                // v4 = viewport*perspective*v4;
                v4 = viewport*perspective*view*v4;
                screen_coords[j].x = (int)(v4.x/v4.w);
                screen_coords[j].y = (int)(v4.y/v4.w);
                screen_coords[j].z = (int)(v4.z/v4.w);
            }

            textured_gouraud_triangle(screen_coords, uvs, norms, zbuffer, image, texture, light_dir);
            // textured_triangle(screen_coords, uvs, zbuffer, image, texture);
        }
    }
}

void draw_wireframe(const Model& model, TGAImage& image, const TGAColor& color) {
    for (int i = 0; i < model.nfaces(); i++) {
        std::vector<int> face = model.face(i);
        for (int j = 0; j < 3; j++) {
            glm::vec3 v0 = model.vert(face[j]);
            glm::vec3 v1 = model.vert(face[(j + 1)%3]);
            int x0 = (v0.x + 1.)*width/2.;
            int y0 = (v0.y + 1.)*height/2.;
            int x1 = (v1.x + 1.)*width/2.;
            int y1 = (v1.y + 1.)*height/2.;
            line(x0, y0, x1, y1, image, green);
        }
    }
}

int main(int argc, char** argv) {
    Model *model = nullptr;
    if (argc == 2) {
        model = new Model(argv[1]);
    } else {
        model = new Model("african_head.obj");
    }

    TGAImage image(width, height, TGAImage::RGB);
    TGAImage texture;
    if (!texture.read_tga_file("african_head_diffuse.tga")) {
        std::cerr << "Failed to open tga file\n";
    }
    texture.flip_vertically();
    fill_border(image, blue, 2);
    // draw_model_random_colors(*model, image);
    float *zbuffer = new float[width * height];
    for (int i = 0; i < width * height; i++) {
        zbuffer[i] = -std::numeric_limits<float>::max();
    }
    // draw_model_lighted(*model, image, glm::vec3(0.f, 0.f, -1.f));
    // draw_model_lighted_zbuffer(*model, zbuffer, image, glm::vec3(0.f, 0.f, -1.f));
    // draw_model_zbuffer_textured(*model, zbuffer, image, texture);
    // draw_model_zbuffer_textured_lighted(*model, zbuffer, image, texture, glm::vec3(0.f, 0.f, -1.f));
    // draw_model_zbuffer_textured_lighted_perspective(*model, zbuffer, image, texture, glm::vec3(0.f, 0.f, -1.f));
    // draw_model_zbuffer_textured_gouraud_perspective(*model, zbuffer, image, texture, glm::vec3(0.f, 0.f, -1.f));
    // draw_model_camera(*model, zbuffer, image, texture, -glm::normalize(camera));
    draw_model_camera(*model, zbuffer, image, texture, glm::vec3(0.f, 0.f, -1.f));
    // draw_wireframe(*model, image, green);

    // glm::ivec2 t0[3] = {Vec2i(10, 70),   Vec2i(50, 160),  Vec2i(70, 80)}; 
    // glm::ivec2 t1[3] = {Vec2i(180, 50),  Vec2i(150, 1),   Vec2i(70, 180)}; 
    // glm::ivec2 t2[3] = {Vec2i(180, 150), Vec2i(120, 160), Vec2i(130, 180)}; 
    // triangle(t0, image, red);
    // triangle(t1, image, white);
    // triangle(t2, image, green);

    image.flip_vertically();
    image.write_tga_file("output.tga");
    delete model;
    delete[] zbuffer;
    return 0;
}

