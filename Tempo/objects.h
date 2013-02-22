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

		void setCentre(float x, float y, float z);

		// Should be 0.f by default
		float shiftZ;

	private:
		struct {
			float x;
			float y;
			float z;
		} centre;
};

class Cube: public Object {
	public:
		enum ColourSet {
			Multi = 0,
		};

		Cube(float centreX, float centreY, float centreZ,
				float width, float height, float depth, ColourSet color);

		static const int nFaces = 6;
		Ver ver[8];

		// defines which vertices should be linked to create a face
		struct {
			unsigned int ver[4];
		} face[6];

		bool draw();

	private:
		float width;
		float height;
		float depth;
};

void initCube();

Cube* getCube();

std::list<Object*> getObstacles();

bool drawObstacles();

#endif
