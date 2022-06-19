#pragma once

#include "window.h"

class QueryWindow : public Window<QueryWindow>
{
public:
	QueryWindow();
	virtual ~QueryWindow();

	bool InitWindow(const std::wstring& winCap, const std::wstring& label, int width, int height);
	void Run();

	void SetLabel(const std::wstring& label) { mLabel = label; }
	std::string GetAnswer() const { return mAnswer; }

private:
	void SetAnswer(bool local=false);
	void QuitWindow();

public:
	virtual LRESULT OnProcessMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);

private:
	std::wstring mWinCaption;
	std::wstring mLabel;
	std::string mAnswer;

	int mWidth;
	int mHeight;
	
	HWND mTextBox;
};