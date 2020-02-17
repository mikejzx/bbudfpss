
// Include the precompiled header.
#include "pch.h"

// GUI methods.
#include "gui.h"

// Defines
#define FILE_PREFS_PATH "./bbudfpss-prefs.txt"

// Statics.
static HWND s_hWnd = NULL;
static HINSTANCE s_hInst = NULL;

// Prototypes.
LRESULT CALLBACK WinProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void ReadPrefsFile();

// Entry point of the application.
INT APIENTRY wWinMain(HINSTANCE hInst, HINSTANCE hPrevInst, PWSTR pCmdLine, int nCmdShow)
{
	(void)hPrevInst;
	(void)pCmdLine;

	// Initialise commctrl
	INITCOMMONCONTROLSEX picce = { };
	picce.dwSize = sizeof(INITCOMMONCONTROLSEX);
	picce.dwICC = ICC_STANDARD_CLASSES;
	InitCommonControlsEx(&picce);

	// Initialise COM in single-thread mode.
	CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);

	// Create window.
	s_hInst = hInst;
	if (!(s_hWnd = InitialiseWindow(WinProc, s_hInst)))
	{
		return -1;
	}

	// Show window with show params.
	ShowWindow(s_hWnd, nCmdShow);
	UpdateWindow(s_hWnd);

	// Run message loop
	MSG msg = { };
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	// Uninitialise COM.
	CoUninitialize();

	return (int)msg.wParam;
}

// Main window procedure.
LRESULT CALLBACK WinProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		// A command has been issued.
		case WM_COMMAND:
		{
			switch (LOWORD(wParam))
			{
				// Selecting directory to back up.
				case BTNID_BROWSE_IN:
				{
					HandleBrowseButton(hwnd, hTextDirInput);
				} break;

				// Selecting directory to save back up to.
				case BTNID_BROWSE_OUT:
				{
					HandleBrowseButton(hwnd, hTextDirOutput);
				} break;

				// Create the archive.
				case BTNID_CREATE:
				{
					// Get strings
					wchar_t indir[MAX_PATH];
					wchar_t outdir[MAX_PATH];
					GetWindowTextW(hTextDirInput, indir, MAX_PATH);
					GetWindowTextW(hTextDirOutput, outdir, MAX_PATH);

					// Write to the preferneces file
					std::wofstream file(FILE_PREFS_PATH);
					file << indir << std::endl << outdir << std::endl;
					file.close();

					// Create the actual archive.
					HandleArchiveCreate(indir, outdir);

					// Change status.
					wchar_t statustext[MAX_PATH + 128] = L"Status: Wrote to ";
					wcscat_s(statustext, outdir);
					wcscat_s(statustext, L"\\(?).tar.gz");
					SetWindowText(hStatus, statustext);
				} break;

				default:
				{
					return DefWindowProc(hwnd, uMsg, wParam, lParam);
				}
			}
		} return 0;

		// Window becoming visible.
		case WM_CREATE:
		{
			// Initialise the GUI.
			InitialiseGUI(hwnd);

			// Try open the preferences file after we've created edit controls.
			ReadPrefsFile();
		} break;

		// Window is being destroyed.
		case WM_DESTROY:
		{
			PostQuitMessage(0);
		} break;

		// Set static colour of label control.
		case WM_CTLCOLORSTATIC:
		{
			// Set the status text colour.
			if ((HWND)lParam == hStatus)
			{
				SetBkMode((HDC)wParam, RGB(255, 255, 255));
				return SetTextColor((HDC)wParam, RGB(0, 128, 0));
			}

			HDC hdc = CreateDC(TEXT("DISPLAY"), NULL, NULL, NULL);
			return SetBkColor(hdc, RGB(255, 255, 255));
		}

		// Window needs to be painted.
		case WM_PAINT:
		{
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hwnd, &ps);
			//FillRect(hdc, &ps.rcPaint, (HBRUSH)(COLOR_WINDOWFRAME));
			EndPaint(hwnd, &ps);
		} break;

		default:
		{
			// Call default procedure.
			return DefWindowProc(hwnd, uMsg, wParam, lParam);
		}
	}

	return 0;
}

// Reads the preferences file stored in app directory.
// If it doesn't exist, then it is created.
void ReadPrefsFile()
{
	// Check if file exists.
	std::wifstream f(FILE_PREFS_PATH);
	if (f)
	{
		// Read the first two lines.
		unsigned i = 0;
		std::vector<std::wstring> readstrs;
		readstrs.reserve(2);
		for (std::wstring line; getline(f, line) && i < 2; ++i)
		{
			readstrs.push_back(line);
		}

		// If we have both strings.
		if (readstrs.size() == 2)
		{
			SetWindowTextW(hTextDirInput, readstrs[0].c_str());
			SetWindowTextW(hTextDirOutput, readstrs[1].c_str());
		}

		f.close();
	}
	else
	{
		// Couldn't open. Create it.
		std::ofstream file(FILE_PREFS_PATH);
		file << std::endl << std::endl;
		file.close();
	}
}
