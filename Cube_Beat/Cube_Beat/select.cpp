#include "select.h"


using namespace FMOD;

extern System* soundSystem;
extern Sound* sounds[4];
extern Channel* channel[2];

extern Json::Value root;
extern Json::Value Highscore;



GLuint object[5];
GLubyte *sel_pBytes;
BITMAPINFO *sel_info;

GLuint rank[4];

extern int select;

extern float volume;
char str[100];

bool se_first = 0;

void se_init()
{
	glGenTextures(root["musicnum"].asInt(), object);
	glGenTextures(4, rank);
	se_first = 1;
	for (int i = 0; i < root["musicnum"].asInt(); i++)
	{
		glBindTexture(GL_TEXTURE_2D, object[i]);
		sel_pBytes = LoadDIBitmap(root["Music"][i]["Imagepath"].asCString(), &sel_info);
		glTexImage2D(GL_TEXTURE_2D, 0, 3, 480, 360, 0, GL_BGR_EXT,
			GL_UNSIGNED_BYTE, sel_pBytes);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	}
	for (int i = 0; i < 4; i++)
	{
		glBindTexture(GL_TEXTURE_2D, rank[i]);
		sel_pBytes = LoadDIBitmap(Highscore["Rankimg"][i].asCString(), &sel_info);
		glTexImage2D(GL_TEXTURE_2D, 0, 3, 100, 100, 0, GL_BGR_EXT,
			GL_UNSIGNED_BYTE, sel_pBytes);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	}
}

void draw_select()
{
	if (se_first == 0)
	{
		se_init();
	}
	
	
	

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(400.0, -400.0, -300.0, 300.0, -5000.0, 5000.0);
	glMatrixMode(GL_MODELVIEW);
	glColor4f(1, 1, 1, 1);

	for (int i = 0; i < root["musicnum"].asInt(); i++) 
	{
		
		glColor4d(1, 1, 1, 1);
		glBegin(GL_QUADS);
		glVertex3d(-400, 300-175*i, 0);
		glVertex3d(-400, 150-175*i, 0);
		glVertex3d(400, 150-175*i, 0);
		glVertex3d(400, 300-175*i, 0);
		glEnd();
		
		

		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, object[i]);
		glBegin(GL_QUADS);

		glTexCoord2f(0.0f, 1.0f);
		glVertex3d(-380, 290 - 175 * i, -10);
		glTexCoord2f(0.0f, 0.0f);
		glVertex3d(-380, 160 - 175 * i, -10);
		glTexCoord2f(1.0f, 0.0f);
		glVertex3d(-180, 160 - 175 * i, -10);
		glTexCoord2f(1.0f, 1.0f);
		glVertex3d(-180, 290 - 175 * i, -10);

		glEnd();
		glDisable(GL_TEXTURE_2D);

		glColor4d(1, 0, 0, 1);
		glLineWidth(5);
		if (select == i)
		{
			glBegin(GL_LINE_LOOP);

			glVertex3d(-380, 290 - 175 * i, -10);
			glVertex3d(-380, 160 - 175 * i, -10);
			glVertex3d(-180, 160 - 175 * i, -10);
			glVertex3d(-180, 290 - 175 * i, -10);

			glEnd();
		}
		glColor4d(0, 0, 0, 1);
		renderBitmapCharacter(-160, 265 - 175 * i, -20, GLUT_BITMAP_TIMES_ROMAN_24, root["Music"][i]["Musicname"].asCString());
		renderBitmapCharacter(-160, 235 - 175 * i, -20, GLUT_BITMAP_9_BY_15, root["Music"][i]["Compname"].asCString());
		
		glColor4d(1, 0, 0, 1);
		renderBitmapCharacter(-160, 165 - 175 * i, -20, GLUT_BITMAP_9_BY_15, "highscore : ");
		
		
		itoa(Highscore["Score"][i].asInt(), str, 10);
		renderBitmapCharacter(20, 165 - 175 * i, -20, GLUT_BITMAP_9_BY_15, str);


		glColor4d(1, 1, 1, 1);
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, rank[Highscore["Rank"][i].asInt()]);

		glBegin(GL_QUADS);


		glTexCoord2f(0.0f, 1.0f);
		glVertex3d(180, 290 - 175 * i, -10);
		glTexCoord2f(0.0f, 0.0f);
		glVertex3d(180, 160 - 175 * i, -10);
		glTexCoord2f(1.0f, 0.0f);
		glVertex3d(380, 160 - 175 * i, -10);
		glTexCoord2f(1.0f, 1.0f);
		glVertex3d(380, 290 - 175 * i, -10);

		glEnd();
		glDisable(GL_TEXTURE_2D);
	}
}