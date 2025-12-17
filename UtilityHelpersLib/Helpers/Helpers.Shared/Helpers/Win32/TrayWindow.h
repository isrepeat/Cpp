#pragma once
#include "Helpers/common.h"

#if COMPILE_FOR_DESKTOP
#include "MainWindow.h"
#include <shellapi.h>

namespace HELPERS_NS {
	namespace Win32 {
		class TrayWindow : public MainWindow {
		public:
			TrayWindow(
				HINSTANCE hInstance,
				HICON trayIcon,
				const std::wstring& tooltipText = L"Tray App"
			);

			~TrayWindow() override;

			void RunMessageLoop();

		protected:
			LRESULT HandleMessage(UINT message, WPARAM wParam, LPARAM lParam) override;

		private:
			void InitTrayIcon(const std::wstring& tooltipText, HICON trayIcon);
			void RemoveTrayIcon();
			void ShowContextMenu(POINT pt);
			void OnCommand(int commandId);

		private:
			NOTIFYICONDATA nid{};
			enum MenuCommand {
				COMMAND_EXIT = 1
			};
		};
	}
}
#endif // COMPILE_FOR_DESKTOP