#pragma comment(linker,"\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#pragma comment(lib, "libcurl")
#pragma comment(lib, "ws2_32")

#include <windows.h>

#define CURL_STATICLIB
#include <curl/curl.h>

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static HWND hEdit1, hEdit2, hButton1, hButton2;
	static HFONT hFont;
	switch (msg)
	{
	case WM_CREATE:
		hFont = CreateFont(32, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, TEXT("Segoe UI"));
		hEdit1 = CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("EDIT"), 0, WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE, 10, 10, 512, 256, hWnd, 0, ((LPCREATESTRUCT)lParam)->hInstance, 0);
		hEdit2 = CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("EDIT"), 0, WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE, 10, 316, 512, 256, hWnd, 0, ((LPCREATESTRUCT)lParam)->hInstance, 0);
		hButton1 = CreateWindow(TEXT("BUTTON"), TEXT("↓エンコード"), WS_CHILD | WS_VISIBLE, 10, 276, 128, 30, hWnd, (HMENU)100, ((LPCREATESTRUCT)lParam)->hInstance, 0);
		hButton2 = CreateWindow(TEXT("BUTTON"), TEXT("↑デコード"), WS_CHILD | WS_VISIBLE, 148, 276, 128, 30, hWnd, (HMENU)101, ((LPCREATESTRUCT)lParam)->hInstance, 0);
		SendMessage(hEdit1, WM_SETFONT, (WPARAM)hFont, 0);
		SendMessage(hEdit2, WM_SETFONT, (WPARAM)hFont, 0);
		SendMessage(hButton1, WM_SETFONT, (WPARAM)hFont, 0);
		SendMessage(hButton2, WM_SETFONT, (WPARAM)hFont, 0);
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case 100:
		{
			const DWORD dwTextLengthW = GetWindowTextLengthW(hEdit1);
			if (dwTextLengthW == 0) break;
			LPWSTR lpszEditTextW = (LPWSTR)GlobalAlloc(GMEM_FIXED, sizeof(WCHAR)*(dwTextLengthW + 1));
			if (lpszEditTextW == NULL) break;
			GetWindowTextW(hEdit1, lpszEditTextW, dwTextLengthW + 1);
			SetWindowTextW(hEdit2, 0);
			{
				const DWORD dwTextLengthA = WideCharToMultiByte(CP_UTF8, 0, lpszEditTextW, -1, 0, 0, 0, 0);
				LPSTR szUTF8TextA = (LPSTR)GlobalAlloc(GMEM_FIXED, dwTextLengthA); // NULL を含んだ文字列バッファを確保
				if (szUTF8TextA) {
					if (WideCharToMultiByte(CP_UTF8, 0, lpszEditTextW, -1, szUTF8TextA, dwTextLengthA, 0, 0)) {
						CURL *curl = curl_easy_init();
						if (curl) {
							char *output = curl_easy_escape(curl, szUTF8TextA, dwTextLengthA - 1);
							if (output) {
								SetWindowTextA(hEdit2, output);
								curl_free(output);
							}
							curl_easy_cleanup(curl);
						}
					}
					GlobalFree(szUTF8TextA);
				}
			}
			GlobalFree(lpszEditTextW);
		}
		break;
		case 101:
		{
			const DWORD dwTextLengthA = GetWindowTextLengthA(hEdit2);
			if (dwTextLengthA == 0) break;
			LPSTR lpszEditTextA = (LPSTR)GlobalAlloc(GMEM_FIXED, dwTextLengthA + 1);
			if (lpszEditTextA == NULL) break;
			GetWindowTextA(hEdit2, lpszEditTextA, dwTextLengthA + 1);
			SetWindowTextA(hEdit1, 0);
			{
				CURL *curl = curl_easy_init();
				if (curl) {
					int nOutputLength = 0;
					char *output = curl_easy_unescape(curl, lpszEditTextA, dwTextLengthA, &nOutputLength);
					if (output) {
						const DWORD iDst = MultiByteToWideChar(CP_UTF8, 0, output, nOutputLength + 1, 0, 0); // NULL 文字を含む
						LPWSTR lpszUnicode = (LPWSTR)GlobalAlloc(0, sizeof(WCHAR) * iDst);
						if (lpszUnicode) {
							if (MultiByteToWideChar(CP_UTF8, 0, output, nOutputLength + 1, lpszUnicode, iDst)) {
								SetWindowTextW(hEdit1, lpszUnicode);
							}
							GlobalFree(lpszUnicode);
						}
						curl_free(output);
					}
					curl_easy_cleanup(curl);
				}
			}
			GlobalFree(lpszEditTextA);
		}
		break;
		}
		break;
	case WM_DESTROY:
		DeleteObject(hFont);
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, msg, wParam, lParam);
	}
	return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPreInst, LPSTR pCmdLine, int nCmdShow)
{
	TCHAR szClassName[] = TEXT("Window");
	MSG msg;
	WNDCLASS wndclass = {
		CS_HREDRAW | CS_VREDRAW,
		WndProc,
		0,
		0,
		hInstance,
		0,
		LoadCursor(0,IDC_ARROW),
		(HBRUSH)(COLOR_WINDOW + 1),
		0,
		szClassName
	};
	RegisterClass(&wndclass);
	RECT rect = { 0,0,532,582 };
	DWORD dwStyle = WS_CAPTION | WS_SYSMENU | WS_CLIPCHILDREN;
	AdjustWindowRect(&rect, dwStyle, FALSE);
	HWND hWnd = CreateWindow(
		szClassName,
		TEXT("libcurl を使った Unicode 文字列の URL エンコード"),
		dwStyle,
		CW_USEDEFAULT,
		0,
		rect.right - rect.left,
		rect.bottom - rect.top,
		0,
		0,
		hInstance,
		0
	);
	ShowWindow(hWnd, SW_SHOWDEFAULT);
	UpdateWindow(hWnd);
	while (GetMessage(&msg, 0, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return (int)msg.wParam;
}
