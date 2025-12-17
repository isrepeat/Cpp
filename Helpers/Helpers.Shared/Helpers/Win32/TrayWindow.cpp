#include "TrayWindow.h"
#include <Helpers/Logger.h>
#include <shellapi.h>

namespace HELPERS_NS {
	namespace Win32 {
		namespace {
			constexpr UINT WM_TRAYICON = WM_APP + 1;
		}

		TrayWindow::TrayWindow(HINSTANCE hInstance, HICON trayIcon, const std::wstring& tooltipText)
			: MainWindow(hInstance, 200, 200, 0, 0, WS_OVERLAPPEDWINDOW) {
			//::ShowWindow(this->GetHwnd(), SW_HIDE);
			this->InitTrayIcon(tooltipText, trayIcon);
		}

		TrayWindow::~TrayWindow() {
			this->RemoveTrayIcon();
		}

		void TrayWindow::RunMessageLoop() {
			this->MainWindow::RunMessageLoop();
		}

		LRESULT TrayWindow::HandleMessage(UINT message, WPARAM wParam, LPARAM lParam) {
			switch (message) {
			case WM_COMMAND:
				this->OnCommand(LOWORD(wParam));
				return 0;

			case WM_TRAYICON:
				if (lParam == WM_RBUTTONUP) {
					POINT pt;
					::GetCursorPos(&pt);
					this->ShowContextMenu(pt);
					return 0;
				}
				break;
			}
			return MainWindow::HandleMessage(message, wParam, lParam);
		}

		void TrayWindow::InitTrayIcon(const std::wstring& tooltipText, HICON trayIcon) {
			this->nid.cbSize = sizeof(NOTIFYICONDATA);
			this->nid.hWnd = this->GetHwnd();
			this->nid.uID = 1;
			this->nid.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
			this->nid.uCallbackMessage = WM_TRAYICON;
			this->nid.hIcon = trayIcon;
			wcscpy_s(this->nid.szTip, tooltipText.c_str());

			if (!::Shell_NotifyIconW(NIM_ADD, &this->nid)) {
				LOG_ERROR_D("Shell_NotifyIcon failed");
				throw std::exception("Shell_NotifyIcon failed");
			}
		}

		void TrayWindow::RemoveTrayIcon() {
			::Shell_NotifyIconW(NIM_DELETE, &this->nid);
		}

		void TrayWindow::ShowContextMenu(POINT pt) {
			HMENU hMenu = ::CreatePopupMenu();
			::AppendMenuW(hMenu, MF_STRING, COMMAND_EXIT, L"Exit");

			::SetForegroundWindow(this->GetHwnd());
			::TrackPopupMenu(hMenu, TPM_RIGHTBUTTON, pt.x, pt.y, 0, this->GetHwnd(), nullptr);
			::DestroyMenu(hMenu);
		}

		void TrayWindow::OnCommand(int commandId) {
			if (commandId == COMMAND_EXIT) {
				::PostQuitMessage(0);
			}
		}
	}
}