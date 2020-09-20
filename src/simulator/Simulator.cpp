
#include <iostream>
#include <math.h>
#include <algorithm>

#include "Simulator.h"
#include "Vec4.h"
#include "precisionType.h"

Simulator::Simulator(Vec4 state, PREDEC m1, PREDEC m2, PREDEC l1, PREDEC l2, PREDEC g) {
	this->state = new Vec4(state.getx(), state.gety(), state.getz(), state.getw());

	this->m1 = m1;
	this->m2 = m2;
	this->l1 = l1;
	this->l2 = l2;
	this->g = g;
}

void Simulator::iterateSnapshot(float dt) {
	Vec4 k1 = getStateChange(*this->state);
	Vec4 k2 = getStateChange(this->state->add(k1.mul(dt / 2)));
	Vec4 k3 = getStateChange(this->state->add(k2.mul(dt / 2)));
	Vec4 k4 = getStateChange(this->state->add(k3.mul(dt)));

	Vec4 stateChange = k1.add(k2.mul(2).add(k3.mul(2).add(k4))).mul(dt / 6);

	PREDEC a = PREfmod(PREfmod(state->getx() + stateChange.getx(), 6.28318530718) + 6.28318530718, 6.28318530718);
	PREDEC b = PREfmod(PREfmod(state->gety() + stateChange.gety(), 6.28318530718) + 6.28318530718, 6.28318530718);
	PREDEC da = state->getz() + stateChange.getz();
	PREDEC db = state->getw() + stateChange.getw();

	this->state->update(a, b, da, db);
}

Vec4 Simulator::getState() {
	return Vec4(state->getx(), state->gety(), state->getz(), state->getw());
}
PREDEC Simulator::getm1() {
	return m1;
}
PREDEC Simulator::getm2() {
	return m2;
}
PREDEC Simulator::getl1() {
	return l1;
}
PREDEC Simulator::getl2() {
	return l2;
}
PREDEC Simulator::getg() {
	return g;
}

float Simulator::getx1() {
	return -((float) l1 * std::sin((float) state->getx()));
}
float Simulator::gety1() {
	return -((float) l1 * std::cos((float) state->getx()));
}

float Simulator::getx2() {
	return -((float) l1 * std::sin((float) state->getx()))
		- ((float) l2 * std::sin((float) state->gety()));
}
float Simulator::gety2() {
	return -((float) l1 * std::cos((float) state->getx()))
		- ((float) l2 * std::cos((float) state->gety()));
}

Vec4 Simulator::getStateChange(Vec4 state) {
	PREDEC a1 = A1();
	PREDEC b1 = B1(state.getx(), state.gety());
	PREDEC c1 = C1(state.getx(), state.gety(), state.getz(), state.getw());

	PREDEC a2 = A2(state.getx(), state.gety());
	PREDEC b2 = B2();
	PREDEC c2 = C2(state.getx(), state.gety(), state.getz(), state.getw());

	PREDEC ddb = (a2 * c1 - a1 * c2) / (a2 * b1 - a1 * b2);
	PREDEC dda = (c1 - b1 * ddb) / a1;

	return Vec4(state.getz(), state.getw(), dda, ddb);
}

PREDEC Simulator::A1() {
	return (m1 + m2) * l1;
}
PREDEC Simulator::B1(PREDEC a, PREDEC b) {
	return m2 * l2 * PREcos(a - b);
}
PREDEC Simulator::C1(PREDEC a, PREDEC b, PREDEC da, PREDEC db) {
	return -(db * db * l2 * m2 * PREsin(a - b)) - (g * (m1 + m2)) * PREsin(a);
}

PREDEC Simulator::A2(PREDEC a, PREDEC b) {
	return m2 * l1 * PREcos(a - b);
}
PREDEC Simulator::B2() {
	return m2 * l2;
}
PREDEC Simulator::C2(PREDEC a, PREDEC b, PREDEC da, PREDEC db) {
	return (da * da * l1 * m2 * PREsin(a - b)) - (g * m2) * PREsin(b);
}
