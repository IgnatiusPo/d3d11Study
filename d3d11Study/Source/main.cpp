#include "Application.h"



int main()
{
	Application app;
	app.Init();
	while (!app.ShouldClose())
	{
		app.Tick();
	}

	return 0;
}