#include <iostream>
#include <list>
#include <vector>
#include <math.h>

struct Grid;

struct SearchPosition {
	int x = 0;
	int y = 0;
	SearchPosition() {}
	SearchPosition(int t_x, int t_y) :
		x(t_x), y(t_y) {}
};

struct Grid {
	enum {
		status_unsearched,
		status_pending,
		status_is_true,
		status_is_false,
	};
	Grid(int t_width, int t_height) :
		m_width(t_width), m_height(t_height) {
		grid_array.resize(width * height);
	}
	char* getPos(int x, int y) {
		return(&grid_array[x + y * width]);
	}
	void setState(SearchPosition& pos, char state) {
		*getPos(pos.x, pos.y) = state;
	}
	const int& width = m_width;
	const int& height = m_height;
	std::vector<char> grid_array;
	std::list<SearchPosition> search_list;
	bool getUnsearchedPos(SearchPosition& search_pos_ref) {
		if (search_list.size()) {
			search_pos_ref = search_list.front();
			search_list.pop_front();
			return true;
		}
		return false;
	}
	// See if a position is valid
	bool isValidX(int pos_x) {
		return (pos_x >= 0 && pos_x < width);
	}
	bool isValidY(int pos_y) {
		return (pos_y >= 0 && pos_y < height);
	}
	// Add position and surrounding positions to search
	void addSearchPosition(const SearchPosition& t_pos) {
		// Centre
		SearchPosition pos_new(t_pos.x, t_pos.y);
		if (*getPos(pos_new.x, pos_new.y) == status_unsearched) {
			*getPos(pos_new.x, pos_new.y) = status_pending;
			search_list.push_back(pos_new);
		}
		// Right
		pos_new.x = t_pos.x + 1;
		pos_new.y = t_pos.y;
		if (isValidX(pos_new.x) && *getPos(pos_new.x, pos_new.y) == status_unsearched) {
			*getPos(pos_new.x, pos_new.y) = status_pending;
			search_list.push_back(pos_new);
		}

		// Left
		pos_new.x = t_pos.x - 1;
		pos_new.y = t_pos.y;
		if (isValidX(pos_new.x) && *getPos(pos_new.x, pos_new.y) == status_unsearched) {
			*getPos(pos_new.x, pos_new.y) = status_pending;
			search_list.push_back(pos_new);
		}

		// Above
		pos_new.x = t_pos.x;
		pos_new.y = t_pos.y - 1;
		if (isValidY(pos_new.y) && *getPos(pos_new.x, pos_new.y) == status_unsearched) {
			*getPos(pos_new.x, pos_new.y) = status_pending;
			search_list.push_back(pos_new);
		}

		// Below
		pos_new.x = t_pos.x;
		pos_new.y = t_pos.y + 1;
		if (isValidY(pos_new.y) && *getPos(pos_new.x, pos_new.y) == status_unsearched) {
			*getPos(pos_new.x, pos_new.y) = status_pending;
			search_list.push_back(pos_new);
		}

	}
protected:
	int m_width;
	int m_height;
};

void setPositionsInsideCircle() {
	double x;
	double y;

}

int search_main() {
	// Create a 10x10 grid
	Grid my_grid = Grid(100, 100);
	SearchPosition pos;
	pos.x = 5;
	pos.y = 5;
	my_grid.addSearchPosition(pos);
	SearchPosition recv_pos;
	while (my_grid.getUnsearchedPos(recv_pos)) {
		my_grid.addSearchPosition(recv_pos);
		my_grid.setState(recv_pos, Grid::status_is_true);
		std::cout << "X: " << recv_pos.x << "  Y: " << recv_pos.y << std::endl;
	}
	return 0;

}