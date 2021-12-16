#include "bitmap_image.hpp"
#include <iostream>
#include <fstream>

#define __FLIP_BIT_ORDER

//Number of bytes used for header. Maximum payload size is the difference of maximum storable bytes and header size
//Byte	Length		Name		Description
//0		1			Magic		A magic byte equal to 0xED, short for "Encoded Data".
//1		4			length		Number of bytes to decode from the bitmap to create the payload. Excludes the header.
constexpr int HEADER_SIZE = 5;
constexpr unsigned char HEADER_MAGIC = 0xED;

struct header_info {

	//Name of the payload file, including filename and extension
	std::string filename;

	//Length of the payload file in bytes
	unsigned int length = 0;
};

//Write header information into a buffer.
void write_header(char* buffer, const header_info& header) {

	//Write magic byte
	buffer[0] = HEADER_MAGIC;

	//Write length field
	buffer[1] = (header.length >> 24) & 0xFF;
	buffer[2] = (header.length >> 16) & 0xFF;
	buffer[3] = (header.length >> 8) & 0xFF;
	buffer[4] = header.length & 0xFF;

}

//Returns true if the nth bit in a byte is set.
inline bool GetBit(const char &byte, const int &n) {
#ifdef __FLIP_BIT_ORDER
	return (byte >> (7 - n)) & 1UL;
#else
	return (byte >> n) & 1UL;
#endif // __FLIP_BIT_ORDER

}

//Sets the nth bit in a byte to the value of 'set' parameter.
//Defaults to setting bit n to 1.
inline void SetBit(unsigned char& byte, const int& n, bool set = true) {
#ifdef __FLIP_BIT_ORDER
	byte ^= (-set ^ byte) & (1UL << (7 - n));
#else
	byte ^= (-set ^ byte) & (1UL << n);
#endif // __FLIP_BIT_ORDER


}

// Returns true if the image contained in parameter img has encoded data.
// A true value does not guarantee the image has actual encoded data; only magic byte is checked.
bool IsEncodedImage(const bitmap_image& img) {
	//Check magic byte

	unsigned char magic = 0x0;
	rgb_t p1, p2, p3;

	img.get_pixel(0, 0, p1);
	img.get_pixel(0, 1, p2);
	img.get_pixel(0, 2, p3);

	SetBit(magic, 0, GetBit(p1.red, 0));
	SetBit(magic, 1, GetBit(p1.red, 1));
	SetBit(magic, 2, GetBit(p1.red, 2));

	SetBit(magic, 3, GetBit(p2.red, 0));
	SetBit(magic, 4, GetBit(p2.red, 1));
	SetBit(magic, 5, GetBit(p2.red, 2));

	SetBit(magic, 6, GetBit(p3.red, 0));
	SetBit(magic, 7, GetBit(p3.red, 1));

	return magic == HEADER_MAGIC;
}

// Returns the size of payload from an encoded image, without iterating over the entire bitmap.
// It is expected that the header is contained only in the red channel.
unsigned int GetPayloadSize(const bitmap_image& img) {

	char size[4];

	for (int y = 0, h = img.height(), w = img.width(), i = 0, bool run = true; y < h && run; ++y) {
		for (int x = 0; x < w && run; ++x, ++i) {
			if (i == 32) {
				run = false;
				continue;
			}
			
		}
	}
}

// Iterates over a bitmap image and fills a buffer with pixel data.
// Returns an std::pair with a pointer to the buffer and the size.
std::pair<char*, unsigned int> ParseBitmap(bitmap_image& img) {
	unsigned long long _b = 0;
	for (int y = 0, height = img.height(), width = img.width(); y < height; ++y) {
		for (int x = 0; x < width; ++x, _b += 3) {
			
		}
	}
}

//Takes in a bitmap image and displays any hidden data inside.
void ParseImage(bitmap_image& img) {

	char header[HEADER_SIZE];

	if (IsEncodedImage(img)) {
		auto data = ParseBitmap(img);

		delete[] data.first;
	}
	else {
		std::cout << "Image is not an encoded image" << std::endl;
		return;
	}

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

	//Make sure payload is large enough to store header in red channel. This is not entirely necessary
	if (image.pixel_count() < (2 + HEADER_SIZE) * 8 / 3) {
		std::wcout << "Source bitmap image isn't large enough. Please use an image with at least " << ((2 + HEADER_SIZE) * 8 / 3) << " pixels" << std::endl;
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
	info.length = len;
	write_header(buff.first, info);

	std::cout << "Buffer filled with header and payload, ready to encode" << std::endl;

	//Modify bits in the source image
	unsigned long long _b = 0;
	int nlayers = 3; //Number of significant bits to use per channel
	long pixel_count = image.pixel_count();

	for (int y = 0, height = image.height(), width = image.width(); y < height; ++y) {
		for (int x = 0; x < width; ++x, _b += 3) {
			unsigned char r, g, b;
			image.get_pixel(x, y, r, g, b);


			//Set red bit 0
			if (_b + 0 < buff.second * 8) {
				//std::cout << _b << "\t" << (_b + 0) / 8 << ", " << (_b + 0) % 8 << " = " << GetBit(buff.first[(_b + 0) / 8], (_b + 0) % 8) << std::endl;
				SetBit(r, (_b + 0) % 8, buff.first[(_b + 0) / 8]);
			}

			//Set red bit 1
			if (_b + 1 < buff.second * 8) {
				//std::cout << _b << "\t" << (_b + 1) / 8 << ", " << (_b + 1) % 8 << " = " << GetBit(buff.first[(_b + 1) / 8], (_b + 1) % 8) << std::endl;
				SetBit(r, (_b + 1) % 8, buff.first[(_b + 1) / 8]);
			}

			//Set red bit 2
			if (_b + 2 < buff.second * 8) {
				//std::cout << _b << "\t" << (_b + 2) / 8 << ", " << (_b + 2) % 8 << " = " << GetBit(buff.first[(_b + 2) / 8], (_b + 2) % 8) << std::endl;
				SetBit(r, (_b + 2) % 8, buff.first[(_b + 2) / 8]);
			}

			//########################

			//Set green bit 0
			if (_b + (pixel_count * 1 * nlayers) + 0 < buff.second * 8) {
				SetBit(g, (_b + (pixel_count * 1 * nlayers) + 0) % 8, buff.first[(_b + (pixel_count * 1 * nlayers) + 0) / 8]);
			}

			//Set green bit 1
			if (_b + (pixel_count * 1 * nlayers) + 1 < buff.second * 8) {
				SetBit(g, (_b + (pixel_count * 1 * nlayers) + 1) % 8, buff.first[(_b + (pixel_count * 1 * nlayers) + 1) / 8]);
			}

			//Set green bit 2
			if (_b + (pixel_count * 1 * nlayers) + 2 < buff.second * 8) {
				SetBit(g, (_b + (pixel_count * 1 * nlayers) + 2) % 8, buff.first[(_b + (pixel_count * 1 * nlayers) + 2) / 8]);
			}

			//########################

			//Set blue bit 0
			if (_b + (pixel_count * 2 * nlayers) + 0 < buff.second * 8) {
				SetBit(b, (_b + (pixel_count * 2 * nlayers) + 0) % 8, buff.first[(_b + (pixel_count * 2 * nlayers) + 0) / 8]);
			}

			//Set blue bit 1
			if (_b + (pixel_count * 2 * nlayers) + 1 < buff.second * 8) {
				SetBit(b, (_b + (pixel_count * 2 * nlayers) + 1) % 8, buff.first[(_b + (pixel_count * 2 * nlayers) + 1) / 8]);
			}

			//Set blue bit 2
			if (_b + (pixel_count * 2 * nlayers) + 2 < buff.second * 8) {
				SetBit(b, (_b + (pixel_count * 2 * nlayers) + 2) % 8, buff.first[(_b + (pixel_count * 2 * nlayers) + 2) / 8]);
			}


			image.set_pixel(x, y, r, g, b);

		}
	}

	std::return_temporary_buffer(buff.first);

	image.save_image("C:/temp/test_out.bmp");
	std::cout << "Output image saved" << std::endl;

	return EXIT_SUCCESS;
}