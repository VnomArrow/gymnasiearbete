#include <iostream>
#include "mandelbrot_generator.h"
#include "simulator/Simulator.h"
#include "simulator/Vec4.h"
#include "search.h"
#include "FlipTest.h"
#include "utilities/math_functions.h"

int wmain() {
	// Initialize the display window
	DisplayWindow::init(800, 800);

	// Create a grid to keep track of which positions that have been searched
	//  and to keep track of which positions that should be searched
	Grid my_grid = Grid(800, 800);

	// Position that the grid will grow from
	SearchPosition pos;
	pos.x = my_grid.width / 2;
	pos.y = my_grid.height / 2;
	my_grid.addSearchPosition(pos);

	// Variable to store the last position that wants to be searched
	SearchPosition recv_pos;

	// Make the display white ((red, green, blue, opacity) where 255 is max)
	DisplayWindow::fill(255, 255, 255, 255);

	// Do while the display window has not been closed and there are positions to search
	while (my_grid.getUnsearchedPos(recv_pos) && DisplayWindow::isRunning()) {
		// Calculate the start angle the position it corresponds to
		double angle_x = (((recv_pos.x + my_grid.width / 2) % my_grid.width) * 2 * PI) / my_grid.width;
		double angle_y = (((recv_pos.y + my_grid.height / 2) % my_grid.height) * 2 * PI) / my_grid.height;
		
		// Setup a vector to store the start position
		Vec4 vec = Vec4(angle_x, angle_y, 0, 0);

		// Setup the simulation
		Simulator simulator = Simulator(vec, 100, 100, 100, 100, 10);
		FlipTest ft(simulator, 0.01);

		// Simulate over 100 seconds
		for (int i = 0; i < 10000; i++) {
			if (ft.flips()) {
				// Exit if the pendulum flips over
				goto flip_detected_label;
			}
		}

		// Draw a rectangle on that position
		DisplayWindow::drawRectangle(recv_pos.x, recv_pos.y, 2, 2, 0, 0, 0, 255);
		
		// Update the screen
		DisplayWindow::flip();
		DisplayWindow::update();
	flip_detected_label:
		// Set the position to already searched and add new positions
		my_grid.setState(recv_pos, Grid::status_is_true);
		my_grid.addSearchPosition(recv_pos);
	}

	// Keep the window alive
	while (DisplayWindow::isRunning()) {
		DisplayWindow::update();
	}

	return 0;
}