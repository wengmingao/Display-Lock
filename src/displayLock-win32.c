// displayLock-win32.cpp : Defines the entry point for the application.
//

#include "header.h"
#include "displayLock-win32.h"
#include <commctrl.h>
#include <shellapi.h>


// if build is x86 use regular strings
#ifdef WIN32
#define LOADSTRING LoadString
#else
#define LOADSTRING LoadStringW
#endif

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

HFONT hFont;

Args *ComboBoxArgs;
winArgs *windowArgs;

BOOL init = FALSE;

Menu menu;
HWND hWndListBox;
HWND hwndButtonStart;
HWND hwndButtonStop;

// threads
HANDLE CLIP;
int currentSel;
HANDLE mutex;
HANDLE w_combobox;

BOOL active;

SETTINGS *settings;

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);



int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Place code here.
    // initalize menu
    initMenu(&menu);
    settings = malloc(sizeof(SETTINGS));
    readSettings(settings);
	ComboBoxArgs = malloc(sizeof(Args));

	currentSel = 0;
	ComboBoxArgs->menu = &menu;
	ComboBoxArgs->hWnd = &hWndListBox;
	ComboBoxArgs->mutex = &mutex;
	ComboBoxArgs->currentSel = &currentSel;

	windowArgs = malloc(sizeof(winArgs));

	windowArgs->mutex = &mutex;
	windowArgs->active = &active;
    windowArgs->settings = settings;
	

    //strcpy(settings.header, "DLOCK");
	//settings.minimize = TRUE;


    // Initialize global strings
    LOADSTRING(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LOADSTRING(hInstance, IDC_DISPLAYLOCKWIN32, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_DISPLAYLOCKWIN32));

    MSG msg;

    // Main message loop:
    while (GetMessage(&msg, NULL, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_DISPLAYLOCKWIN32));
    wcex.hCursor        = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_DISPLAYLOCKWIN32);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));


    return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Store instance handle in our global variable

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
      CW_USEDEFAULT, 0, 360, 200, NULL, NULL, hInstance, NULL);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	char selector[500];
    GetWindowTextA(hWndListBox, selector, 500);

    switch (message)
    {
    case WM_CREATE:
    {
        hFont = CreateFont(18, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, ANSI_CHARSET,
            OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
            DEFAULT_PITCH | FF_DONTCARE, TEXT("Calibri"));

        SendMessage(hWnd, WM_SETFONT, (WPARAM)hFont, TRUE);

    	// create dropdown menu
        hWndListBox = CreateWindowEx(
            WS_EX_CLIENTEDGE,
            TEXT("COMBOBOX"),   // Predefined class; Unicode assumed
            NULL,
            WS_VISIBLE | WS_CHILD | CBS_DROPDOWNLIST | CBS_HASSTRINGS | CBS_AUTOHSCROLL | CBS_DISABLENOSCROLL | WS_VSCROLL,
            20,     // x position 
            20,     // y position 
            300,    // Button width
            200,    // Button height
            hWnd,   // Parent window
            (HMENU)IDC_DROPDOWNLIST,
            NULL,
            NULL);

        // start button
        hwndButtonStart = CreateWindowEx(
            WS_EX_WINDOWEDGE,
            TEXT("BUTTON"),  // Predefined class; Unicode assumed 
            TEXT("Start"),      // Button text 
            WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,  // Styles 
            110,         // x position 
            60,         // y position 
            100,        // Button width
            35,        // Button height
            hWnd,     // Parent window
            (HMENU)IDC_OPBUTTON_START,       // No menu.
            NULL,
            NULL); // Pointer not needed.
        
        // stop button
        hwndButtonStop = CreateWindowEx(
            WS_EX_WINDOWEDGE,
            TEXT("BUTTON"),  // Predefined class; Unicode assumed 
            TEXT("Stop"),      // Button text 
            WS_DISABLED | WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,  // Styles 
            220,         // x position 
            60,         // y position 
            100,        // Button width
            35,        // Button height
            hWnd,     // Parent window
            (HMENU)IDC_OPBUTTON_STOP,       // No menu.
            NULL,
            NULL); // Pointer not needed.

        SendMessage(hwndButtonStart, WM_SETFONT, (WPARAM)hFont, TRUE);
        SendMessage(hwndButtonStop, WM_SETFONT, (WPARAM)hFont, TRUE);
        SendMessage(hWndListBox, WM_SETFONT, (WPARAM)hFont, TRUE);
        SendMessage(hWndListBox, CB_SETCURSEL, 0, 0);
        
        break;
    }
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Parse the menu selections:
            switch (wmId)
            {
			case IDC_DROPDOWNLIST:
			{
				switch(HIWORD(wParam))
				{
				case CBN_DROPDOWN:
				{
					SendMessage((HWND)lParam, CB_RESETCONTENT, (WPARAM)NULL, 0); // clear combo box
					updateComboBox(ComboBoxArgs);

					// write to combo box
					for (int i = 0; i < ComboBoxArgs->menu->windows.count; i++)
					{
						char foo[500];
						strcpy_s(foo, 500, ComboBoxArgs->menu->windows.windows[i].title);
						SendMessageA((HWND)lParam, CB_ADDSTRING, (WPARAM)NULL, (LPARAM)foo);
					}

					break;
				}
				default:
					break;
				}
				break;
			}
			// on startbutton click
            case IDC_OPBUTTON_START:
            {
                if(!menu.active && strcmp(selector, "") != 0)
                {
                    WaitForSingleObject(&ComboBoxArgs->mutex, INFINITE);    // wait for mutex

                    ComboBoxArgs->mutex = CreateMutex(NULL, FALSE, NULL);   // lock the mutex
					// set active to true
                    *(windowArgs->active) = TRUE;
                    ComboBoxArgs->menu->active = TRUE;
                    menu.active = TRUE;

					// lock the listbox and start button, enable the stop button
                    EnableWindow(hWndListBox, FALSE);
                    EnableWindow(hwndButtonStop, TRUE);
                    EnableWindow(hwndButtonStart, FALSE);

                    EnableMenuItem(GetMenu(hWnd), ID_FILE_SETTINGS, MF_DISABLED | MF_GRAYED);

					// sort the window
                    windowArgs->window = sortWindow(ComboBoxArgs, selector, menu.windows.count);
                    active = TRUE;

                    if(windowArgs->window != NULL)
                    {
                        ReleaseMutex(&ComboBoxArgs->mutex);
                        CLIP = (HANDLE)_beginthreadex(NULL, 0, cursorLockEx, (void*)windowArgs, 0, NULL); // begin multi thread
                    }

					// minimize window when start is pressed if setting is enabled
                    if(settings->minimize)
                        ShowWindow(hWnd, SW_MINIMIZE);

                }
                
                break;
            }
            case IDC_OPBUTTON_STOP:
            {
				// only if function is active
                if (menu.active)
                {
					// set active to false
                    menu.active = FALSE;
                    *(windowArgs->active) = FALSE;
                    active = FALSE;
                    ComboBoxArgs->mutex = CreateMutex(NULL, FALSE, NULL);   // lock the mutex

					// toggle active buttons
                    EnableWindow(hwndButtonStop, FALSE);
                    EnableWindow(hwndButtonStart, TRUE);

                    ReleaseMutex(&ComboBoxArgs->mutex);

                    EnableMenuItem(GetMenu(hWnd), ID_FILE_SETTINGS, 0);

					// wait for CLIP to end
					WaitForSingleObject(CLIP, INFINITE);
                    EnableWindow(hWndListBox, TRUE);
					// safely close the handle
                    CloseHandle(CLIP);
                }
                break;
            }
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
			case ID_FILE_SETTINGS:
				DialogBox(hInst, MAKEINTRESOURCE(IDD_SETTINGS), hWnd, Settings);
				break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: Add any drawing code that uses hdc here...
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
		destroyWin();	// free's memory and ends threads safely
        DeleteObject(hFont);
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_NOTIFY:
        if (((LPNMHDR)lParam)->code == NM_CLICK)
            ShellExecuteW(NULL, TEXT("open"), TEXT("https://github.com/idietmoran/Display-Lock"), NULL, NULL, SW_SHOWNORMAL);
        break;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        } 
        
        break;
	default:
		break;
    }
    return (INT_PTR)FALSE;
}

// Message handler for settings box.
INT_PTR CALLBACK Settings(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		initalizeSettings(hDlg, settings);
		return (INT_PTR)TRUE;

	case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case ID_SETTINGS_OK:
        {
            updateSettings(hDlg, settings);
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        case ID_SETTINGS_CANCEL:
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        default:
            break;
        }
	default:
		break;
	}
	return (INT_PTR)FALSE;
}

// cleans up variables safely
void destroyWin()
{
    writeSettings(*settings);
	// wait for mutex
	WaitForSingleObject(&ComboBoxArgs->mutex, INFINITE);

	ComboBoxArgs->mutex = CreateMutex(NULL, FALSE, NULL);   // lock the mutex
	// set all thread variables to false
	menu.live = FALSE;
	*(windowArgs->active) = FALSE;
	ReleaseMutex(&ComboBoxArgs->mutex);	// unlock mutex

	// wait for threads to shut down safely
	WaitForSingleObject(CLIP, INFINITE);


	// free memory
    free(settings);
	free(ComboBoxArgs);
	free(windowArgs);

    if (CLIP != NULL)
        CloseHandle(CLIP);
}