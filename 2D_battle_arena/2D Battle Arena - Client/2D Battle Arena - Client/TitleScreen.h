#include "Screen.h"

class TitleScreen : public Screen
{
public:
	TitleScreen();
	~TitleScreen();

	void RenderScene();
	void Update(float time, NGPInputs* inputs);
	void ScreenChange();

private:

};

