#ifndef GUI_H
#define GUI_H

/*
	gui.h
	Contains code related to GUI.
*/

// Properties
#define WINDOW_WID 350
#define WINDOW_HEI 210

// IDs
#define BTNID_BROWSE_IN 500
#define BTNID_BROWSE_OUT 505
#define BTNID_CREATE 510

#define CLI_APP_NAME L"bbudfpss-CLI.exe"

HWND hTextDirInput;
HWND hTextDirOutput;
HWND hStatus;

// Initialise the window.
HWND InitialiseWindow(WNDPROC wndproc, HINSTANCE hInst)
{
	// Register window class.
	const wchar_t CLASS_NAME[] = L"bbudfpss";
	WNDCLASSEX wc = { };
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = wndproc;
	wc.cbClsExtra = wc.cbWndExtra = 0;
	wc.hInstance = hInst;
	wc.hIcon = LoadIcon(hInst, IDI_APPLICATION);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = CLASS_NAME;
	wc.hIconSm = LoadIcon(wc.hInstance, IDI_APPLICATION);
	wc.hbrBackground = (HBRUSH)COLOR_WINDOWFRAME; //GetSysColorBrush(COLOR_3DFACE);
	RegisterClassEx(&wc);

	// Create window.
	HWND hwnd = CreateWindowEx(
		WS_EX_COMPOSITED, CLASS_NAME, L"bbudfpss",
		WS_VISIBLE | WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
		CW_USEDEFAULT, CW_USEDEFAULT, WINDOW_WID, WINDOW_HEI,
		NULL, NULL, hInst, NULL
	);

	return hwnd;
}

// Initialise GUI controls.
void InitialiseGUI(HWND hwnd)
{
	// Statics.
	CreateWindowW(L"STATIC", L"Directory to back up:", WS_VISIBLE | WS_CHILD,
		5, 5,
		150, 20,
		hwnd, NULL, 0, 0);
	CreateWindowW(L"STATIC", L"Output archive directory:", WS_VISIBLE | WS_CHILD,
		5, 55,
		150, 20,
		hwnd, NULL, 0, 0);
	hStatus = CreateWindowW(L"STATIC", L"Status: OK", WS_VISIBLE | WS_CHILD,
		5, WINDOW_HEI - 60,
		WINDOW_WID - 10, 20,
		hwnd, NULL, 0, 0);

	// Create directory buttons.
	CreateWindowW(L"BUTTON", L"Browse...", WS_VISIBLE | WS_CHILD, 
		WINDOW_WID - 30 - 80, 25, 
		80, 20, 
		hwnd, (HMENU)BTNID_BROWSE_IN, 0, 0);
	CreateWindowW(L"BUTTON", L"Browse...", WS_VISIBLE | WS_CHILD, 
		WINDOW_WID - 30 - 80, 50 + 25, 
		80, 20, 
		hwnd, (HMENU)BTNID_BROWSE_OUT, 0, 0);

	// Create directory text fields.
	hTextDirInput = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", NULL, WS_VISIBLE | WS_CHILD | WS_BORDER | ES_AUTOHSCROLL, 
		10, 25, 
		WINDOW_WID - 30 - 80 - 5 - 10, 20, 
		hwnd, NULL, 0, 0);
	hTextDirOutput = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", NULL, WS_VISIBLE | WS_CHILD | WS_BORDER | ES_AUTOHSCROLL, 
		10, 50 + 25, 
		WINDOW_WID - 30 - 80 - 5 - 10, 20, 
		hwnd, NULL, 0, 0);
	SendMessage(hTextDirInput, EM_SETCUEBANNER, (WPARAM)TRUE, (LPARAM)L"Select a path");
	SendMessage(hTextDirOutput, EM_SETCUEBANNER, (WPARAM)TRUE, (LPARAM)L"Select a path");
	SendMessage(hTextDirInput, EM_SETLIMITTEXT, (WPARAM)MAX_PATH, (LPARAM)0);
	SendMessage(hTextDirOutput, EM_SETLIMITTEXT, (WPARAM)MAX_PATH, (LPARAM)0);

	// The big backup button!
	CreateWindowW(L"BUTTON", L"WRITE TAR+GZIP ARCHIVE", WS_VISIBLE | WS_CHILD, 
		20, 50 + 25 + 20 + 15,
		WINDOW_WID - 20 * 3, 30, 
		hwnd, (HMENU)BTNID_CREATE, 0, 0);

	// Set fonts of all windows.
	bool(CALLBACK *lpSetFonts)(HWND, LPARAM) = [](HWND child, LPARAM font) 
	{
		SendMessage(child, WM_SETFONT, font, true);
		return true;
	};
	EnumChildWindows(hwnd, (WNDENUMPROC)lpSetFonts, (LPARAM)GetStockObject(DEFAULT_GUI_FONT));
}

// Handles buttons that browse folder.
void HandleBrowseButton(HWND hwnd, HWND hTextfield)
{
	BROWSEINFOW bi = { 0 };
	bi.hwndOwner = hwnd;
	bi.pidlRoot = NULL;
	bi.lParam = 0;
	bi.iImage = -1;
	bi.ulFlags = BIF_USENEWUI;

	PIDLIST_ABSOLUTE dir = SHBrowseForFolderW(&bi);
	if (dir == NULL)
	{
		return;
	}
	wchar_t path[MAX_PATH];
	if (!SHGetPathFromIDListW(dir, path))
	{
		return;
	}
	// Don't set if the length is 0.
	if (wcslen(path) == 0)
	{
		return;
	}
	// Set the path as the field's title text.
	SetWindowTextW(hTextfield, path);
}

// Handle creation of the archive.
// This is done via the CLI version of the program
// in the same directory as the GUI.
void HandleArchiveCreate(const wchar_t* indir, const wchar_t* outdir)
{
	// Additional info.
	STARTUPINFOW si;
	PROCESS_INFORMATION pi;

	// Set size of structures.
	ZeroMemory(&si, sizeof si);
	ZeroMemory(&pi, sizeof pi);
	si.cb = sizeof si;

	// Create the command line  args string.
	wchar_t cmdline[MAX_PATH * 2 + 10] = L" -i ";
	wcscat_s(cmdline, indir);
	wcscat_s(cmdline, L" -o ");
	wcscat_s(cmdline, outdir);

	// Get the absolute path of the executable.
	wchar_t pBuffer[MAX_PATH];
	int bytes = GetModuleFileName(NULL, pBuffer, MAX_PATH);
	if (bytes == 0)
	{
		MessageBox(NULL, L"Couldn't get the current directory.\nAborting...", L"Error", MB_ICONERROR);
		return;
	}
	// Convert to directory, and append name of the CLI program.
	size_t cPathLen = wcslen(pBuffer);
	for (size_t i = cPathLen - 1; i > 0; --i)
	{
		if (pBuffer[i] == '\\' || pBuffer[i] == '/')
		{
			pBuffer[i + 1] = 0;
			break;
		}
	}
	wcscat_s(pBuffer, CLI_APP_NAME);
	wcscat_s(pBuffer, cmdline);

	// Start the program.
	if (!CreateProcessW(
		NULL,
		(LPWSTR)pBuffer,
		NULL, NULL, FALSE, 0, NULL, NULL,
		&si, &pi
	))
	{
		if (GetLastError() == 0x02)
		{
			wchar_t errinfo[1024] = L"Couldn't find the CLI program at path '";
			wcscat_s(errinfo, pBuffer);
			wcscat_s(errinfo, L"'\nMake sure the program exists in the same directory as the GUI program.");
			MessageBoxW(NULL, errinfo, L"Error", MB_ICONERROR);
		}
		else
		{
			wchar_t errinfo[1024] = L"Unknown error: ";
			wcscat_s(errinfo, std::to_wstring(GetLastError()).c_str());
			MessageBoxW(NULL, errinfo, L"Error", MB_ICONERROR);
		}
	}

	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);
}

#endif