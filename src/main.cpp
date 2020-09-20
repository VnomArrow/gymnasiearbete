#include <iostream>
#include "mandelbrot_generator.h"
#include "simulator/Simulator.h"
#include "simulator/Vec4.h"
#include "FlipTest.h"

int wmain() {
	Vec4 vec = Vec4(3, 3, 0, 0);
	Simulator simulator = Simulator(vec, 100, 100, 100, 100, 10);
	FlipTest ft = FlipTest(simulator, 0.009);
	DisplayWindow::init(800, 800);
	while (DisplayWindow::isRunning()) {
		//simulator.iterateSnapshot(0.01);
		if (ft.flips())
			std::cout << "FLIPPAR!!!!!!" << std::endl;
		DisplayWindow::fill(0, 0, 0, 255);
		DisplayWindow::drawRectangle(400, 400, 20, 20, 0, 0, 255, 255);
		DisplayWindow::drawRectangle(400 - ft.getSim().getx1(), 400 - ft.getSim().gety1(), 20, 20, 255, 0, 0, 255);
		DisplayWindow::drawRectangle(400 - ft.getSim().getx2(), 400 - ft.getSim().gety2(), 20, 20, 0, 255, 0, 255);
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