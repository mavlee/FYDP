#include "objects.h"

float testCubeVertices[8][3] = {
	{ -50.f,  50.f,  50.f},
	{  50.f,  50.f,  50.f},
	{  50.f,  50.f, -50.f},
	{ -50.f,  50.f, -50.f},
	{ -50.f, -50.f,  50.f},
	{  50.f, -50.f,  50.f},
	{  50.f, -50.f, -50.f},
	{ -50.f, -50.f, -50.f},
};

float testCubeColours[8][3] = {
	{0.5f, 0.0f, 0.0f},
	{0.5f, 1.0f, 0.0f},
	{0.5f, 0.0f, 0.0f},
	{0.5f, 0.0f, 1.0f},
	{0.5f, 1.0f, 1.0f},
	{0.0f, 0.0f, 0.9f},
	{1.0f, 0.8f, 0.7f},
	{1.0f, 0.0f, 0.0f},
};

int testCubeFaces[6][4] = {
	{0, 1, 2, 3},
	{0, 3, 7, 4},
	{3, 2, 6, 7},
	{2, 1, 5, 6},
	{0, 4, 5, 1},
	{4, 7, 6, 5},
};

Cube testCube;

void initCube() {
	int i;
	int j;

	for (i = 0; i < 8; i++) {
		for (j = 0; j < 3; j++) {
			testCube.ver[i].pos[j] = testCubeVertices[i][j];
			testCube.ver[i].col[j] = testCubeColours[i][j];
		}
	}

	for (i = 0; i < 6; i++) {
		for (j = 0; j < 4; j++) {
			testCube.face[i].ver[j] = testCubeFaces[i][j];
		}
	}
}

Cube getCube() {
	return testCube;
}