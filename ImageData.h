#ifndef IMAGEDATA_H
#define IMAGEDATA_H
#define STB_IMAGE_IMPLEMENTATION 
#include <stb_image.h>
#include <iostream>
/*
* I tried using the loadBitMap function provided in assignment 4 but it didn't recognize water/displacement map 
* as valid bmp file. I used this stb_image to load the texture instead.
* 
*/
struct ImageData {
	int width, height, nChannels;
	unsigned char* data;
	bool loaded;

	ImageData(const char* path) : loaded(false) {
		stbi_set_flip_vertically_on_load(true); 
		data = stbi_load(path, &width, &height, &nChannels, 0);
		if (data == NULL) {
			std::cout << "Failed to load " << path << ": " << stbi_failure_reason() << std::endl;
			return;
		}
		loaded = true;
	}

	void free() {
		stbi_image_free(data);
	}
};

#endif