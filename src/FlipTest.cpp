
#include "FlipTest.h"
#include "simulator/Simulator.h"
#include "simulator/precisionType.h"
#include "simulator/Vec4.h"
#include "utilities/math_functions.h"

FlipTest::FlipTest(Simulator sim, float dt) {
	this->sim = new Simulator(
		sim.getState(), sim.getm1(), sim.getm2(), sim.getl1(), sim.getl2(), sim.getg());
	this->dt = dt;
}

bool FlipTest::flips() {
	//PREDEC prevA = sim->getState().getx();
	PREDEC prevB = sim->getState().gety();
	
	sim->iterateSnapshot(dt);

	//PREDEC currentA = sim->getState().getx();
	PREDEC currentB = sim->getState().gety();

	if ((currentB < PI && 
		prevB > PI && 
		currentB < 5 && 
		prevB < 5) 
		|| 
		(currentB > PI && 
			prevB < PI && 
			currentB < 5 && 
			prevB < 5))
		return true;
	return false;
}

Simulator FlipTest::getSim() {
	return *sim;
}
