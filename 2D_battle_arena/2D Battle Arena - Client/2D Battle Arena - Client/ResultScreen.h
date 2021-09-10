#pragma once
#include "Screen.h"
class ResultScreen :
    public Screen
{
public:
	ResultScreen(bool Result);
	~ResultScreen();

	void RenderScene();
	void Update(float time, NGPInputs* inputs);

private:
	bool GameResult;
};

