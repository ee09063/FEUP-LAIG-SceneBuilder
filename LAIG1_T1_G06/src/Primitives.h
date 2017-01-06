#pragma once
#ifndef PRIMITIVES_H
#define PRIMITIVES_H

#include "CGFobject.h"
#include <vector>
#include <math.h>

#define PI 3.14159265358979323846


class TriP: public CGFobject {
	public:
		void draw(vector<float>v);
};

class CylP: public CGFobject {
	public:
		void draw(vector<float>v);
};

class SphP : public CGFobject {
	public:
		void draw(vector<float>v);
};

class TorP : public CGFobject {
	public:
		void draw(vector<float>v);
};

#endif