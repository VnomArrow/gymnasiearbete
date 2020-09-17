#pragma once

#include "precisionType.h"

class Vec4 {
public:
	Vec4(PREDEC x, PREDEC y, PREDEC z, PREDEC w);
	void update(PREDEC x, PREDEC y, PREDEC z, PREDEC w);

	Vec4 add(Vec4 vec);
	Vec4 mul(PREDEC m);

	PREDEC getx();
	PREDEC gety();
	PREDEC getz();
	PREDEC getw();

private:
	PREDEC x, y, z, w;

};
