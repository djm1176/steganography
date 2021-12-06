#include "bitmap_image.hpp"
#include <iostream>
#include <fstream>

//Number of bits used for header. Maximum payload size is the difference of maximum storable bits and header size
constexpr int HEADER_SIZE = 8;

int main() {

	//Load the encoded file
	std::string filename = "C:/temp/test_out.bmp";
	bitmap_image image(filename);

	if (!image) {
		std::cout << "No encoded file named 'C:/temp/test_out.bmp' exists";
		return EXIT_FAILURE;
	}




	return EXIT_SUCCESS;
}