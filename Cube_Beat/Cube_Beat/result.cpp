#include "result.h"


extern System* soundSystem;
extern Sound* resultbgm[2];
extern Channel* channel[2];

extern Value root;
extern Value Highscore;

extern int select;

extern float volume;

extern int perfact;
extern int great;
extern int good;
extern int miss;
extern int score;
extern int combo;

GLuint resultscreen[2];
GLubyte *re_pBytes;
BITMAPINFO *re_info;

extern int notenum;

bool first_re = 0;
int re_rank = 0;
extern GLuint rank[4];
extern GLuint object[5];
extern GLubyte *sel_pBytes;
extern BITMAPINFO *sel_info;

extern char str[100];

void re_init(bool isgameover)
{
	if (isgameover)
	{
		soundSystem->playSound(resultbgm[0], NULL, 0, &channel[0]);
		channel[0]->setVolume(volume);
		first_re = 1;

		glGenTextures(2, resultscreen);

		glBindTexture(GL_TEXTURE_2D, resultscreen[0]);
		re_pBytes = LoadDIBitmap("../Resource/Gameover.bmp", &re_info);
		glTexImage2D(GL_TEXTURE_2D, 0, 3, 480, 360, 0, GL_BGR_EXT,
			GL_UNSIGNED_BYTE, re_pBytes);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	}
	else
	{
		soundSystem->playSound(resultbgm[1], NULL, 0, &channel[0]);
		channel[0]->setVolume(volume);
		first_re = 1;
		if (notenum * 2 >= perfact * 2 + great * 1 && notenum * 1.6 <= perfact * 2 + great * 1)
		{
			re_rank = 0;
		}
		else if (notenum * 1.6 >= perfact * 2 + great * 1 && notenum * 1.2 <= perfact * 2 + great * 1)
		{
			re_rank = 1;
		}
		else if (notenum * 1.2 >= perfact * 2 + great * 1 && notenum * 0.8 <= perfact * 2 + great * 1)
		{
			re_rank = 2;
		}
		else
		{
			re_rank = 3;
		}
		if (Highscore["Score"][select].asInt() < score)
		{
			Highscore["Score"][select] = score;
			Highscore["Rank"][select] = re_rank;

			std::ofstream ou("../Resource/highscore.json", std::ios::out);
			ou << Highscore;
		}
	}
}

void draw_result(bool isgameover)
{
	if (first_re == 0)
	{
		re_init(isgameover);
	}

	glViewport(0, 0, 800, 800);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-400.0, 400.0, -300.0, 300.0, -5000.0, 5000.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	if (isgameover)
	{
		

		glColor4f(1, 1, 1, 1);

		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, resultscreen[0]);
		glBegin(GL_QUADS);
		{
			glTexCoord2f(0.0f, 1.0f);
			glVertex3d(-400, 400, 0);
			glTexCoord2f(0.0f, 0.0f);
			glVertex3d(-400, -400, 0);
			glTexCoord2f(1.0f, 0.0f);
			glVertex3d(400, -400, 0);
			glTexCoord2f(1.0f, 1.0f);
			glVertex3d(400, 400, 0);
		}
		glEnd();
		glDisable(GL_TEXTURE_2D);
	}
	else
	{
		glColor4d(1, 1, 1, 1);
		glBegin(GL_QUADS);
		glVertex3d(-400, 300, -100);
		glVertex3d(-400, -300, -100);
		glVertex3d(400, -300, -100);
		glVertex3d(400,300, -100);
		glEnd();

		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, object[select]);
		glBegin(GL_QUADS);

		glTexCoord2f(0.0f, 1.0f);
		glVertex3d(-380, 290, -10);
		glTexCoord2f(0.0f, 0.0f);
		glVertex3d(-380, 160, -10);
		glTexCoord2f(1.0f, 0.0f);
		glVertex3d(-180, 160, -10);
		glTexCoord2f(1.0f, 1.0f);
		glVertex3d(-180, 290, -10);

		glEnd();
		glDisable(GL_TEXTURE_2D);

		glColor4d(0, 0, 0, 1);
		renderBitmapCharacter(-160, 265, -20, GLUT_BITMAP_TIMES_ROMAN_24, root["Music"][select]["Musicname"].asCString());
		renderBitmapCharacter(-160, 235, -20, GLUT_BITMAP_9_BY_15, root["Music"][select]["Compname"].asCString());

		glColor4d(1, 0, 0, 1);
		renderBitmapCharacter(-160, 165, -20, GLUT_BITMAP_9_BY_15, "highscore : ");


		itoa(Highscore["Score"][select].asInt(), str, 10);
		renderBitmapCharacter(20, 165, -20, GLUT_BITMAP_9_BY_15, str);


		glColor4d(1, 1, 1, 1);
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, rank[Highscore["Rank"][select].asInt()]);

		glBegin(GL_QUADS);


		glTexCoord2f(0.0f, 1.0f);
		glVertex3d(180, 290, -10);
		glTexCoord2f(0.0f, 0.0f);
		glVertex3d(180, 160, -10);
		glTexCoord2f(1.0f, 0.0f);
		glVertex3d(380, 160, -10);
		glTexCoord2f(1.0f, 1.0f);
		glVertex3d(380, 290, -10);

		glEnd();
		glDisable(GL_TEXTURE_2D);

		glColor4d(0, 0, 1, 1);
		renderBitmapCharacter(-160, 50, -20, GLUT_BITMAP_TIMES_ROMAN_24, "Perfact");
		itoa(perfact, str, 10);
		renderBitmapCharacter(0, 50, -20, GLUT_BITMAP_TIMES_ROMAN_24, str);
		renderBitmapCharacter(-160, 0, -20, GLUT_BITMAP_TIMES_ROMAN_24, "Great");
		itoa(great, str, 10);
		renderBitmapCharacter(0, 0, -20, GLUT_BITMAP_TIMES_ROMAN_24, str);
		renderBitmapCharacter(-160, -50, -20, GLUT_BITMAP_TIMES_ROMAN_24, "Good");
		itoa(good, str, 10);
		renderBitmapCharacter(0, -50, -20, GLUT_BITMAP_TIMES_ROMAN_24, str);
		renderBitmapCharacter(-160, -100, -20, GLUT_BITMAP_TIMES_ROMAN_24, "Miss");
		itoa(miss, str, 10);
		renderBitmapCharacter(0, -100, -20, GLUT_BITMAP_TIMES_ROMAN_24, str);
		renderBitmapCharacter(-160, -175, -20, GLUT_BITMAP_TIMES_ROMAN_24, "Score");
		itoa(score, str, 10);
		renderBitmapCharacter(0, -175, -20, GLUT_BITMAP_TIMES_ROMAN_24, str);



		glColor4d(1, 1, 1, 1);
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, rank[re_rank]);

		glBegin(GL_QUADS);


		glTexCoord2f(0.0f, 0.0f);
		glVertex3d(130, -190, -10);
		glTexCoord2f(0.0f, 1.0f);
		glVertex3d(130, -60, -10);
		glTexCoord2f(1.0f, 1.0f);
		glVertex3d(330, -60, -10);
		glTexCoord2f(1.0f, 0.0f);
		glVertex3d(330, -190, -10);

		glEnd();
		glDisable(GL_TEXTURE_2D);
	}
}