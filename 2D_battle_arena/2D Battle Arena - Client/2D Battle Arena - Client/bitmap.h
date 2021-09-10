#pragma once
#include "stdafx.h"

GLubyte * LoadDIBitmap(const char *filename, BITMAPINFO **info);
void renderBitmapCharacter(float x, float y, float z, void *font, const char *string);

void renderBitmapCharacterCenter(float x, float y, float z, void* font, const char* string);
