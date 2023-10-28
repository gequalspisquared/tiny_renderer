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
	fill_border(image, red, 2);
	for (int i = 0; i < model->nfaces(); i++) {
		std::vector<int> face = model->face(i);
		for (int j = 0; j < 3; j++) {
			Vec3f v0 = model->vert(face[j]);
			Vec3f v1 = model->vert(face[(j + 1)%3]);
			int x0 = (v0.x + 1.)*width/2.;
			int y0 = (v0.y + 1.)*height/2.;
			int x1 = (v1.x + 1.)*width/2.;
			int y1 = (v1.y + 1.)*height/2.;
			line(x0, y0, x1, y1, image, green);
		}
	}

	image.flip_vertically();
	image.write_tga_file("output.tga");
	delete model;
	return 0;
}

