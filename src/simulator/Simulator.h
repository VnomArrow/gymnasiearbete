#pragma once

#include "precisionType.h"

class Vec4;

class Simulator {
public:
	Simulator(Vec4 state, PREDEC m1, PREDEC m2, PREDEC l1, PREDEC l2, PREDEC g);

	void iterateSnapshot(float dt);

	float getx1();
	float gety1();

	float getx2();
	float gety2();

private:
	Vec4* state;
	PREDEC m1, m2, l1, l2, g;

	Vec4 getStateChange(Vec4 state);

	PREDEC A1();
	PREDEC B1(PREDEC a, PREDEC b);
	PREDEC C1(PREDEC a, PREDEC b, PREDEC da, PREDEC db);

	PREDEC A2(PREDEC a, PREDEC b);
	PREDEC B2();
	PREDEC C2(PREDEC a, PREDEC b, PREDEC da, PREDEC db);

};