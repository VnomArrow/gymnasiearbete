#pragma once

#include "simulator/precisionType.h"

class Vec4;
class Simulator;

class FlipTest {
public:

	FlipTest(Simulator sim, float dt);
	
	bool flips();

	Simulator getSim();

private:

	Simulator* sim;
	float dt;

};
