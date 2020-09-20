#include <iostream>
#include "mandelbrot_generator.h"
#include "simulator/Simulator.h"
#include "simulator/Vec4.h"
#include "search.h"
#include "FlipTest.h"

int wmain() {
	DisplayWindow::init(800, 800);
	/*Grid my_grid = Grid(100, 100);
	SearchPosition pos;
	pos.x = my_grid.width / 2;
	pos.y = my_grid.height / 2;
	my_grid.addSearchPosition(pos);
	SearchPosition recv_pos;

	while (my_grid.getUnsearchedPos(recv_pos) && DisplayWindow::isRunning()) {
		//DisplayWindow::fill(0, 0, 0, 255);
		my_grid.addSearchPosition(recv_pos);
		my_grid.setState(recv_pos, Grid::status_is_true);
		DisplayWindow::drawRectangle(recv_pos.x*4, recv_pos.y*4, 4, 4, 0, 0, 255, 255);
		//std::cout << "X: " << recv_pos.x << "  Y: " << recv_pos.y << std::endl;
		DisplayWindow::flip();
		DisplayWindow::update();
		while (true) {

		}
	}*/

	Vec4 vec = Vec4(3, 4, 0, 0);
	Simulator simulator = Simulator(vec, 100, 100, 100, 100, 10);
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

	return 0;
}