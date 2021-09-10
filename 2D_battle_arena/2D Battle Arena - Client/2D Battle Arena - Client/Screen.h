#pragma once

#include "Renderer.h"
#include "NGPGlobals.h"



class Screen
{
public:
	Screen();
	~Screen();

	virtual void Update(float time, NGPInputs* inputs) = 0;
	virtual void RenderScene() = 0;

	void DrawRect(float x, float y, float sizeX, float sizeY, float r, float g, float b, float a);

	void DrawTextureRect(float x, float y, float sizeX, float sizeY, GLuint texture);

	int check = 0;		// title = 0, game = 1

	int ScreenNo = 0;

protected:
	Renderer* m_Renderer = NULL;
};

