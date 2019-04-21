#include "GLComponent.h"
#include "GLMemory.h"
#include <iostream>
#include <Windows.h>
#include "Input/InputState.h"
#include "GameState.h"
#include "PlayerData.h"
#include "../Input/Keyboard/Keyboard.h"
#include "../Input/Xinput/Xinput.h"
#include "../Constants.h"
#include "wtypes.h"
#include "../MainModule.h"
#include "CustomPlayerData.h"
#include "PlayerDataManager.h"
#include "FrameRateManager.h"
#include "CameraController.h"
#include "Input/InputEmulator.h"
#include "Input/TouchPanelEmulator.h"
#include "../Components/EmulatorComponent.h"
#include "../FileSystem/ConfigFile.h"
#include "parser.hpp"
#include <fstream>
#include <sstream>

#include <chrono>
#include <thread>

#include <cassert>
#include "imgui/imgui.h"
#include "imgui/imgui_impl_opengl2.h"
#include "imgui/imgui_impl_win32.h"
#include "GL/gl.h"
#include "GL/glext.h"

#include "base64/base64.h"

namespace DivaHook::Components
{
	using namespace std::chrono;
	using dsec = duration<double>;

	static int moduleEquip1 = 0;
	static int moduleEquip2 = 0;
	static int btnSeEquip = 0;
	static int skinEquip = 0;
	static bool showUi = false;
	static bool showFps = false;
	static bool showUi2 = false;
	static bool showAbout = false;
	static int firstTime = 1000;
	static int fpsLimit = 0;
	static int fpsLimitSet = 0;
	static int sfxVolume = 100;
	static int bgmVolume = 100;
	static float uiTransparency = 0.8;
	static float sleep = 0;
	static float fpsDiff = 0;
	static bool morphologicalAA = 0;
	static bool morphologicalAA2 = 0;
	static bool temporalAA = 0;

	static bool toonShader = true;
	static bool toonShader2 = false;

	PlayerDataManager* pdm;
	FrameRateManager* frm;
	InputEmulator* inp;
	TouchPanelEmulator* tch;
	CameraController* cam;

	static int maxRenderWidth = 2560;
	static int maxRenderHeight = 1440;
	static std::chrono::time_point mBeginFrame = system_clock::now();
	static std::chrono::time_point prevTimeInSeconds = time_point_cast<seconds>(mBeginFrame);
	static unsigned frameCountPerSecond = 0;

	static bool resetGame = false;
	static bool resetGameUi = false;
	static bool debugUi = false;

	static int module1[1000];
	static int module2[1000];
	static int currentPv = 0;
	static bool fileLoaded = true;

	static bool recordReplay = false;
	static bool recordReplay2 = false;

	static bool playReplay = false;
	static bool playReplay2 = false;

	static int timeBeforeInit = 5000;
	static bool initialized = false;

	char chara[0x212U] = "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAA==";

	static char lastState[255];

	GLubyte* pixels;
	GLubyte* pixels2;
	GLubyte* pixels3;
	static const GLenum FORMAT = GL_COMPRESSED_RGBA;
	static const GLuint FORMAT_NBYTES = 4;
	static bool wtf = false;
	static bool copydepth = false;
	static bool glinit = false;
	GLuint pbo1 = 400000000;
	GLuint pbo2 = 400000001;
	GLuint pbo3 = 400000003;
	GLuint texture3 = 400000002;
	GLuint texture4 = 400000004;
	GLuint texture5 = 400000005;
	bool pboflag = false;
	bool pbo1yay = false;
	bool pbo2yay = false;
	bool pbo3yay = false;
	GLint depth;
	static int pboindex = 0;
	int i1 = -1;	int i2 = 1;
	int i3 = -1;	int i4 = 0;
	int i5 = 0;		int i6 = 0;
	int i7 = 0;		int i8 = 1;

	int aX = 0; int aY = 0;
	int aW = 640; int aH = 720;
	int bX = 640; int bY = 0;
	int bW = 640; int bH = 720;

	GLComponent::GLComponent()
	{
		pdm = new PlayerDataManager();
		frm = new FrameRateManager();
		inp = new InputEmulator();
		tch = new TouchPanelEmulator();
		cam = new CameraController();
		//pixels2 = new GLubyte[FORMAT_NBYTES * 1280 * 720];
		//memset(pixels2, 255, sizeof(pixels2));
	}

	GLComponent::~GLComponent()
	{

	}

	void drawshit()
	{
		//glBindTexture(GL_TEXTURE_2D, 0);

		//glCopyTexImage2D(renderedTexture, 0, GL_RGB, 0, 0, 640, 720, 0);
		//glCopyTexImage2D(renderedTexture, 0, GL_RGB, 640, 0, 640, 720, 0);

		//glBindTexture(0, renderedTexture);
	}

	PFNGLGENFRAMEBUFFERSEXTPROC glGenFramebuffersEXT;
	PFNGLBINDFRAMEBUFFEREXTPROC glBindFramebufferEXT;
	PFNGLFRAMEBUFFERTEXTURE2DEXTPROC glFramebufferTexture2DEXT;
	PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC glCheckFramebufferStatusEXT;
	PFNGLDELETEFRAMEBUFFERSEXTPROC glDeleteFramebuffersEXT;
	PFNGLBINDBUFFERARBPROC glBindBufferARB;
	PFNGLBUFFERDATAARBPROC glBufferDataARB;
	PFNGLMAPBUFFERARBPROC glMapBufferARB;
	PFNGLUNMAPBUFFERARBPROC glUnmapBufferARB;
	PFNGLGENBUFFERSARBPROC glGenBuffersARB;
	PFNGLBLITFRAMEBUFFEREXTPROC glBlitFramebufferEXT;

	void drawPbo(GLubyte* pixels, GLint x, GLint y, GLint w, GLint h)
	{
		glBindTexture(GL_TEXTURE_2D, texture3);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1280, 720, 0, GL_BGR, GL_UNSIGNED_BYTE, pixels);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glEnable(GL_TEXTURE_2D);

		glBegin(GL_QUADS);
		glTexCoord2i(i1, i2); glVertex2i(x, y);
		glTexCoord2i(i3, i4); glVertex2i(x, y + h);
		glTexCoord2i(i5, i6); glVertex2i(x+h, y+h);
		glTexCoord2i(i7, i8); glVertex2i(x+h, y);
		glEnd();
		glDisable(GL_TEXTURE_2D);

		glBindTexture(GL_TEXTURE_2D, 0);
		glBindBufferARB(GL_PIXEL_UNPACK_BUFFER, 0);
	}

	void writePbo(GLuint pbo)
	{
		glBindBufferARB(GL_PIXEL_PACK_BUFFER, pbo);
		glReadBuffer(GL_BACK);
		glReadPixels(0, 0, 1280, 720, GL_BGR, GL_UNSIGNED_BYTE, nullptr);
		glBindBufferARB(GL_PIXEL_UNPACK_BUFFER, 0);
	}

	GLubyte* readPbo(GLuint pbo)
	{
		glBindBufferARB(GL_PIXEL_PACK_BUFFER, pbo);
		GLubyte* src = (GLubyte*)glMapBufferARB(GL_PIXEL_PACK_BUFFER_ARB, GL_READ_ONLY_ARB);
		glUnmapBufferARB(GL_PIXEL_PACK_BUFFER);
		glBindBufferARB(GL_PIXEL_UNPACK_BUFFER, 0);
		return src;
	}

	void pboInit(GLuint pbo)
	{
		glGenBuffersARB(1, &pbo);
		glBindBufferARB(GL_PIXEL_PACK_BUFFER, pbo);
		glBufferDataARB(GL_PIXEL_PACK_BUFFER, 1280 * 720 * 4, 0, GL_STREAM_READ);
		glBindBufferARB(GL_PIXEL_UNPACK_BUFFER, 0);
	}

	void hwglSwapBuffers(_In_ HDC hDc)
	{
		int* fbWidth = (int*)FB_RESOLUTION_WIDTH_ADDRESS;
		int* fbHeight = (int*)FB_RESOLUTION_HEIGHT_ADDRESS;

		if (!glinit)
		{
			glGenFramebuffersEXT = (PFNGLGENFRAMEBUFFERSEXTPROC)wglGetProcAddress("glGenFramebuffersEXT");
			glBindFramebufferEXT = (PFNGLBINDFRAMEBUFFEREXTPROC)wglGetProcAddress("glBindFramebufferEXT");
			glFramebufferTexture2DEXT = (PFNGLFRAMEBUFFERTEXTURE2DEXTPROC)wglGetProcAddress("glFramebufferTexture2DEXT");
			glCheckFramebufferStatusEXT = (PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC)wglGetProcAddress("glCheckFramebufferStatusEXT");
			glDeleteFramebuffersEXT = (PFNGLDELETEFRAMEBUFFERSEXTPROC)wglGetProcAddress("glDeleteFramebuffersEXT");
			glBindBufferARB = (PFNGLBINDBUFFERARBPROC)wglGetProcAddress("glBindBufferARB");
			glBufferDataARB = (PFNGLBUFFERDATAARBPROC)wglGetProcAddress("glBufferDataARB");
			glMapBufferARB = (PFNGLMAPBUFFERARBPROC)wglGetProcAddress("glMapBufferARB");
			glUnmapBufferARB = (PFNGLUNMAPBUFFERARBPROC)wglGetProcAddress("glUnmapBufferARB");
			glGenBuffersARB = (PFNGLGENBUFFERSARBPROC)wglGetProcAddress("glGenBuffersARB");
			glBlitFramebufferEXT = (PFNGLBLITFRAMEBUFFEREXTPROC)wglGetProcAddress("glBlitFramebufferEXT");

			glGetIntegerv(GL_DEPTH_BITS, &depth);
			//pboInit(pbo1);
			//pboInit(pbo2);
			//pboInit(pbo3);
			
			glGenBuffersARB(1, &pbo1);
			glGenBuffersARB(1, &pbo2);
			glGenBuffersARB(1, &pbo3);
			glBindBufferARB(GL_PIXEL_PACK_BUFFER, pbo1);
			glBufferDataARB(GL_PIXEL_PACK_BUFFER, 1280 * 720 * 4, 0, GL_STREAM_READ);
			glBindBufferARB(GL_PIXEL_PACK_BUFFER, pbo2);
			glBufferDataARB(GL_PIXEL_PACK_BUFFER, 1280 * 720 * 4, 0, GL_STREAM_READ);
			glBindBufferARB(GL_PIXEL_PACK_BUFFER, pbo3);
			glBufferDataARB(GL_PIXEL_PACK_BUFFER, 1280 * 720 * 4, 0, GL_STREAM_READ);
			glBindBufferARB(GL_PIXEL_UNPACK_BUFFER, 0);
			

			glinit = true;
		}

		if (copydepth)
		{
			RECT hWindow;
			GetClientRect(DivaHook::MainModule::DivaWindowHandle, &hWindow);
			long uiWidth = hWindow.right - hWindow.left;
			long uiHeight = hWindow.bottom - hWindow.top;

			glBindFramebufferEXT(GL_READ_FRAMEBUFFER, 3);
			glBindFramebufferEXT(GL_DRAW_FRAMEBUFFER, 0);
			glBlitFramebufferEXT(0, 0, *fbWidth, *fbHeight, 0, 0, uiWidth, uiHeight,
				GL_DEPTH_BUFFER_BIT, GL_NEAREST);
		}

		if (wtf) {
			pboindex++;
			if (pboindex >= 3)
				pboindex = 0;
			
			glBindTexture(GL_TEXTURE_2D, 0);

			if (pboflag)
			{
				writePbo(pbo2);
				pbo2yay = true;
				cam->SetEye(pboflag);
				if (pbo1yay)
				{
					pixels = readPbo(pbo1);
				}
			}

			if (!pboflag)
			{
				writePbo(pbo1);
				cam->SetEye(pboflag);
				pbo1yay = true;
				if (pbo2yay)
				{
					pixels2 = readPbo(pbo2);
				}
				
			}
			

			/*
			bool pbowritten = false;
			bool firstwritten = false;
			if (pboindex == 0)
			{
				writePbo(pbo1);
				pbo1yay = true;
			}
			else {
				if (pboindex != 1)
				if (pbo3yay)
				{
					pixels3 = readPbo(pbo3);
				}
			}

			if (pboindex == 1)
			{
				writePbo(pbo2);
				pbo2yay = true;
			}
			else {
				if (pboindex != 2)
				if (pbo1yay)
					pixels = readPbo(pbo1);
			}

			if (pboindex == 2)
			{
				writePbo(pbo3);
				pbo3yay = true;
			}
			else {
				if (pboindex != 0)
				if (pbo2yay)
					pixels2 = readPbo(pbo2);
			}
			*/

			glBindBufferARB(GL_PIXEL_UNPACK_BUFFER, 0);

			drawPbo(pixels, aX, aY, aW, aH);
			drawPbo(pixels2, bX, bY , bW ,bH);
			//glBindBufferARB(GL_PIXEL_PACK_BUFFER_ARB, 0);
			//glReadPixels(0, 0, 1280, 720, FORMAT, GL_UNSIGNED_BYTE, pixels);
			
			//glGetTexImage(GL_TEXTURE_2D, 0, GL_BGR, GL_UNSIGNED_BYTE, pixels);

			glBindTexture(GL_TEXTURE_2D, 0);
			
			/*
			if (pboflag && pbo1yay)
			{
				glBindBufferARB(GL_PIXEL_PACK_BUFFER, texture);
				pixels = nullptr;
				glUnmapBufferARB(GL_PIXEL_PACK_BUFFER);
			}

			if ((!pboflag) && pbo2yay)
			{
				glBindBufferARB(GL_PIXEL_PACK_BUFFER, texture2);
				pixels = nullptr;
			}
			*/

			glBindBufferARB(GL_PIXEL_UNPACK_BUFFER, 0);

			if (pboflag)
				pboflag = false;
			else pboflag = true;
		}

		//glFramebufferTexture2DEXT(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, 5, 0);
		//glReadBuffer(GL_FRONT_AND_BACK);
		

		ImGuiIO& io = ImGui::GetIO();
		auto keyboard = DivaHook::Input::Keyboard::GetInstance();
		auto xinput = DivaHook::Input::Xinput::GetInstance();
		io.MouseDown[0] = keyboard->IsDown(VK_LBUTTON);
		io.MouseDown[1] = keyboard->IsDown(VK_RBUTTON);
		io.MouseDown[2] = keyboard->IsDown(VK_MBUTTON);
		
		io.KeysDown[ImGuiKey_Enter] = keyboard->IsDown(VK_RETURN);
		io.KeysDown[ImGuiKey_Backspace] = keyboard->IsDown(VK_BACK);

		

		if (*fbWidth > maxRenderWidth) *fbWidth = maxRenderWidth;
		if (*fbHeight > maxRenderHeight) *fbHeight = maxRenderHeight;

		if (((keyboard->IsDown(VK_CONTROL)) && (keyboard->IsDown(VK_LSHIFT)) && (keyboard->IsTapped(VK_BACK))) || (xinput->IsTapped(XINPUT_BACK)))
		{
			if (showUi) { showUi = false; showUi2 = false; }
			else showUi = true;
		}

		int i = 48;
		while (i != 58)
		{
			if (keyboard->IsTapped(i))
			{
				io.AddInputCharacter(i);
			}
			i++;
		}

		bool p_open = true;
		RECT hWindow;
		GetClientRect(DivaHook::MainModule::DivaWindowHandle, &hWindow);

		ImGui::SetNextWindowBgAlpha(uiTransparency);

		ImGui_ImplWin32_NewFrame();
		ImGui_ImplOpenGL2_NewFrame();
		ImGui::NewFrame();

		if (firstTime > 0)
		{
			ImGuiWindowFlags window_flags = 0;
			window_flags |= ImGuiWindowFlags_NoMove;
			window_flags |= ImGuiWindowFlags_NoResize;
			window_flags |= ImGuiWindowFlags_NoTitleBar;
			window_flags |= ImGuiWindowFlags_NoCollapse;
			window_flags |= ImGuiWindowFlags_AlwaysAutoResize;
			ImGui::Begin("ELAC", &p_open, window_flags);
			ImGui::SetWindowPos(ImVec2(0, 0));
			ImGui::Text("Press Ctrl+LShift+Backspace or Select/Back/Share in your Gamepad to show/hide UI.");
			ImGui::End();
		}

		if (showFps)
		{
			ImGui::SetNextWindowBgAlpha(uiTransparency);
			ImGuiWindowFlags window_flags = 0;
			window_flags |= ImGuiWindowFlags_NoMove;
			window_flags |= ImGuiWindowFlags_NoResize;
			window_flags |= ImGuiWindowFlags_NoTitleBar;
			window_flags |= ImGuiWindowFlags_NoCollapse;
			window_flags |= ImGuiWindowFlags_AlwaysAutoResize;
			ImGui::Begin("FPS", &p_open, window_flags);
			ImGui::SetWindowPos(ImVec2(hWindow.right - 100, 0));
			ImGui::SetWindowSize(ImVec2(100, 70));
			ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
			ImGui::Text("FT: %.2fms", 1000 / ImGui::GetIO().Framerate);
			ImGui::End();
		}

		if (showAbout)
		{
			ImGuiWindowFlags window_flags = 0;
			window_flags |= ImGuiWindowFlags_NoMove;
			window_flags |= ImGuiWindowFlags_NoResize;
			window_flags |= ImGuiWindowFlags_NoCollapse;
			window_flags |= ImGuiWindowFlags_AlwaysAutoResize;
			io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
			io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
			ImGui::Begin("About DivaHook/ELAC", &showAbout, window_flags);
			ImGui::Text("Main Developer:");
			ImGui::Text("Samyuu");
			ImGui::Text("DIVAHook/ELAC Contributors:");
			ImGui::Text("Brolijah, Crash5band, Rakisaionji, Deathride58, lybxlpsv");
			ImGui::Text("DIVAHook UI by:");
			ImGui::Text("lybxlpsv");
			ImGui::Text("DIVAHook UI Contributors:");
			ImGui::Text("BesuBaru");
			if (ImGui::Button("Close")) { showAbout = false; };
			ImGui::End();
		}

		if (resetGameUi)
		{
			ImGuiWindowFlags window_flags = 0;
			window_flags |= ImGuiWindowFlags_NoMove;
			window_flags |= ImGuiWindowFlags_NoResize;
			window_flags |= ImGuiWindowFlags_NoCollapse;
			window_flags |= ImGuiWindowFlags_AlwaysAutoResize;
			io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
			io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
			ImGui::Begin("Reset Game", &resetGameUi, window_flags);
			ImGui::SetWindowPos(ImVec2((hWindow.right / 2) - 100, (hWindow.bottom / 2) - 50));
			ImGui::Text("Would you like to reset game?");
			if (ImGui::Button("No")) { resetGameUi = false; }; ImGui::SameLine();
			if (ImGui::Button("Yes")) { resetGame = true; resetGameUi = false; showUi = false; showAbout = false; }; ImGui::SameLine();
			ImGui::End();
		}

		if (debugUi)
		{
			ImGuiWindowFlags window_flags = 0;
			window_flags |= ImGuiWindowFlags_NoResize;
			window_flags |= ImGuiWindowFlags_NoCollapse;
			window_flags |= ImGuiWindowFlags_AlwaysAutoResize;
			io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
			io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
			ImGui::Begin("debugUi", &debugUi, window_flags);
			ImGui::InputInt("currentSelectedPv", &currentPv);
			ImGui::InputInt("Module 1 ID", &moduleEquip1);
			ImGui::InputInt("Module 2 ID", &moduleEquip2);
			ImGui::Checkbox("recordreplay", &recordReplay);
			ImGui::Checkbox("playreplay", &playReplay);
			ImGui::Checkbox("3dsbs", &wtf);
			ImGui::Checkbox("depthcopy", &copydepth);
			ImGui::InputInt("pboIndex", &pboindex);
			ImGui::InputInt("1", &aX);
			ImGui::InputInt("2", &aY);
			ImGui::InputInt("3", &aW);
			ImGui::InputInt("4", &aH);
			ImGui::InputInt("5", &bX);
			ImGui::InputInt("6", &bY);
			ImGui::InputInt("7", &bW);
			ImGui::InputInt("8", &bH);
			ImGui::Text(chara);
			if (ImGui::Button("Close")) { debugUi = false; }; ImGui::SameLine();
			if (ImGui::Button("CloseMainUi")) { showUi = false; }; ImGui::SameLine();
			ImGui::End();
		}

		if (showUi) {
			ImGui::SetNextWindowBgAlpha(uiTransparency);
			ImGuiWindowFlags window_flags = 0;
			window_flags |= ImGuiWindowFlags_NoResize;
			window_flags |= ImGuiWindowFlags_NoCollapse;
			window_flags |= ImGuiWindowFlags_AlwaysAutoResize;
			io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
			io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
			ImGui::Begin("DivaHook Config", &showUi, window_flags);
			if (ImGui::CollapsingHeader("Modules and Custom Skins/Sounds"))
			{
				ImGui::Text("--- Changes only takes effect after entering a new stage. ---");
				ImGui::InputInt("Module 1 ID", &moduleEquip1);
				ImGui::InputInt("Module 2 ID", &moduleEquip2);
				ImGui::InputInt("Button SFX ID", &btnSeEquip);
				ImGui::InputInt("HUD Skin ID", &skinEquip);
			}
			if (ImGui::CollapsingHeader("Internal Resolution"))
			{
				ImGui::InputInt("Resolution Width", fbWidth);
				ImGui::InputInt("Resolution Height", fbHeight);
			}
			if (!recordReplay)
			if (ImGui::CollapsingHeader("Framerate"))
			{
				ImGui::Text("--- Set the FPS cap to 0 only if you have vsync. ---");
				ImGui::InputInt("Framerate Cap", &fpsLimitSet);
			}
			if (ImGui::CollapsingHeader("Graphics settings"))
			{
				ImGui::Text("--- Anti-Aliasing ---");
				ImGui::Checkbox("MLAA (Morphological AA)", &morphologicalAA);
				ImGui::Checkbox("TAA (Temporal AA)", &temporalAA);
				ImGui::Text("--- Bug Fixes ---");
				ImGui::Checkbox("Toon Shader (When Running with: -hdtv1080/-aa)", &toonShader);
			}
			if (ImGui::CollapsingHeader("Sound Settings"))
			{
				ImGui::SliderInt("HP Volume", &bgmVolume, 0, 100);
				ImGui::SliderInt("ACT Volume", &sfxVolume, 0, 100);
			}
			if (ImGui::CollapsingHeader("UI Settings"))
			{
				ImGui::SliderFloat("UI Transparency", &uiTransparency, 0, 1.0);
				ImGui::Checkbox("Framerate Overlay", &showFps);
			}
			if (ImGui::Button("Close")) { showUi = false; }; ImGui::SameLine();
			if (ImGui::Button("Reset")) { resetGameUi = true; }; ImGui::SameLine();
			if (ImGui::Button("About")) { showAbout = true; } ImGui::SameLine();
			if (ImGui::Button("Debug")) { debugUi = true; } ImGui::SameLine();
			ImGui::End();
		}
		// Rendering
		ImGui::Render();
		ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());

		//The worst framelimit/pacer ever.
		//TODO : Destroy this thing and replace with a much better one.

		if (fpsLimitSet != fpsLimit)
		{
			mBeginFrame = system_clock::now();
			prevTimeInSeconds = time_point_cast<seconds>(mBeginFrame);
			frameCountPerSecond = 0;
			fpsLimit = fpsLimitSet;
		}

		auto invFpsLimit = round<system_clock::duration>(dsec{ 1. / fpsLimit });
		auto mEndFrame = mBeginFrame + invFpsLimit;

		auto timeInSeconds = time_point_cast<seconds>(system_clock::now());
		++frameCountPerSecond;
		if (timeInSeconds > prevTimeInSeconds)
		{
			frameCountPerSecond = 0;
			prevTimeInSeconds = timeInSeconds;
		}

		// This part keeps the frame rate.
		if (fpsLimit > 19)
			std::this_thread::sleep_until(mEndFrame);
		mBeginFrame = mEndFrame;
		mEndFrame = mBeginFrame + invFpsLimit;

		return;
	}

	DWORD owglSwapBuffers = 0;
	void __declspec(naked) SwapTrampoline()
	{
		__asm {
			PUSHFD										//Stores EFLAGS
			PUSHAD										//Stores GP Registers
			CALL hwglSwapBuffers						//Redirects the execution to our function
			POPAD                                       //Restores GP Registers
			POPFD                                       //Restores EFLAGS
			PUSH EBP									//Restores Overwritten PUSH EBP
			MOV EBP, ESP								//Restores Overwritten MOV  EBP, ESP
			JMP[owglSwapBuffers]						//Restores the execution to the original function
		}
	}

	const char* GLComponent::GetDisplayName()
	{
		return "gl_component";
	}

	const std::string RESOLUTION_CONFIG_FILE_NAME = "graphics.ini";
	static HINSTANCE hGetProcIDDLL;

	typedef void(__stdcall * startReplay)(int);
	startReplay StartReplay;

	typedef void(__stdcall * writeReplay)(float, int, int);
	writeReplay WriteReplay;

	typedef void(__stdcall * stopReplay)();
	stopReplay StopReplay;

	typedef char *(__stdcall * readReplay)(int, float, int, int, float);
	readReplay ReadReplay;

	typedef void(__stdcall * initReadReplay)(int);
	initReadReplay InitReadReplay;

	void GLComponent::Initialize()
	{
		std::ifstream f("pv_modules.csv");
		if (f.good())
		{
			aria::csv::CsvParser parser(f);
			int rowNum = 0;
			int fieldNum = 0;
			int currentPvId = 0;
			for (auto& row : parser) {
				currentPvId = 999;
				for (auto& field : row) {
					if (fieldNum == 0)
						currentPvId = std::stoi(field);
					if (fieldNum == 1)
						module1[currentPvId] = std::stoi(field);
					if (fieldNum == 2)
						module2[currentPvId] = std::stoi(field);
					fieldNum++;
				}
				fieldNum = 0;
				rowNum++;
			}
			fileLoaded = true;
		}
	}

	void GLComponent::Update()
	{
		if (initialized == true) {

			int *currentModule = (int*)CURRENT_SELECTED_PV;
			if ((currentPv != *currentModule) && (fileLoaded))
			{
				if (module1[*currentModule] != NULL)
				{
					moduleEquip1 = module1[*currentModule];
				}

				if (module2[*currentModule] != NULL)
				{
					moduleEquip2 = module2[*currentModule];
				}
				currentPv = *currentModule;
			}

			if (resetGame)
			{
				resetGame = false;
				typedef void ChangeGameState(GameState);
				ChangeGameState* changeBaseState = (ChangeGameState*)CHANGE_MODE_ADDRESS;
				changeBaseState(GS_GAME);
			}

			frm->fpsLimit = fpsLimit;
			if (fpsLimit > 19)
			{
				frm->useFpsLimitValue = true;
			}
			else frm->useFpsLimitValue = false;

			int* taa;
			taa = (int*)GFX_TEMPORAL_AA;
			if (temporalAA)
			{
				DWORD oldProtect, bck;
				VirtualProtect((BYTE*)GFX_TEMPORAL_AA, 1, PAGE_EXECUTE_READWRITE, &oldProtect);
				Memory::Write(GFX_TEMPORAL_AA, 0x01);
				VirtualProtect((BYTE*)GFX_TEMPORAL_AA, 1, oldProtect, &bck);
			}
			else {
				DWORD oldProtect, bck;
				VirtualProtect((BYTE*)GFX_TEMPORAL_AA, 1, PAGE_EXECUTE_READWRITE, &oldProtect);
				Memory::Write(GFX_TEMPORAL_AA, 0x00);
				VirtualProtect((BYTE*)GFX_TEMPORAL_AA, 1, oldProtect, &bck);
			}

			if (morphologicalAA)
			{
				if (!morphologicalAA2) {
					DWORD oldProtect, bck;
					VirtualProtect((BYTE*)0x006EE6F8, 3, PAGE_EXECUTE_READWRITE, &oldProtect);
					*((byte*)0x006EE6F8 + 0) = 0x85;
					*((byte*)0x006EE6F8 + 1) = 0x45;
					*((byte*)0x006EE6F8 + 2) = 0x10;
					VirtualProtect((BYTE*)0x006EE6F8, 3, oldProtect, &bck);
					morphologicalAA2 = true;
				}
			}
			else {
				if (morphologicalAA2) {
					DWORD oldProtect, bck;
					VirtualProtect((byte*)0x006EE6F8, 3, PAGE_EXECUTE_READWRITE, &oldProtect);
					*((byte*)0x006EE6F8 + 0) = 0x83;
					*((byte*)0x006EE6F8 + 1) = 0xE0;
					*((byte*)0x006EE6F8 + 2) = 0x00;
					VirtualProtect((byte*)0x006EE6F8, 3, oldProtect, &bck);
					morphologicalAA2 = !morphologicalAA2;
				}
			}

			if (toonShader)
			{
				if (!toonShader2)
				{
					DWORD oldProtect, bck;
					VirtualProtect((BYTE*)0x00715B86, 2, PAGE_EXECUTE_READWRITE, &oldProtect);
					*((byte*)0x00715B86 + 0) = 0x90;
					*((byte*)0x00715B86 + 1) = 0x90;
					VirtualProtect((BYTE*)0x006EE6F8, 2, oldProtect, &bck);
					toonShader2 = true;
				}
			}
			else {
				if (toonShader2)
				{
					DWORD oldProtect, bck;
					VirtualProtect((BYTE*)0x00715B86, 2, PAGE_EXECUTE_READWRITE, &oldProtect);
					*((byte*)0x00715B86 + 0) = 0x74;
					*((byte*)0x00715B86 + 1) = 0x0d;
					VirtualProtect((BYTE*)0x006EE6F8, 2, oldProtect, &bck);
					toonShader2 = !toonShader2;
				}
			}
			pdm->customPlayerData->ModuleEquip[0] = moduleEquip1;
			pdm->customPlayerData->ModuleEquip[1] = moduleEquip2;
			pdm->customPlayerData->BtnSeEquip = btnSeEquip;
			pdm->customPlayerData->SkinEquip = skinEquip;
			if (showUi) {
				if (!showUi2)
				{
					bgmVolume = pdm->playerData->hp_vol;
					sfxVolume = pdm->playerData->act_vol;
					showUi2 = true;
				}
				pdm->playerData->hp_vol = bgmVolume;
				pdm->playerData->act_vol = sfxVolume;
			}
			pdm->Update();
			frm->Update();
			cam->Update();

			if (!showUi)
			{
				tch->Update();
				inp->Update();
			}

			if (firstTime > 0) firstTime = firstTime - round(GetElapsedTime());

			float muspos = *((float*)0x1034EC8);

			if ((playReplay) && (!playReplay2))
			{
				inp->inputState->ClearState();
				InitReadReplay(currentPv);
				playReplay2 = true;
			}

			if (playReplay)
			{
				char* charas = ReadReplay(currentPv, muspos, inp->inputState->GetAddr(), sizeof(inp->inputState), fpsLimit);
				strcpy_s(chara, charas);
				inp->UpdateInputNoPoll();
			}
			else {
				playReplay2 = false;
			}
		}
		else {

			if (timeBeforeInit > 0)
			{
				timeBeforeInit = timeBeforeInit - round(GetElapsedTime());
			}

			if ((initialized == false) && (timeBeforeInit < 0))
			{
				char dllname[] = "divahook1.dll";
				size_t size = strlen(dllname) + 1;
				wchar_t wtext[20];
				size_t outSize;
				mbstowcs_s(&outSize, wtext, size, dllname, size - 1);

				hGetProcIDDLL = LoadLibrary(wtext);
				FARPROC lpfnGetProcessID = GetProcAddress(HMODULE(hGetProcIDDLL), "StartReplay");
				StartReplay = startReplay(lpfnGetProcessID);
				lpfnGetProcessID = GetProcAddress(HMODULE(hGetProcIDDLL), "WriteReplay");
				WriteReplay = writeReplay(lpfnGetProcessID);
				lpfnGetProcessID = GetProcAddress(HMODULE(hGetProcIDDLL), "StopReplay");
				StopReplay = stopReplay(lpfnGetProcessID);
				lpfnGetProcessID = GetProcAddress(HMODULE(hGetProcIDDLL), "ReadReplay");
				ReadReplay = readReplay(lpfnGetProcessID);
				lpfnGetProcessID = GetProcAddress(HMODULE(hGetProcIDDLL), "InitReadReplay");
				InitReadReplay = initReadReplay(lpfnGetProcessID);

				pdm->Initialize();
				frm->Initialize();
				tch->Initialize();
				inp->Initialize();
				cam->Initialize();

				*((int*)0x00F06290) = *((int*)0x0102C21C);
				*((int*)0x00F0628C) = *((int*)0x0102C218);

				DivaHook::FileSystem::ConfigFile resolutionConfig(MainModule::GetModuleDirectory(), RESOLUTION_CONFIG_FILE_NAME.c_str());
				bool success = resolutionConfig.OpenRead();
				if (!success)
				{
					printf("GLComponent: Unable to parse %s\n", RESOLUTION_CONFIG_FILE_NAME.c_str());
				}

				if (success) {
					std::string trueString = "1";
					std::string *value;
					if (resolutionConfig.TryGetValue("fpsLimit", value))
					{
						fpsLimitSet = std::stoi(*value);
					}
					if (resolutionConfig.TryGetValue("MLAA", value))
					{
						morphologicalAA = std::stoi(*value);
					}
					if (resolutionConfig.TryGetValue("TAA", value))
					{
						temporalAA = std::stoi(*value);
					}
					if (resolutionConfig.TryGetValue("toonShaderWorkaround", value))
					{
						toonShader = std::stoi(*value);
					}
					if (resolutionConfig.TryGetValue("showFps", value))
					{
						if (*value == trueString)
							showFps = true;
					}
				}

				moduleEquip1 = pdm->customPlayerData->ModuleEquip[0];
				moduleEquip2 = pdm->customPlayerData->ModuleEquip[1];
				btnSeEquip = pdm->customPlayerData->BtnSeEquip;
				skinEquip = pdm->customPlayerData->SkinEquip;

				int* fbWidth = (int*)FB_RESOLUTION_WIDTH_ADDRESS;
				int* fbHeight = (int*)FB_RESOLUTION_HEIGHT_ADDRESS;

				maxRenderHeight = *fbHeight;
				maxRenderWidth = *fbWidth;

				DWORD AddressToHook = (DWORD)GetProcAddress(GetModuleHandle(L"opengl32.dll"), "wglSwapBuffers");
				owglSwapBuffers = Memory::JumpHook(AddressToHook, (DWORD)SwapTrampoline, 5);

				ImGui::CreateContext();
				ImGuiIO& io = ImGui::GetIO(); (void)io;
				io.WantCaptureKeyboard == true;

				ImGui_ImplWin32_Init(MainModule::DivaWindowHandle);
				ImGui_ImplOpenGL2_Init();
				ImGui::StyleColorsDark();
				printf("GLComponent::Initialize(): Initialized\n");
				std::cout << &inp->inputState;
				initialized = true;
			}
			return;
		}
	}

		void GLComponent::UpdateInput()
		{
			if (initialized)
			{
				pdm->UpdateInput();
				frm->UpdateInput();
				cam->UpdateInput();
				if (!showUi && !playReplay)
				{
					inp->UpdateInput();
					tch->UpdateInput();
				}

				float muspos = *((float*)0x1034EC8);

				if (recordReplay)
				{
					if (!recordReplay2)
					{
						StartReplay(currentPv);
						recordReplay2 = true;
					}
					WriteReplay(muspos, inp->inputState->GetAddr(), sizeof(InputState));
				}

				if (!recordReplay)
				{
					if (recordReplay2)
					{
						StopReplay();
						recordReplay2 = false;
					}
				}
			}
		}
	}

