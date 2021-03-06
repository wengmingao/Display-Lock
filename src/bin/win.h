#ifndef WIN_H
#define WIN_H

#include <Windows.h>
//#include "menu.h"

typedef struct PREVIOUSRECT
{
    int width;
    int height;
    int x;
    int y;
} PREVIOUSRECT;

typedef struct WINDOW
{
    char title[500];
    int x;
    int y;
    RECT size;
    BOOL selected;
    HWND hWnd;
} WINDOW;

typedef struct WINDOWLIST
{
    int count;
    WINDOW windows[25];
} WINDOWLIST;


typedef struct Menu
{
    WINDOWLIST windows;
    WINDOW selectedWindow;
    BOOL active;
    int currentSelection;
    BOOL live;
    DWORD mode;
    INPUT_RECORD event;
} Menu;


BOOL CALLBACK EnumWindowsProc(HWND, LPARAM);

void openWindows(WINDOWLIST*);
void clearWindows(WINDOWLIST*);
void getCurrentMousePos(POINT*);
void lockFocused(WINDOW*);
void cursorLock(void*);
BOOL checkClientArea(POINT*, RECT*);
BOOL checkResizeStyle(HWND);
void borderlessWindow(HWND);
void undoborderlessWindow(HWND);
void fullScreen(WINDOW, PREVIOUSRECT*);
void disableFullScreen(WINDOW, PREVIOUSRECT*);
int __stdcall cursorLockEx(void* arguments);

#endif