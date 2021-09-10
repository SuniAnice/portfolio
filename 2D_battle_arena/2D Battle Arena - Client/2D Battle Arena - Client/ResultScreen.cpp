#include "ResultScreen.h"
#include "bitmap.h"

extern GLuint title_texture[2];

ResultScreen::ResultScreen(bool Result)
{
	GameResult = Result;
}

ResultScreen::~ResultScreen()
{
}

void ResultScreen::RenderScene()
{
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	if (GameResult == true)
	{
		renderBitmapCharacterCenter(0, 200, 0, GLUT_BITMAP_TIMES_ROMAN_24, "You Win!");
	}
	else
	{
		renderBitmapCharacterCenter(0, 200, 0, GLUT_BITMAP_TIMES_ROMAN_24, "You Lose..");
	}

	DrawTextureRect(0, -150, 300, 150, title_texture[1]);
}

void ResultScreen::Update(float time, NGPInputs* inputs)
{
	if (inputs->MouseX > -150 && inputs->MouseX < 150 && inputs->MOUSE_LEFT == true)
	{
		if (inputs->MouseY > 75 && inputs->MouseY < 225)
		{
			// 종료 버튼
			exit(0);
		}
	}
}
