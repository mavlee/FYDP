#include <list>

#ifndef OBJECT_H
#define OBJECT_H

class Ver {
	public:
		float pos[3];
		float col[3];
};

class Object {
	public:
		// defines the number of vertices per face.
		static const int nFaces;

		virtual bool draw() = 0;
};

class Cube: public Object {
	public:
		static const int nFaces = 6;
		Ver ver[8];

		// defines which vertices should be linked to create a face
		struct {
			unsigned int ver[4];
		} face[6];

		bool draw();
};

void initCube();

Cube getCube();

std::list<Object*> getObstacles();

bool drawObstacles();

#endif
