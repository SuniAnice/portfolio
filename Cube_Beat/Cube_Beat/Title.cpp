#include "Title.h"

using namespace FMOD;

extern System* soundSystem;
extern Sound* title;
extern Channel* channel[2];

GLuint texture_object[2];
GLubyte *pBytes;
BITMAPINFO *info;

extern float volume;






bool init = 0;



void draw_title()
{
	
	if (init == 0)
	{
		glGenTextures(1, texture_object);

		glBindTexture(GL_TEXTURE_2D, texture_object[0]);
		pBytes = LoadDIBitmap("../Resource/title.bmp", &info);
		glTexImage2D(GL_TEXTURE_2D, 0, 3, 400, 300, 0, GL_BGR_EXT,
			GL_UNSIGNED_BYTE, pBytes);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		soundSystem->playSound(title, NULL, 0, &channel[0]);
		channel[0]->setVolume(volume);
		init = 1;
	}
	

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(400.0, -400.0, -300.0, 300.0, -5000.0, 5000.0);
	glMatrixMode(GL_MODELVIEW);
	glColor4f(1, 1, 1,1);

	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, texture_object[0]);
	glBegin(GL_QUADS);
	{
		glTexCoord2f(0.0f, 1.0f);
		glVertex3d(-400, 300, 0);
		glTexCoord2f(0.0f, 0.0f);
		glVertex3d(-400, -300, 0);
		glTexCoord2f(1.0f, 0.0f);
		glVertex3d(400, -300, 0);
		glTexCoord2f(1.0f, 1.0f);
		glVertex3d(400, 300, 0);
	}
	glEnd();
	glDisable(GL_TEXTURE_2D);
}