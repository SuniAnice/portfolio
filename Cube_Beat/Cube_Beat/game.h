#pragma once
#include <GL/glut.h>
#include <stdio.h>
#include <fmod.hpp>
#include <iostream>
#include <time.h>
#include "jsoncpp\json\json.h"
#include "bitmap.h"


using namespace FMOD;
using namespace Json;
using namespace std;

typedef struct {
	int y;
	int line;
	int valid;
}NODE;

void DrawCube(int Cubesize);

void dropnote(int value);

void rotatetimer(int value);

void initgame();

void draw_game();

void RotateX(int value);

void RotateY(int value);

void RotateZ(int value);

void setortho();

void setperspective();

void hiton(int value);
void hitoff(int value);

