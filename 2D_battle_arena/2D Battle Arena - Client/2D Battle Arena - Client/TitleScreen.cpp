#include "stdafx.h"
#include "TitleScreen.h"
#include "bitmap.h"
#include "GameScreen.h"

extern char namebuf[10];
extern char SERVERIPinfo[15];

extern GLuint title_texture[2];



TitleScreen::TitleScreen()
{
	
	
	
	
}

TitleScreen::~TitleScreen()
{
}

void TitleScreen::RenderScene()
{
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glColor4f(1, 1, 1, 1);
	renderBitmapCharacter(-600, -350, 0, GLUT_BITMAP_TIMES_ROMAN_24, "IPAddress : ");
	renderBitmapCharacter(-450, -350, 0, GLUT_BITMAP_TIMES_ROMAN_24, SERVERIPinfo);
	renderBitmapCharacter(-600, -380, 0, GLUT_BITMAP_TIMES_ROMAN_24, "PlayerName : ");
	renderBitmapCharacter(-450, -380, 0, GLUT_BITMAP_TIMES_ROMAN_24, namebuf);

	renderBitmapCharacterCenter(0, 200, 0, GLUT_BITMAP_TIMES_ROMAN_24, "2D Battle Arena");


	DrawTextureRect(0, 50, 300, 150, title_texture[0]);

	DrawTextureRect(0, -150, 300, 150, title_texture[1]);


}

void TitleScreen::Update(float time, NGPInputs* inputs)
{
		
	if (inputs->MouseX > -150 && inputs->MouseX < 150 && inputs->MOUSE_LEFT == true)
	{
		if (inputs->MouseY > -125 && inputs->MouseY < 25)
		{
			// 시작 버튼
			ScreenChange();
			
		}
		if (inputs->MouseY > 75 && inputs->MouseY < 225)
		{
			// 종료 버튼
			exit(0);
		}
	}
}

void TitleScreen::ScreenChange()
{
	check = 1;
}