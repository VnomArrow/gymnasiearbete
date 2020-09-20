#include <iostream>
#include "mandelbrot_generator.h"

int wmain() {
	mandelbrot_main();
	// Print "hello world!"
	std::cout << "hello world!" << std::endl;

	// To not close the window immediately
	std::cin.get();

	return 0;
}