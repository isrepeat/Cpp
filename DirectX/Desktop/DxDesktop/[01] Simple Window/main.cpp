#include <Windows.h>
#include "MainWindow.h"

int WINAPI WinMain(
	HINSTANCE hInstance,
	HINSTANCE /*hPrevInstance*/,
	LPSTR /*lpCmdLine*/,
	int /*nShowCmd*/)
{
	auto mainWindow = DxDesktop::MainWindow{ hInstance, 800, 600 };

	mainWindow.StartMessageLoop();
	return 0;
}