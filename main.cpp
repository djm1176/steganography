#include "bitmap_image.hpp"
#include <iostream>
#include <fstream>

//Number of bytes used for header. Maximum payload size is the difference of maximum storable bytes and header size
//Byte	Length		Name		Description
//0		4			length		Number of bytes to decode from the bitmap to create the payload. Excludes the header.
constexpr int HEADER_SIZE = 4;

struct header_info {

	//Name of the payload file, including filename and extension
	std::string filename;

	//Length of the payload file in bytes
	unsigned int length = 0;
};

//Write header information into a buffer.
//The write operation will not exceed maximum header size.
void write_header(char* buffer, const header_info& header) {


	//Write length field
	buffer[0] = (header.length >> 24) & 0xFF;
	buffer[1] = (header.length >> 16) & 0xFF;
	buffer[2] = (header.length >> 8) & 0xFF;
	buffer[3] = header.length & 0xFF;

}


//Returns true if the nth bit in a byte is set.
inline bool GetBit(const char &byte, const int &n) {
	return (byte >> n) & 1UL;
}

//Sets the nth bit in a byte to the value of 'set' parameter.
//Defaults to setting bit n to 1.
inline bool SetBit(char& byte, const int& n, bool set = true) {
	byte ^= (-set ^ byte) & (1UL << n);
}

int main() {

	//Get user input
	std::string filename_payload = "";
	std::cout << "Enter in payload filename: " << std::endl;
	std::cin >> filename_payload;

	//Load the source bitmap image
	bitmap_image image("C:/temp/test.bmp");
	if (!image) {
		std::cout << "Failed to open bitmap image 'test.bmp'" << std::endl;
		return EXIT_FAILURE;
	}

	//Load a payload file
	std::ifstream fPayload("C:/temp/" + filename_payload, std::ios::binary);
	if (!fPayload.is_open()) {
		std::cout << "Failed to open payload file '" + filename_payload + "'" << std::endl;
		return EXIT_FAILURE;
	}

	//Get size of the payload
	fPayload.seekg(0, std::ios::beg);
	auto beg_pos = fPayload.tellg();
	fPayload.seekg(0, std::ios::end);
	auto end_pos = fPayload.tellg();
	unsigned long len = end_pos - beg_pos;
	std::cout << beg_pos << end_pos;
	fPayload.seekg(0, std::ios::beg);

	//Make sure payload isn't larger than storable bitmap bits
	//Compare bit lengths, not byte lengths
	if ((len + HEADER_SIZE) * 8 > image.height() * image.width() * 9) {
		std::cout << "Payload is too large to fit in image (Image can store " << image.height() * image.width() * 9 / 8 << " bytes, but payload is " << len << " bytes)" << std::endl;
		fPayload.close();

		return EXIT_FAILURE;
	}

	//Make sure payload isn't larger than 4GB
	if (len > 4294967295) {
		std::cout << "Payload is larger than maximum encode size of 4 GB" << std::endl;
		fPayload.close();

		return EXIT_FAILURE;
	}

	//Load file into buffer and close
	auto buff = std::get_temporary_buffer<char>(len + HEADER_SIZE);
	if (buff.second == 0) {
		std::cout << "Failed to get buffer. The payload may be too large to hold in memory!" << std::endl;
		fPayload.close();
		std::return_temporary_buffer(buff.first); //This may not be necessary. Might even throw an exception. Idk lmao

		return EXIT_FAILURE;
	}

	std::cout << "Successfully loaded buffer of size " << buff.second << std::endl;

	//Store into buffer, starting at an offset of HEADER_SIZE bytes

	fPayload.read(buff.first + HEADER_SIZE, buff.second);

	//Write the header into the buffer
	header_info info;
	info.length = buff.second;
	write_header(buff.first, info);

	std::cout << "Buffer filled with header and payload, ready to encode" << std::endl;

	//Modify bits in the source image
	unsigned long long _b = 0;
	int nlayers = 3; //Number of significant bits to use per channel
	long pixel_count = image.pixel_count();

	for (int y = 0, height = image.height(); y < height; ++y) {
		for (int x = 0, width = image.width(); x < width; ++x, _b += 3) {
			unsigned char r, g, b;
			image.get_pixel(x, y, r, g, b);


			//Set red bit 0
			if (_b + 0 <= buff.second * 8) {
				std::cout << "Setting red bit 0 to bit " << _b + 0 << ", which equals " << GetBit(buff.first[(_b + 0) / 8], _b + 0) << std::endl;
				r ^= (-(buff.first[(_b + 0) / 8] >> ((_b + 0) % 8)) & (1UL << 0));
			}

			//Set red bit 1
			if (_b + 1 <= buff.second * 8) {
				std::cout << "Setting red bit 0 to bit " << _b + 1 << ", which equals " << GetBit(buff.first[(_b + 1) / 8], _b + 1) << std::endl;
				r ^= (-(buff.first[(_b + 1) / 8] >> ((_b + 1) % 8)) & (1UL << 1));
			}

			//Set red bit 2
			if (_b + 2 <= buff.second * 8) {
				std::cout << "Setting red bit 0 to bit " << _b + 2 << ", which equals " << GetBit(buff.first[(_b + 2) / 8], _b + 2) << std::endl;
				r ^= (-(buff.first[(_b + 2) / 8] >> ((_b + 2) % 8)) & (1UL << 2));
			}

			//########################

			//Set green bit 0
			if (_b + (pixel_count * 1 * nlayers) + 0 <= buff.second * 8) {
				g ^= (-(buff.first[(_b * pixel_count * 1 + 0) / 8] >> ((_b * pixel_count * 1 + 1) % 8)) & (1UL << 0));
			}

			//Set green bit 1
			if (_b + (pixel_count * 1 * nlayers) + 1 <= buff.second * 8) {
				g ^= (-(buff.first[(_b * pixel_count * 1 + 1) / 8] >> ((_b * pixel_count * 1 + 1) % 8)) & (1UL << 1));
			}

			//Set green bit 2
			if (_b + (pixel_count * 1 * nlayers) + 2 <= buff.second * 8) {
				g ^= (-(buff.first[(_b * pixel_count * 1 + 2) / 8] >> ((_b * pixel_count * 1 + 1) % 8)) & (1UL << 2));
			}

			//########################

			//Set blue bit 0
			if (_b + (pixel_count * 2 * nlayers) + 0 <= buff.second * 8) {
				g ^= (-(buff.first[(_b * pixel_count * 2 + 0) / 8] >> ((_b * pixel_count * 2 + 1) % 8)) & (1UL << 0));
			}

			//Set blue bit 1
			if (_b + (pixel_count * 2 * nlayers) + 1 <= buff.second * 8) {
				g ^= (-(buff.first[(_b * pixel_count * 2 + 1) / 8] >> ((_b * pixel_count * 2 + 1) % 8)) & (1UL << 1));
			}

			//Set blue bit 2
			if (_b + (pixel_count * 2 * nlayers) + 2 <= buff.second * 8) {
				g ^= (-(buff.first[(_b * pixel_count * 2 + 2) / 8] >> ((_b * pixel_count * 2 + 1) % 8)) & (1UL << 2));
			}


			image.set_pixel(x, y, r, g, b);

		}
	}

	std::return_temporary_buffer(buff.first);

	image.save_image("C:/temp/test_out.bmp");
	std::cout << "Output image saved" << std::endl;

	return EXIT_SUCCESS;
}