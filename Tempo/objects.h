#include <list>

#ifndef OBJECT_H
#define OBJECT_H

#include "cubeConstants.h"

class Vertex {
	public:
		float pos[3];
		float col[3];
};

class Object {
	public:
		// defines the number of vertices per face.
		static const int nFaces;

		virtual void draw() = 0;

		void setCentre(float x, float y, float z);

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
			NO_COLOUR = 0,
			C1 = 1,
			C2 = 2,
			C3 = 3,
            C4 = 4,
		};

		// used for collision detection
		float zFront;
		float zBack;
		float wLeft;
		float wRight;

		Cube(int x, int y, float centreZ,
				float width, float height, float depth, ColourSet color);

		static const int nFaces = 6;
		Vertex ver[8];

		// defines which vertices should be linked to create a face
		struct {
			unsigned int ver[4];
		} face[6];

		void draw();

        void draw(GLuint* texture);

		bool collided;

        ColourSet colour;

	private:
		float width;
		float height;
		float depth;
};

#endif
