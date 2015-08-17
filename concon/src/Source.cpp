#include <regex>

#include <windows.h>

#include <redir/redir.h>

HWND h_edit;
concon::redir_t* redir;

void append_text(const char* str)
{
	std::regex re("\n");
	std::string s = std::regex_replace(str, re, "\r\n");
	int len = GetWindowTextLength(h_edit);
	SendMessage(h_edit, EM_SETSEL, len, len);
	SendMessage(h_edit, EM_REPLACESEL, 0, LPARAM(s.c_str()));
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_CREATE:
	{
		h_edit = CreateWindowEx(0, "EDIT", nullptr, WS_VISIBLE | WS_CHILD | WS_HSCROLL | WS_VSCROLL | ES_MULTILINE | ES_AUTOHSCROLL | ES_AUTOVSCROLL,
			0, 0, 0, 0,
			hWnd, 0, GetModuleHandle(nullptr), nullptr);

		LOGFONT lf = {0};
		GetObject(GetStockObject(DEFAULT_GUI_FONT), sizeof(lf), &lf);
		strncpy(lf.lfFaceName, "Consolas", LF_FACESIZE);
		lf.lfCharSet = DEFAULT_CHARSET;
		lf.lfHeight = -13;
		HFONT h_font = ::CreateFontIndirect(&lf);
		SendMessage(h_edit, WM_SETFONT, WPARAM(h_font), MAKELPARAM(TRUE, 0));

		redir = new concon::redir_t;
		redir->set_callback(append_text);
		redir->open(R"(D:\YangTao\tools\lua-5.3\lua53.exe)", R"(D:\YangTao\tools\lua-5.3)");
		return 0;
	}
	case WM_SIZE:
	{
		int width = LOWORD(lParam);
		int height = HIWORD(lParam);

		MoveWindow(h_edit, 0, 0, width, height, TRUE);
		return 0;
	}
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	MSG msg;
	WNDCLASSEX wce = {0};
	HWND hWnd;

	wce.cbSize = sizeof(wce);
	wce.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wce.hCursor = LoadCursor(NULL, IDC_ARROW);
	wce.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wce.hInstance = hInstance;
	wce.lpfnWndProc = WndProc;
	wce.lpszClassName = "concon";
	wce.style = CS_HREDRAW | CS_VREDRAW;
	if(!RegisterClassEx(&wce))
		return 1;
	hWnd = CreateWindowEx(0, "concon", "concon - Windows Console Container", WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, 500, 400,
		NULL, NULL, hInstance, NULL);
	if(hWnd == NULL)
		return 1;

	ShowWindow(hWnd, nShowCmd);
	UpdateWindow(hWnd);

	while(GetMessage(&msg, NULL, 0, 0)){
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return msg.wParam;
}
