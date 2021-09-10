#pragma once
#include <GL/glut.h>
#include <stdio.h>
GLubyte * LoadDIBitmap(const char *filename, BITMAPINFO **info);
void renderBitmapCharacter(float x, float y, float z, void *font, const char *string);