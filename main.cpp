#include <vector>
#include <iostream>
#include <cstdlib>

#include "tgaimage.h"
#include "geometry.h"
#include "model.h"

const int width = 800;
const int height = 800;

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red   = TGAColor(255, 0,   0,   255);
const TGAColor green = TGAColor(0,   255, 0,   255);
const TGAColor blue  = TGAColor(0,   0,   255, 255);

Model *model = nullptr;

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

Vec3f barycentric_coordinates(Vec2i v[3], Vec2i P) {
	Vec3f w = Vec3f(v[2].x - v[0].x, v[1].x - v[0].x, v[0].x - P.x) ^ Vec3f(v[2].y - v[0].y, v[1].y - v[0].y, v[0].y - P.y);
	if (std::abs(w.z) < 1.f) return Vec3f(-1.f, 1.f, 1.f);
	return Vec3f(1.f - (w.x + w.y)/w.z, w.y/w.z, w.x/w.z);
}

void triangle(Vec2i vertices[3], TGAImage& image, const TGAColor& color, bool wireframe = false) {
	Vec2i bboxmin(image.get_width() - 1, image.get_height() - 1);
	Vec2i bboxmax(0, 0);
	Vec2i clamp(image.get_width() - 1, image.get_height() - 1);
	for (int i = 0; i < 3; i++) {
		bboxmin.x = std::max(0, std::min(bboxmin.x, vertices[i].x));
		bboxmin.y = std::max(0, std::min(bboxmin.y, vertices[i].y));

		bboxmax.x = std::min(clamp.x, std::max(bboxmax.x, vertices[i].x));
		bboxmax.y = std::min(clamp.y, std::max(bboxmax.y, vertices[i].y));
	}
	
	Vec2i P;
	for (P.x = bboxmin.x; P.x <= bboxmax.x; P.x++) {
		for (P.y = bboxmin.y; P.y <= bboxmax.y; P.y++) {
			Vec3f bc = barycentric_coordinates(vertices, P);
			if (bc.x < 0.f || bc.y < 0.f || bc.z < 0.f) {
				continue;
			}
			image.set(P.x, P.y, color);
		}
	}
}

void draw_model_random_colors(const Model& model, TGAImage& image) {
	for (int i = 0; i < model.nfaces(); i++) {
		std::vector<int> face = model.face(i);
		Vec2i screen_coords[3];
		for (int j = 0; j < 3; j++) {
			Vec3f world_coords = model.vert(face[j]);
			screen_coords[j] = Vec2i((world_coords.x+1.)*width/2., (world_coords.y+1.)*height/2.);
		}
		TGAColor color(std::rand()%255, std::rand()%255, std::rand()%255, 255);
		triangle(screen_coords, image, color);
	}
}

void draw_model_lighted(const Model& model, TGAImage& image, Vec3f light_dir) {
	for (int i = 0; i < model.nfaces(); i++) {
		std::vector<int> face = model.face(i);
		Vec2i screen_coords[3];
		Vec3f world_coords[3];
		for (int j = 0; j < 3; j++) {
			Vec3f v = model.vert(face[j]);
			screen_coords[j] = Vec2i((v.x+1.)*width/2., (v.y+1.)*height/2.);
			world_coords[j] = v;
		}
		Vec3f normal = (world_coords[2] - world_coords[0])^(world_coords[1]-world_coords[0]);
		normal.normalize();
		float intensity = normal * light_dir;
		if (intensity > 0) {
			TGAColor color(intensity*255, intensity*255, intensity*255, 255);
			triangle(screen_coords, image, color);
		}
	}
}

void draw_wireframe(const Model& model, TGAImage& image, const TGAColor& color) {
	for (int i = 0; i < model.nfaces(); i++) {
		std::vector<int> face = model.face(i);
		for (int j = 0; j < 3; j++) {
			Vec3f v0 = model.vert(face[j]);
			Vec3f v1 = model.vert(face[(j + 1)%3]);
			int x0 = (v0.x + 1.)*width/2.;
			int y0 = (v0.y + 1.)*height/2.;
			int x1 = (v1.x + 1.)*width/2.;
			int y1 = (v1.y + 1.)*height/2.;
			line(x0, y0, x1, y1, image, green);
		}
	}
}

int main(int argc, char** argv) {
	// TGAImage image(100, 100, TGAImage::RGB);
	// image.set(52, 41, red);
	// line(10, 10, 60, 80, image, red);
	// line(20, 20, 60, 40, image, green);
	// fill_border(image, white);
	// image.flip_vertically(); // i want to have the origin at the left bottom corner of the image
	// image.write_tga_file("output.tga");
	// return 0;
	
	if (argc == 2) {
		model = new Model(argv[1]);
	} else {
		model = new Model("african_head.obj");
	}

	TGAImage image(width, height, TGAImage::RGB);
	fill_border(image, blue, 2);
	// draw_model_random_colors(*model, image);
	draw_model_lighted(*model, image, Vec3f(0.f, 0.f, -1.f));
	// draw_wireframe(*model, image, green);
	
	// Vec2i t0[3] = {Vec2i(10, 70),   Vec2i(50, 160),  Vec2i(70, 80)}; 
	// Vec2i t1[3] = {Vec2i(180, 50),  Vec2i(150, 1),   Vec2i(70, 180)}; 
	// Vec2i t2[3] = {Vec2i(180, 150), Vec2i(120, 160), Vec2i(130, 180)}; 
	// triangle(t0[0], t0[1], t0[2], image, red); 
	// triangle(t1[0], t1[1], t1[2], image, white); 
	// triangle(t2[0], t2[1], t2[2], image, green);
	// triangle(t0, image, red);
	// triangle(t1, image, white);
	// triangle(t2, image, green);
	// triangle(t0[0], t0[1], t0[2], image, green, true); 
	// triangle(t1[0], t1[1], t1[2], image, blue, true); 
	// triangle(t2[0], t2[1], t2[2], image, red, true);

	image.flip_vertically();
	image.write_tga_file("output.tga");
	delete model;
	return 0;
}

