#include <cstring>
#include <iostream>

#include "Buffer.h"

int main()
{
	Buffer<int,3> buf;

	buf[0] = 10;
	buf[1] = 11;
	buf[2] = 12;

	int item1 = *buf;

	std::cout << "item 1 = " << item1
		<< std::endl;

	std::cout << "buf[0] = " << buf[0]
		<< std::endl;
	std::cout << "buf[1] = " << buf[1]
		<< std::endl;
	std::cout << "buf[2] = " << buf[2]
		<< std::endl;

	int* ptr = buf+2;

	std::cout << "*ptr   = " << *ptr
		<< std::endl;

	Buffer<int,2,3,4> buf2;
	buf2[0][1][2] = 500;

	buf2[2][0][0]; // should given an error
	buf2[0][3][0]; // should given an error
	buf2[0][0][5]; // should given an error

	std::cout << "value = "
		<< buf2[0][1][2] << std::endl;

	/*
	 * Testing out type conversions
	 */
	Buffer<uint64,3> dest;
	Buffer<uint64,3> orig;

	dest[0] = 1;
	dest[1] = 2;
	dest[2] = 3;

	orig[0] = 4;
	orig[1] = 5;
	orig[2] = 6;

	std::printf("dest[0] = %llu\n", dest[0]);
	std::printf("dest[1] = %llu\n", dest[1]);
	std::printf("dest[2] = %llu\n", dest[2]);
	std::printf("\n");

	std::memcpy(dest,
		orig, sizeof(uint64) * 3);

	std::printf("dest[0] = %llu\n", dest[0]);
	std::printf("dest[1] = %llu\n", dest[1]);
	std::printf("dest[2] = %llu\n", dest[2]);
	std::fflush(stdout);

	return 0;
}
