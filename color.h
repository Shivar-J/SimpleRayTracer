#ifndef COLOR_H
#define COLOR_H

#include "vec3.h"
#include <iostream>
#include <sstream>
#include <algorithm>

static int width;
static int height;
static vec3* renderBuffer;

void WriteBufferToFile(std::ostream& file) {
	std::stringstream buf;
	int pixelCount = width * height;

	for (int i = pixelCount; i > 0; i--) {
		buf << static_cast<int>(256 * clamp(renderBuffer[i].r, 0.0, 0.999)) << ' '
			<< static_cast<int>(256 * clamp(renderBuffer[i].g, 0.0, 0.999)) << ' '
			<< static_cast<int>(256 * clamp(renderBuffer[i].b, 0.0, 0.999)) << '\n';
	}
	file << buf.str();
}

void write_color(int x, int y, color pixel_color, int samples_per_pixel) {
	auto r = pixel_color.r;
	auto g = pixel_color.g;
	auto b = pixel_color.b;

	auto scale = 1.0 / samples_per_pixel;
	r = sqrt(scale * r);
	g = sqrt(scale * g);
	b = sqrt(scale * b);

	int index = width * y + x;

	renderBuffer[index] = vec3(r, g, b);
}

void write_color(std::ostream& out, color pixel_color, int samples_per_pixel) {
	auto r = pixel_color.x;
	auto g = pixel_color.y;
	auto b = pixel_color.z;

	if (r != r) r = 0.0;
	if (g != g) g = 0.0;
	if (b != b) b = 0.0;

	// Divide the color by the number of samples and gamma-correct for gamma=2.0.
	auto scale = 1.0 / samples_per_pixel;
	r = sqrt(scale * r);
	g = sqrt(scale * g);
	b = sqrt(scale * b);

	// Write the translated [0,255] value of each color component.
	out << static_cast<int>(256 * clamp(r, 0.0, 0.999)) << ' '
		<< static_cast<int>(256 * clamp(g, 0.0, 0.999)) << ' '
		<< static_cast<int>(256 * clamp(b, 0.0, 0.999)) << '\n';
}

#endif