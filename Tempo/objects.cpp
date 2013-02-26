#include "LOpenGL.h"
#include "objects.h"
#include "constants.h"

float cubeColours[1][8][3] = {
	{
		{1.0f, 1.0f, 0.0f},
		{1.0f, 0.0f, 0.0f},
		{1.0f, 0.0f, 1.0f},
		{0.0f, 0.0f, 1.0f},
		{0.0f, 1.0f, 1.0f},
		{0.0f, 1.0f, 0.0f},
		{1.0f, 1.0f, 0.0f},
		{1.0f, 0.0f, 0.0f},
	},
};

int cubeFaces[6][4] = {
	{0, 1, 2, 3},
	{0, 3, 7, 4},
	{3, 2, 6, 7},
	{2, 1, 5, 6},
	{0, 4, 5, 1},
	{4, 7, 6, 5},
};

void Object::setCentre(float x, float y, float z) {
	centre.x = x;
	centre.y = y;
	centre.z = z;
}

Cube::Cube(float centreX, float centreY, float centreZ,
				float width, float height, float depth, ColourSet color) {
	float vertices[8][3] = {
		{ centreX - width / 2, centreY + height / 2, centreZ + depth / 2 },
		{ centreX + width / 2, centreY + height / 2, centreZ + depth / 2 },
		{ centreX + width / 2, centreY + height / 2, centreZ - depth / 2 },
		{ centreX - width / 2, centreY + height / 2, centreZ - depth / 2 },
		{ centreX - width / 2, centreY - height / 2, centreZ + depth / 2 },
		{ centreX + width / 2, centreY - height / 2, centreZ + depth / 2 },
		{ centreX + width / 2, centreY - height / 2, centreZ - depth / 2 },
		{ centreX - width / 2, centreY - height / 2, centreZ - depth / 2 },
	};

	int i, j;
	for (i = 0; i < 8; i++) {
		for (j = 0; j < 3; j++) {
			ver[i].pos[j] = vertices[i][j];
			ver[i].col[j] = cubeColours[color][i][j];
		}
	}

	for (i = 0; i < 6; i++) {
		for (j = 0; j < 4; j++) {
			face[i].ver[j] = cubeFaces[i][j];
		}
	}
}

void Cube::draw() {
	int i, j;
	int currentVer;

	// Render the cube
	glBegin( GL_QUADS );
	for (i = 0; i < nFaces; i++) {
		for (j = 0; j < 4; j++) {
			currentVer = face[i].ver[j];
			glColor3fv(ver[currentVer].col);
			glVertex3fv(ver[currentVer].pos);
		}
	}
	glEnd();
}
