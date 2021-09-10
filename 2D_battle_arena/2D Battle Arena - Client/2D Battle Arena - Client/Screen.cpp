#include "Screen.h"



Screen::Screen()
{
	//m_Renderer = new Renderer(ScreenSizeX, ScreenSizeY);
}

Screen::~Screen()
{
}

void Screen::DrawRect(float x, float y, float sizeX, float sizeY, float r, float g, float b, float a)
{
	glColor4f(r, g, b, a);
	glRectf(x - sizeX / 2, y - sizeY / 2, x + sizeX / 2, y + sizeY / 2);
}

void Screen::DrawTextureRect(float x, float y, float sizeX, float sizeY, GLuint texture)
{
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glBegin(GL_QUADS);
	{
		glTexCoord2f(0.0f, 1.0f);
		glVertex3f(x - sizeX / 2, y - sizeY / 2, 0);
		glTexCoord2f(0.0f, 0.0f);
		glVertex3f(x - sizeX / 2, y + sizeY / 2, 0);
		glTexCoord2f(1.0f, 0.0f);
		glVertex3f(x + sizeX / 2, y + sizeY / 2, 0);
		glTexCoord2f(1.0f, 1.0f);
		glVertex3f(x + sizeX / 2, y - sizeY / 2, 0);
	}
	glEnd();
	glDisable(GL_TEXTURE_2D);
}

