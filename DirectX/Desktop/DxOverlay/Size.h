#pragma once

struct Size {
	unsigned int width;
	unsigned int height;

	Size(unsigned int w, unsigned int h) : width(w), height(h) {}

	bool operator != (const Size& other) {
		return width != other.width || height != other.height;
	}
};