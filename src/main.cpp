#include <iostream>
#include "mandelbrot_generator.h"
#include "simulator/Simulator.h"
#include "simulator/Vec4.h"

int wmain() {
	Vec4 vec = Vec4(3, 3, 0, 0);
	Simulator simulator = Simulator(vec, 100, 100, 100, 100, 10);
	DisplayWindow::init(800, 800);
	while (DisplayWindow::isRunning()) {
		simulator.iterateSnapshot(0.01);
		DisplayWindow::fill(0, 0, 0, 255);
		DisplayWindow::drawRectangle(400, 400, 20, 20, 0, 0, 255, 255);
		DisplayWindow::drawRectangle(400 - simulator.getx1(), 400 - simulator.gety1(), 20, 20, 255, 0, 0, 255);
		DisplayWindow::drawRectangle(400 - simulator.getx2(), 400 - simulator.gety2(), 20, 20, 0, 255, 0, 255);
		DisplayWindow::flip();
		DisplayWindow::update();
	}


	//mandelbrot_main();
	// Print "hello world!"
	std::cout << "hello world!" << std::endl;

	// To not close the window immediately
	std::cin.get();

	return 0;
}