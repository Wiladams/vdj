#pragma once


// Keep a list of currently active windows
// create screensnapshots of ones we want
#include "apphost.h"

#include <vector>

struct WindowCaptures
{
	static BOOL CALLBACK enumWins(HWND hWnd, LPARAM lParam)
	{
		if (::IsWindowVisible(hWnd))
		{
			std::vector<HWND>* handles = (std::vector<HWND>*)lParam;
			handles->push_back(hWnd);
			// GetWindowText - for titles
			char title[256];
			auto len = ::GetWindowTextA(hWnd, title, 256);
			if (len > 0)
				printf("Title: %s\n", title);
			
			RECT aRect;
			GetWindowRect(hWnd, &aRect);
			//printf("== enumwins %d %d %d %d ==\n", aRect.left, aRect.top, aRect.right, aRect.bottom);
			printf("== enumwins %d %d %d %d ==\n", aRect.left, aRect.top, aRect.right-aRect.left, aRect.bottom-aRect.top);

		}

		return TRUE;
	}

	// Enumerate all the windows, putting them into a vector
	// for later usage.
	static void getVisibleWindowHandles(std::vector<HWND> &handles)
	{
		auto bResult = EnumWindows(enumWins, (LPARAM)&handles);
	}
};