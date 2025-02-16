#include <imgui/imgui.h>

#include "app_wrap.h"
#include "input_wrap.h"
#include "imgui_wrap.h"
#include "wrap/recorder_wrap.h"

#include "core/platform.h"
#include "core/shader_loader.h"
#include "core/gl_message_log.h"
#include "core/dev_menus.h"

#include "../src/timers.h"
#include "../core/cpu_timer.h"

extern void toyInit();
extern bool toyLoop(Input& in, AppState& app);
extern void toyTerm();

int main(int, char**)
{
	{
		BlockTimer timer("LogInit");
		logInit("log.txt");
		printLog("Client main entry.\n");
	}
	int ret = 0;

	AppInit init;
	init.title = "SDF Toy";
	//init.res_x = 2560;
	//init.res_x = 720;
	//init.res_y = 1600;
	init.res_x = 1920;
	init.res_y = 1080;
	init.antialiasing = 0;
	//init.fullscreen = true;
	{
		BlockTimer timer("AppInit");
		ret = appInit(init);
		printLog("App init: %d\n", ret);
	}
	if (ret) return ret;
	{
		BlockTimer timer("GLMessageInit");
		glMessageLogInit();
		printLog("GL message log init ok.\n");
	}
	{
		BlockTimer timer("ImGuiInit");
		imguiInit();
		printLog("ImGui init ok.\n");
	}
	{
		BlockTimer timer("GameInit");
		toyInit();
		printLog("Game init ok.\n");
	}

	Input in;
	AppState app_state;

    while (!appShouldClose())
    {
		inputPoll(in);
		appGetState(app_state);

		imguiNewFrame();

		//ctimer_reset();
		//ctimer_gui();
		// gtimer_gui();
		recUI(in.key.press[KEY_F10]);

		bool exit = toyLoop(in, app_state); 

		if (checkForShaderErrors())
			setDevMenuOpen("Shaders", true);
		if (*getDevMenuOpen("Shaders"))
			guiShader(getDevMenuOpen("Shaders"));

		if (*getDevMenuOpen(GL_MESSAGE_LOG_WINDOW_NAME))
			glMessageLogUI(getDevMenuOpen(GL_MESSAGE_LOG_WINDOW_NAME));

		ImGui::ShowDemoWindow();
		imguiRender();

		recFrame("all", 0,0,app_state.res_x,app_state.res_y);
		recUpdate();

		appSwapBuffers();

		if (exit)
			break;
    }

	{
		BlockTimer timer("AllTerm");
		toyTerm();
		imguiTerm();
		appTerm();
		// Can't time logTerm, because once it's term'ed it can't log the result lol
	}

	printLog("Client main exit.\n");
	logTerm();

    return 0;
}
