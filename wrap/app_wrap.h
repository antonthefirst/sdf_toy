#pragma once

struct AppInit
{
	int res_x = 0;
	int res_y = 0;
	bool fullscreen = false;
	int antialiasing = 0;
	const char* title = "App";
};

struct AppState
{
	int res_x = 0;
	int res_y = 0;
};

int appInit(AppInit init);
void appGetState(AppState& state);
bool appShouldClose();
void appToggleCursor();
void appEnableCursor(bool enable);
void appSwapBuffers();
void appTerm();
void appDelay(float time);