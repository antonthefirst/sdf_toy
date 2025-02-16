#include "app_wrap.h"

#ifdef _WIN32
#include <GL/gl3w.h>
#include <GLFW/glfw3.h>
#include <stdio.h>

GLFWwindow* gWindow;

static void error_callback(int error, const char* description)
{
	fprintf(stderr, "Error %d: %s\n", error, description);
}


#if 0
//preprocessor
GLFW_EXPOSE_NATIVE_WIN32
GLFW_EXPOSE_NATIVE_WGL
#include <GLFW/glfw3native.h>

#pragma lib Dwmapi.lib
#include <Dwmapi.h>
HRESULT EnableBlurBehind(HWND hwnd)
{
	HRESULT hr = S_OK;

	// Create and populate the Blur Behind structure
	DWM_BLURBEHIND bb = { 0 };

	// Enable Blur Behind and apply to the entire client area
	bb.dwFlags = DWM_BB_ENABLE;
	bb.fEnable = true;
	bb.hRgnBlur = NULL;

	// Apply Blur Behind
	hr = DwmEnableBlurBehindWindow(hwnd, &bb);
	if (SUCCEEDED(hr))
	{
		printf("cool\n");
		// ...
	}
	return hr;
}
#endif

int appInit(AppInit init)
{
	// Setup window
	glfwSetErrorCallback(error_callback);
	if (!glfwInit())
		return 1;
	//glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	//glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	//glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_SAMPLES, init.antialiasing);
	//glfwWindowHint(GLFW_ALPHA_BITS, GL_TRUE);
	//glfwWindowHint(GLFW_DECORATED, GL_FALSE);
	gWindow = glfwCreateWindow(init.res_x, init.res_y, init.title, init.fullscreen ? glfwGetPrimaryMonitor() : NULL, NULL);
	if (!gWindow)
		return 1;

	GLFWmonitor* monitor = glfwGetPrimaryMonitor();
	int x,y,width,height;
	glfwGetMonitorWorkarea(monitor, &x, &y, &width, &height);
	glfwSetWindowPos(gWindow, x + width / 2 - init.res_x / 2, y + height / 2 - init.res_y / 2);

	glfwMakeContextCurrent(gWindow);

	gl3wInit();
	//EnableBlurBehind(glfwGetWin32Window(gWindow));
	return 0;
}

void appGetState(AppState& state)
{
	glfwGetFramebufferSize(gWindow, &state.res_x, &state.res_y);
}

bool appShouldClose()
{
	return glfwWindowShouldClose(gWindow) != 0;
}
void appToggleCursor()
{
	int currInputMode = glfwGetInputMode(gWindow, GLFW_CURSOR);
	if (currInputMode == GLFW_CURSOR_DISABLED)
		glfwSetInputMode(gWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	else
		glfwSetInputMode(gWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}
void appEnableCursor(bool enable)
{
	glfwSetInputMode(gWindow, GLFW_CURSOR, enable ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);
}
void appSwapBuffers()
{
	glfwSwapBuffers(gWindow);
}

void appTerm()
{
	glfwTerminate();
}
void appDelay(float time)
{
	Sleep(int(time * 1000.0f));
}

#endif


















#if __APPLE__
glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif