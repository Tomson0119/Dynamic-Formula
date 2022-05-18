#include "stdafx.h"
#include "queryWindow.h"

QueryWindow::QueryWindow()
	: mTextBox{}
{
}

QueryWindow::~QueryWindow()
{
}

bool QueryWindow::InitWindow(const std::wstring& winCap, const std::wstring& label, int width, int height)
{
	SetLabel(label);
	mWinCaption = winCap;
	mWidth = width;
	mHeight = height;

	if (!Window::Init(width, height, winCap.c_str(), L"Query Window"))
		return false;
	return true;
}

void QueryWindow::Run()
{
	ShowWindow();

	MSG msg{};

	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
}

void QueryWindow::QuitWindow()
{
	mAnswer.resize(GetWindowTextLength(mTextBox) + 1);
	int len = GetWindowTextA(mTextBox, (LPSTR)mAnswer.c_str(), (int)mAnswer.size());
	if (len != 0)
	{
		PostMessage(mHwnd, WM_CLOSE, NULL, NULL);
	}
}

LRESULT QueryWindow::OnProcessMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps{};
	HDC hdc{};
	HFONT font{}, oldfont{};

	switch (uMsg)
	{
	case WM_CREATE:
		mTextBox = CreateWindow(L"EDIT", L"", WS_BORDER | WS_CHILD | WS_VISIBLE,
			60, 120, 400, 30, mHwnd, 0, 0, 0);
		CreateWindow(L"BUTTON", L"Done", WS_VISIBLE | WS_CHILD | WS_BORDER,
			480, 120, 70, 30, mHwnd, (HMENU)1, 0, 0);
		break;
	case WM_PAINT:	
		hdc = BeginPaint(mHwnd, &ps);
		font = CreateFont(30, 0, 0, 0, FW_BOLD, 0, 0, 0, ANSI_CHARSET, 0, 0, 0, 0, NULL);
		oldfont = (HFONT)SelectObject(hdc, font);
		TextOut(hdc, mWidth/2 - 100, mHeight/3, mLabel.c_str(), (int)mLabel.size());
		EndPaint(mHwnd, &ps);
		break;

	case WM_COMMAND:
		if (LOWORD(wParam) == 1)
		{
			QuitWindow();
		}
		break;

	case WM_KEYUP:
		if (wParam == VK_ESCAPE)
		{
			QuitWindow();
		}
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;

	default:
		return DefWindowProc(mHwnd, uMsg, wParam, lParam);
	}
	return 0;
}
