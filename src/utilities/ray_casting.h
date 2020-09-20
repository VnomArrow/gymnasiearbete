#pragma once

namespace raycast_ {
	const double infinite = (unsigned __int64)(-1);

	struct Line {
		double x1;
		double y1;
		double x2;
		double y2;
		double k;
		double m;
		// Sets the m and k value(y = kx + m)
		inline void update() {
			if (x1 != x2) {
				k = (y1 - y2) / (x1 - x2);
				m = y1 - k * x1;
			}
			else {
				// Infinite
				k = infinite;
				m = infinite;
			}
		}
		Line(double t_x1, double t_y1, double t_x2, double t_y2) { x1 = t_x1; y1 = t_y1; x2 = t_x2; y2 = t_y2; update(); }
	};

	inline bool intersectingLines(Line* line1, Line* line2, double &intersect_x, double& intersect_y) {
		if (abs(line1->k) != abs(line2->k)) {
			// Non parallel lines

			if (line1->k != infinite && line2->k != infinite) {
				// None of the lines are pointing right up

				// Calculate where the lines intersect
				intersect_x = (line2->m - line1->m) / (line1->k - line2->k);
				intersect_y = line2->k * intersect_x + line2->m;

				// See if the intersection is on one of the lines
				if (intersect_x >= line1->x1) {
					return (intersect_x <= line1->x2);
				}
				else {
					return (intersect_x >= line1->x2);
				}
			}
			else {
				// One of the lines points right up, k = inf

				if (line1->k == infinite) {
					// The kollision will be on the line pointing ups x value
					intersect_x = line1->x1;

					// Calculate where the other line intersects that x value
					intersect_y = line2->k * intersect_x + line2->m;

					// See if the intersection is on one of the lines
					if (intersect_y >= line2->y1) {
						return (intersect_y <= line2->y2);
					}
					else {
						return (intersect_y >= line2->y2);
					}
				}
				else {
					// The x kollision will be on the line pointing ups x value
					intersect_x = line2->x1;

					// Calculate where the other line intersects that x value
					intersect_y = line1->k * intersect_x + line1->m;

					// See if the intersection is on one of the lines
					if (intersect_y >= line1->y1) {
						return (intersect_y <= line1->y2);
					}
					else {
						return (intersect_y >= line1->y2);
					}
				}
			}
		}

		// Parallel lines will never intersect
		return false;
	}

	void raycast(std::list<Line*> lines, double viewer_x, double viewer_y, double min_x, double min_y, double max_x, double max_y) {
		Line view_ray(0, 0, 0, min_y);
		double intersect_x;
		double intersect_y;

		// List to keep track of references
		struct WatchLineListRef {
			std::list<WatchLineListRef*>::iterator* it = nullptr;
			Line* line;
		};

		// The ray will start directly below the viewer (x = 0, y -> -inf) and then sweep counter clockwise

		// Line points on the right side of the viewer (dx > 0)
		std::list<std::pair<double, WatchLineListRef*>> r_start_list;
		std::list<std::pair<double, WatchLineListRef*>> r_end_list;

		// Line points on the left side of the viewer (dx < 0)
		std::list<std::pair<double, WatchLineListRef*>> l_start_list;
		std::list<std::pair<double, WatchLineListRef*>> l_end_list;

		// Lines that may block the next point
		std::list<WatchLineListRef*> watch_line_list;

		// Sort the points relative to the viewer

		std::list<Line*>::iterator line_it = lines.begin();
		while (line_it != lines.end()) {
			Line* line = *line_it;
			WatchLineListRef* line_ref = new WatchLineListRef();
			line_ref->line = line;
			if (line->x1 != viewer_x && line->x2 != viewer_x) {
				// The line does not start or end directly above or below the viewer

				double val1 = (line->y1 - viewer_y) / (line->x1 - viewer_x);
				double val2 = (line->y2 - viewer_y) / (line->x2 - viewer_x);

				if (line->x1 > viewer_x && line->x2 > viewer_x) {
					// To the right side of the viewer

					if (val1 > val2) {
						// The line ends on a smaller angle (against ray)
						r_start_list.push_back({ val2, line_ref });
						r_end_list.push_back({ val1, line_ref });
					}
					else {
						// The line ends on a bigger angle (with ray)
						r_start_list.push_back({ val1, line_ref });
						r_end_list.push_back({ val2, line_ref });
					}
				}

				else if (line->x1 < viewer_x && line->x2 < viewer_x) {
					// To the left side of the viewer

					if (val1 > val2) {
						// The line ends on a smaller angle (against ray)
						l_start_list.push_back({ val2, line_ref });
						l_end_list.push_back({ val1, line_ref });
					}

					else {
						// The line ends on a bigger angle (with ray)
						l_start_list.push_back({ val1, line_ref });
						l_end_list.push_back({ val2, line_ref });
					}
				}

				else if(intersectingLines(line, &view_ray, intersect_x, intersect_y)) {
					// The line goes directly below the viewer

					watch_line_list.push_front(line_ref);
					line_ref->it = &watch_line_list.begin();

					if (line->x1 > viewer_x) { // line->x2 < viewer_x
						// The line starts on the right side and then ends on the left side (againt ray)
						l_end_list.push_back({ val1, line_ref });
						r_start_list.push_back({ val2, line_ref });
					}

					else { // line->x1 < viewer_x && line->x2 > viewer_x
						// The line starts on the left side and then ends on the right side (with ray)
						r_end_list.push_back({ val2, line_ref });
						l_start_list.push_back({ val1, line_ref });
					}
				}
				else if (line->x1 > viewer_x) { // line->x2 < viewer_x
					// The line goes directly above the viewer
					// Starts on the right side, ends on the left side (with ray)
					r_start_list.push_back({ val1, line_ref });
					l_end_list.push_back({ val2, line_ref });
				}

				else { // line->x1 < viewer_x && line->x2 > viewer_x
					// The line goes directly above the viewer
					// Starts on the left side, ends on the right side (against ray)
					r_start_list.push_back({ val2, line_ref });
					l_end_list.push_back({ val1, line_ref });
				}
			}
			else if (line->x1 != viewer_x) {
				// The line ends directly above or below the viewer

				double val1 = (line->y1 - viewer_y) / (line->x1 - viewer_x);
				if (line->x1 > viewer_x) {
					if (line->y2 < viewer_y) {
						// The line starts to the right of the viewer and ends directly below (against ray)
						r_start_list.push_back({ -infinite, line_ref });
						r_end_list.push_back({ val1, line_ref });
					}
					else {
						// The line starts to the right of the viewer and ends directly above (with ray)
						r_end_list.push_back({ infinite, line_ref });
						r_start_list.push_back({ val1, line_ref });
					}
				}
				else {
					if (line->y2 < viewer_y) {
						// The line starts to the left of the viewer and ends directly below (with ray)
						l_end_list.push_back({ infinite, line_ref });
						l_start_list.push_back({ val1, line_ref });
					}
					else {
						// The line starts to the left of the viewer and ends directly above (against ray)
						l_start_list.push_back({ -infinite, line_ref });
						l_end_list.push_back({ val1, line_ref });
					}
				}
			}
			else if (line->x2 != viewer_x){
				// The line starts directly above or below the viewer

				double val2 = (line->y2 - viewer_y) / (line->x2 - viewer_x);
				if (line->x2 > viewer_x) {
					if (line->y1 < viewer_y) {
						// The line starts to directly below the viewer and ends to the right (with ray)
						r_start_list.push_back({ -infinite, line_ref });
						r_end_list.push_back({ val2, line_ref });
					}
					else {
						// The line starts to directly above the viewer and ends to the right (against ray)
						r_end_list.push_back({ infinite, line_ref });
						r_start_list.push_back({ val2, line_ref });
					}
				}
				else {
					if (line->y1 < viewer_y) {
						// The line starts to directly below the viewer and ends to the left (against ray)
						l_end_list.push_back({ infinite, line_ref });
						l_start_list.push_back({ val2, line_ref });
					}
					else {
						// The line starts to directly above the viewer and ends to the left (with ray)
						l_start_list.push_back({ -infinite, line_ref });
						l_end_list.push_back({ val2, line_ref });
					}
				}
			}
			else {
				// The line starts and ends directly above or below the viewer
				// A Line that points directly at the viewer cannot be seen
				delete(line_ref);
			}
			line_it++;
		}

		// Sort the lists
		r_start_list.sort();
		r_end_list.sort();

		l_start_list.sort();
		l_end_list.sort();


		std::list<std::pair<double, WatchLineListRef*>>::iterator it;

		// Iterate through the lines on the right side
		while (r_start_list.size() != 0) {
			// Get the next ref
			WatchLineListRef* start_ref = r_start_list.front().second;
			double start_val = r_start_list.front().first;

			// Check for collisions

		}

		// Iterate  through the lines on the left side

	}
}