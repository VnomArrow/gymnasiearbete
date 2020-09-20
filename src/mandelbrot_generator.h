#pragma once
#include <vector>
#include "utilities/display.h"
#include "utilities/time_functions.h"

class Grid2D {
public:
	const unsigned int& size_x = m_size_x;
	const unsigned int& size_y = m_size_y;
	unsigned char* getPos(unsigned __int64 x, unsigned __int64 y) {
		return(&arr[x + y * size_x]);
	}
	Grid2D(int t_size_x, int t_size_y) :
		m_size_x(t_size_x), m_size_y(t_size_y) {
		arr = {};
		arr.resize((unsigned __int64)t_size_x * t_size_y);
	}
private:
	unsigned int m_size_x = 0;
	unsigned int m_size_y = 0;
	std::vector<unsigned char> arr;
};

inline void generateMandelbrotSet(double start_x,
	double start_y,
	double size_x,
	double size_y,
	Grid2D* grid) {

	for (int x = 0; x < grid->size_x; x++) {
		for (int y = 0; y < grid->size_y; y++) {
			// The real and imaginary value of C
			double c_r = ((size_x) * x) / grid->size_x + start_x;
			double c_i = ((size_y) * y) / grid->size_y + start_y;

			// The current position
			double val_r = 0;
			double val_i = 0;

			// Iterate 256 times
			for (int count = 1; count < 256; ++count) {
				// Save the old position
				double prev_r = val_r;
				double prev_i = val_i;

				// Calculate the new position using the old position
				val_r = prev_r * prev_r - prev_i * prev_i + c_r;
				val_i = 2 * prev_r * prev_i + c_i;

				// See if the value is outside range
				if (abs(val_r) > 100 || abs(val_i) > 100) {
					*grid->getPos(x, y) = count;
					goto exit_loop;
				}
			}
			// The value stayed inside range, set it to 0 for stable
			*grid->getPos(x, y) = 0;
			// Label for exit
			exit_loop:
			continue;
		}
	}
}

inline void mandelbrot_main() {
	double centre_x = 0;
	double centre_y = 0;
	double zoom = 4;
	int gridsize = 800;
	int pixel_size = 1;
	DisplayWindow::init(800, 800);
	while (DisplayWindow::isRunning()) {
		Grid2D my_grid(gridsize, gridsize);
		generateMandelbrotSet(centre_x - zoom / 2, centre_y - zoom / 2, zoom, zoom, &my_grid);
		for (int x = 0; x < gridsize; x++) {
			for (int y = 0; y < gridsize; y++) {
				unsigned __int64 color = (__int64)*my_grid.getPos(x, y) * 5;
				if (color > 255) {
					color = 255;
				}
				DisplayWindow::drawRectangle(x * pixel_size, y * pixel_size, pixel_size, pixel_size,
					color, color, color, 255);
			}
		}
		DisplayWindow::flip();
		std::cout << "done!" << std::endl;
		while (DisplayWindow::isRunning()) {
			double x = (DisplayWindow::getMouseXPos() * zoom) / 800 + centre_x - zoom / 2;
			double y = (DisplayWindow::getMouseYPos() * zoom) / 800 + centre_y - zoom / 2;
			std::cout << "x: " << x << "  y: " << y << "  zoom: " << zoom << std::endl;
			DisplayWindow::update();
			const Uint8* state = DisplayWindow::getKeyboardState();
			if (state[SDL_SCANCODE_W]) {
				sleep(10);
				DisplayWindow::update();
				centre_x = x;
				centre_y = y;
				zoom = zoom / 2;
				break;
			}
			else if (state[SDL_SCANCODE_B]) {
				sleep(10);
				DisplayWindow::update();
				centre_x = x;
				centre_y = y;
				zoom = zoom * 2;
				break;
			}
		}
	}
}