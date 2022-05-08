#pragma once

#include "BaseWin.h"

class QueryWindow : public BaseWin<QueryWindow>
{
public:
	QueryWindow();
	virtual ~QueryWindow();

	bool InitWindow(const std::wstring& winCap, const std::wstring& label, int width, int height);
	void Run();

	void SetLabel(const std::wstring& label) { mLabel = label; }
	std::string GetAnswer() const { return mAnswer; }

public:
	virtual LRESULT OnProcessMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual PCWSTR ClassName() const { return L"Query Window"; }

private:
	std::wstring mWinCaption;
	std::wstring mLabel;
	std::string mAnswer;

	int mWidth;
	int mHeight;
	
	HWND mTextBox;
};