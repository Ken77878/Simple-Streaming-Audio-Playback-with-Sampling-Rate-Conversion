
#include "audioTest1.h"
#include <cstdio>
#include <iostream>

int main() {
#if defined(WIN32) && defined(UNICODE)
	setlocale(LC_ALL, setlocale(LC_CTYPE, ""));
#endif

	try {
		if (audioTest1())
			return 1;
	}
	catch (std::bad_alloc &e) {
		std::cout << e.what() << std::endl;
	}   

	return 0;
}
