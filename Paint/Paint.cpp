// Paint.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "Paint.h"

//-------------------------------------------------------
#include<Windowsx.h>
#include<CommCtrl.h>
#pragma comment(lib, "Comctl32.lib")
#include <commdlg.h>
//To use vector
#include <vector>
#include <fstream>
#include <string>
#include <iostream>

//To use vector
using namespace std;

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

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

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_PAINT, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_PAINT));

    MSG msg;

    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}

//-------------------------------------------------------------
BOOL OnCreate(HWND hwnd, LPCREATESTRUCT lpCreateStruct);
void OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);
void OnDestroy(HWND hwnd);
void OnPaint(HWND hwnd);
void OnLButtonDown(HWND hwnd, BOOL fDoubleClick, int x, int y, UINT keyFlags);
void OnLButtonUp(HWND hwnd, int x, int y, UINT keyFlags);
void OnMouseMove(HWND hwnd, int x, int y, UINT keyFlags);

//To store lines
vector<int> LfromXG;
vector<int> LfromYG;
vector<int> LtoXG;
vector<int> LtoYG;

//To store rectanges
vector<int> RfromXG;
vector<int> RfromYG;
vector<int> RtoXG;
vector<int> RtoYG;

//To store ellipses
vector<int> EfromXG;
vector<int> EfromYG;
vector<int> EtoXG;
vector<int> EtoYG;

//To store points
vector<int> Px;
vector<int> Py;

//To get current first and last point
int fromX;
int fromY;
int toX;
int toY;
//Virtual image
bool isPreview;

//Case for the tools
#define LINE         1
#define RECTANGLE    2
#define ELLIPSE      3
#define POINT        4

//Default = POINT
int theSwitch = POINT;

//Define for the image
#define IMAGE_WIDTH     18
#define IMAGE_HEIGHT    18
#define BUTTON_WIDTH    0
#define BUTTON_HEIGHT   0

//Select the color for the brush and like all the panel and stuff
CHOOSECOLOR cc;     //Info of the selected color
COLORREF acrCustClr[16];    //Custom color array
DWORD rgbCurrent = RGB(255, 0, 0);  //Red
HBRUSH hbrush;  //Create the brush from the choosen color

class Tokenizer
{
public:
    static vector<string> split(string haystack, string seperator = " ")
    {
        vector<string> tokens;

        int startPos = 0;
        size_t foundPos = 0;

        while (true)
        {
            foundPos = haystack.find(seperator, startPos);

            if (foundPos != string::npos)
            {
                string substring = haystack.substr(startPos, foundPos);
                tokens.push_back(substring);

                startPos = foundPos + seperator.length();
            }
            else
            {
                string substring = haystack.substr(startPos, haystack.length() - startPos);
                tokens.push_back(substring);
                break;
            }
        }

        return tokens;
    }
};

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
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_PAINT));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_BTNFACE+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_PAINT);
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

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      250, 100, 800, 550, nullptr, nullptr, hInstance, nullptr);

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
//  PURPOSE: Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
        HANDLE_MSG(hWnd, WM_CREATE, OnCreate);
        HANDLE_MSG(hWnd, WM_COMMAND, OnCommand);
        HANDLE_MSG(hWnd, WM_DESTROY, OnDestroy);
        HANDLE_MSG(hWnd, WM_PAINT, OnPaint);
        HANDLE_MSG(hWnd, WM_LBUTTONDOWN, OnLButtonDown);
        HANDLE_MSG(hWnd, WM_LBUTTONUP, OnLButtonUp);
        HANDLE_MSG(hWnd, WM_MOUSEMOVE, OnMouseMove);

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

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

//----------------------------------------------
BOOL OnCreate(HWND hwnd, LPCREATESTRUCT lpCreateStruct)
{
    //Get the system font
    LOGFONT lf;
    GetObject(GetStockObject(DEFAULT_GUI_FONT), sizeof(LOGFONT), &lf);
    HFONT hFont = CreateFont(lf.lfHeight, lf.lfWidth, lf.lfEscapement,
        lf.lfOrientation, lf.lfWeight, lf.lfItalic,
        lf.lfUnderline, lf.lfStrikeOut, lf.lfCharSet,
        lf.lfOutPrecision, lf.lfClipPrecision, lf.lfQuality,
        lf.lfPitchAndFamily, lf.lfFaceName);

    //Common buttons
    InitCommonControls();
    TBBUTTON tbButtons[] =
    {
        { STD_FILENEW, ID_FILE_NEW, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0},
        { STD_FILEOPEN, ID_FILE_OPEN, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0},
        { STD_FILESAVE, ID_FILE_SAVE, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0}
    };

    //Add them in
    HWND hToolBarWnd = CreateToolbarEx(hwnd,
        WS_CHILD | WS_VISIBLE | CCS_ADJUSTABLE | TBSTYLE_TOOLTIPS,
        ID_TOOLBAR, sizeof(tbButtons) / sizeof(TBBUTTON), HINST_COMMCTRL,
        0, tbButtons, sizeof(tbButtons) / sizeof(TBBUTTON),
        BUTTON_WIDTH, BUTTON_HEIGHT, IMAGE_WIDTH, IMAGE_HEIGHT, sizeof(TBBUTTON));

    //Edit buttons
    TBBUTTON buttonsToAdd[] =
    {
        { 0, 0, TBSTATE_ENABLED, TBSTYLE_SEP, 0, 0},    //A blank button to space the default buttons and added buttons
        { STD_CUT, ID_EDIT_CUT, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0},
        { STD_COPY, ID_EDIT_COPY, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0},
        { STD_PASTE, ID_EDIT_PASTE, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0},
        { STD_DELETE, ID_EDIT_DELETE, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0}
    };

    //Add them in
    SendMessage(hToolBarWnd, TB_ADDBUTTONS, (WPARAM)sizeof(buttonsToAdd) / sizeof(TBBUTTON),
        (LPARAM)(LPTBBUTTON)&buttonsToAdd);

    //Custom buttons
    TBBUTTON userButtons[] =
    {
        { 0, 0, TBSTATE_ENABLED, TBSTYLE_SEP, 0 ,0},
        { 0, ID_DRAW_POINT, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0},
        { 1, ID_DRAW_LINE, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0},
        { 2, ID_DRAW_ELLIPSE, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0},
        { 3, ID_DRAW_RECTANGLE, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0}
    };

    //Take the image id and add in i guess?
    TBADDBITMAP tbBitmap = { hInst, IDB_BITMAP5 };
    //Add the image to toolbar
    int idx = SendMessage(hToolBarWnd, TB_ADDBITMAP, (WPARAM)sizeof(tbBitmap) / sizeof(TBADDBITMAP),
        (LPARAM)(LPTBADDBITMAP)&tbBitmap);
    //Find and add the index from the tool.bmp image to several buttons
    userButtons[1].iBitmap += idx;
    userButtons[2].iBitmap += idx;
    userButtons[3].iBitmap += idx;
    userButtons[4].iBitmap += idx;
    //Add the buttons to toolbar
    SendMessage(hToolBarWnd, TB_ADDBUTTONS, (WPARAM)sizeof(userButtons) / sizeof(TBBUTTON),
        (LPARAM)(LPTBBUTTON)&userButtons);

    return TRUE;
}

void OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
    int size;
    string buffer;
    vector<string> tokens;
    ofstream out;
    ifstream in;
    switch (id)
    {
    case ID_CHOOSE_COLOR:   //Click the choose -> color -> choose a color for the brush
        ZeroMemory(&cc, sizeof(CHOOSECOLOR));
        cc.lStructSize = sizeof(CHOOSECOLOR);
        cc.hwndOwner = hwnd;
        cc.lpCustColors = (LPDWORD)acrCustClr;
        cc.rgbResult = rgbCurrent;
        cc.Flags = CC_FULLOPEN | CC_RGBINIT;
        
        if (ChooseColor(&cc))
        {
            hbrush = CreateSolidBrush(cc.rgbResult);
            rgbCurrent = cc.rgbResult;
        }
        break;

    case ID_DRAW_LINE:  //Click the line icon in tool
        theSwitch = LINE;   //switch to draw line
        break;

    case ID_DRAW_RECTANGLE:     //Click the rectangle icon in tool
        theSwitch = RECTANGLE;  //switch to draw rectangle
        break;

    case ID_DRAW_ELLIPSE:   //Click the ellipse icon in tool
        theSwitch = ELLIPSE;    //switch to draw ellipse
        break;

    case ID_DRAW_POINT:
        theSwitch = POINT;
        break;

    case ID_FILE_NEW:   //Clear all the vector -> clear the screen

        //line
        LfromXG.clear();
        LfromYG.clear();
        LtoXG.clear();
        LtoYG.clear();

        //rectangle
        RfromXG.clear();
        RfromYG.clear();
        RtoXG.clear();
        RtoYG.clear();

        //ellipse
        EfromXG.clear();
        EfromYG.clear();
        EtoXG.clear();
        EtoYG.clear();

        //point
        Px.clear();
        Py.clear();

        //Clear screen
        InvalidateRect(hwnd, NULL, TRUE);

        break;

    case ID_FILE_OPEN:  //Load the file
        in.open("out.txt", ios::in);
        //before open file - empty all the vector

        //line
        LfromXG.clear();
        LfromYG.clear();
        LtoXG.clear();
        LtoYG.clear();

        //rectangle
        RfromXG.clear();
        RfromYG.clear();
        RtoXG.clear();
        RtoYG.clear();

        //ellipse
        EfromXG.clear();
        EfromYG.clear();
        EtoXG.clear();
        EtoYG.clear();

        //point
        Px.clear();
        Py.clear();

        //Clear screen
        InvalidateRect(hwnd, NULL, TRUE);

        //line
        getline(in, buffer);
        size = stoi(buffer);

        if (size > 0)
        {
            //from x
            getline(in, buffer);
            tokens = Tokenizer::split(buffer, ",");
            for (int i = 0; i < tokens.size() - 1; i++)
            {
                LfromXG.push_back(stoi(tokens[i]));
            }

            //from y
            getline(in, buffer);
            tokens = Tokenizer::split(buffer, ",");
            for (int i = 0; i < tokens.size() - 1; i++)
            {
                LfromYG.push_back(stoi(tokens[i]));
            }

            //to x
            getline(in, buffer);
            tokens = Tokenizer::split(buffer, ",");
            for (int i = 0; i < tokens.size() - 1; i++)
            {
                LtoXG.push_back(stoi(tokens[i]));
            }

            //to y
            getline(in, buffer);
            tokens = Tokenizer::split(buffer, ",");
            for (int i = 0; i < tokens.size() - 1; i++)
            {
                LtoYG.push_back(stoi(tokens[i]));
            }
        }
        else
        {
            getline(in, buffer);
            getline(in, buffer);
            getline(in, buffer);
            getline(in, buffer);
        }
       
        //rectangle
        getline(in, buffer);
        size = stoi(buffer);

        if (size > 0)
        {
            //from x
            getline(in, buffer);
            tokens = Tokenizer::split(buffer, ",");
            for (int i = 0; i < tokens.size() - 1; i++)
            {
                RfromXG.push_back(stoi(tokens[i]));
            }

            //from y
            getline(in, buffer);
            tokens = Tokenizer::split(buffer, ",");
            for (int i = 0; i < tokens.size() - 1; i++)
            {
                RfromYG.push_back(stoi(tokens[i]));
            }

            //to x
            getline(in, buffer);
            tokens = Tokenizer::split(buffer, ",");
            for (int i = 0; i < tokens.size() - 1; i++)
            {
                RtoXG.push_back(stoi(tokens[i]));
            }

            //to y
            getline(in, buffer);
            tokens = Tokenizer::split(buffer, ",");
            for (int i = 0; i < tokens.size() - 1; i++)
            {
                RtoYG.push_back(stoi(tokens[i]));
            }
        }
        else
        {
            getline(in, buffer);
            getline(in, buffer);
            getline(in, buffer);
            getline(in, buffer);
        }

        //ellipse
        getline(in, buffer);
        size = stoi(buffer);

        if (size > 0)
        {
            //from x
            getline(in, buffer);
            tokens = Tokenizer::split(buffer, ",");
            for (int i = 0; i < tokens.size() - 1; i++)
            {
                EfromXG.push_back(stoi(tokens[i]));
            }

            //from y
            getline(in, buffer);
            tokens = Tokenizer::split(buffer, ",");
            for (int i = 0; i < tokens.size() - 1; i++)
            {
                EfromYG.push_back(stoi(tokens[i]));
            }

            //to x
            getline(in, buffer);
            tokens = Tokenizer::split(buffer, ",");
            for (int i = 0; i < tokens.size() - 1; i++)
            {
                EtoXG.push_back(stoi(tokens[i]));
            }

            //to y
            getline(in, buffer);
            tokens = Tokenizer::split(buffer, ",");
            for (int i = 0; i < tokens.size() - 1; i++)
            {
                EtoYG.push_back(stoi(tokens[i]));
            }
        }
        else
        {
            getline(in, buffer);
            getline(in, buffer);
            getline(in, buffer);
            getline(in, buffer);
        }

        //line
        getline(in, buffer);
        size = stoi(buffer);

        if (size > 0)
        {
            //x
            getline(in, buffer);
            tokens = Tokenizer::split(buffer, ",");
            for (int i = 0; i < tokens.size() - 1; i++)
            {
                Px.push_back(stoi(tokens[i]));
            }

            //y
            getline(in, buffer);
            tokens = Tokenizer::split(buffer, ",");
            for (int i = 0; i < tokens.size() - 1; i++)
            {
                Py.push_back(stoi(tokens[i]));
            }
        }

        in.close();
        break;

    case ID_FILE_SAVE:  //Save all the vector -> txt file
        out.open("out.txt", ios::out);
        //line
        size = LfromXG.size();
        //size of line first
        out << size << endl;
        //from x
        for (int i = 0; i < size; i++)
        {
            out << LfromXG[i] << ",";
        }
        out << endl;

        //from y
        for (int i = 0; i < size; i++)
        {
            out << LfromYG[i] << ",";
        }
        out << endl;

        //to x
        for (int i = 0; i < size; i++)
        {
            out << LtoXG[i] << ",";
        }
        out << endl;

        //to y
        for (int i = 0; i < size; i++)
        {
            out << LtoYG[i] << ",";
        }
        out << endl;

        //rectangle
        size = RfromXG.size();
        //size of rectangle first
        out << size << endl;
        //from x
        for (int i = 0; i < size; i++)
        {
            out << RfromXG[i] << ",";
        }
        out << endl;

        //from y
        for (int i = 0; i < size; i++)
        {
            out << RfromYG[i] << ",";
        }
        out << endl;

        //to x
        for (int i = 0; i < size; i++)
        {
            out << RtoXG[i] << ",";
        }
        out << endl;

        //to y
        for (int i = 0; i < size; i++)
        {
            out << RtoYG[i] << ",";
        }
        out << endl;

        //ellipse
        size = EfromXG.size();
        //size of ellipse first
        out << size << endl;
        //from x
        for (int i = 0; i < size; i++)
        {
            out << EfromXG[i] << ",";
        }
        out << endl;

        //from y
        for (int i = 0; i < size; i++)
        {
            out << EfromYG[i] << ",";
        }
        out << endl;

        //to x
        for (int i = 0; i < size; i++)
        {
            out << EtoXG[i] << ",";
        }
        out << endl;

        //to y
        for (int i = 0; i < size; i++)
        {
            out << EtoYG[i] << ",";
        }
        out << endl;

        //point
        size = Px.size();
        //size of point first
        out << size << endl;
        //x
        for (int i = 0; i < size; i++)
        {
            out << Px[i] << ",";
        }
        out << endl;

        //y
        for (int i = 0; i < size; i++)
        {
            out << Py[i] << ",";
        }

        out.close();
        break;

    //case ID_CHOOSE_FONT:
    //    CHOOSEFONT cf;
    //    LOGFONT lf;
    //    HFONT hfNew, hfOld;
    //    ZeroMemory(&cf, sizeof(CHOOSEFONT));
    //    cf.lStructSize = sizeof(CHOOSEFONT);
    //    cf.hwndOwner = hwnd;
    //    cf.lpLogFont = &lf;
    //    cf.Flags = CF_SCREENFONTS | CF_EFFECTS;

    //    if (ChooseFont(&cf) == TRUE)
    //    {
    //        hfont = CreateFontIndirect(cf.lpLogFont);
    //        hfontPrev = SelectObject(hdc, hfont);
    //        rgbCurrent = cf.rgbColors;
    //        rgbPrev = SetTextColor(hdc, rgbCurrent);
    //    }
    }
}

void OnDestroy(HWND hwnd)
{
    PostQuitMessage(0);
}

void OnPaint(HWND hwnd)
{
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(hwnd, &ps);

    //Create a pen - type = dot - thickness = 3 - color = red
    HPEN hPen = CreatePen(PS_DASHDOT, 3, rgbCurrent);
    SelectObject(hdc, hPen);

    //Switch to determine what shape to draw
    switch (theSwitch)
    {
    case LINE:  //draw a line
        MoveToEx(hdc, fromX, fromY, NULL);
        LineTo(hdc, toX, toY);
        break;
        
    case RECTANGLE: //draw a rectangle
        Rectangle(hdc, fromX, fromY, toX, toY);
        break;

    case ELLIPSE:   //draw an ellipse
        Ellipse(hdc, fromX, fromY, toX, toY);
        break;

    case POINT:
        SetPixel(hdc, fromX, fromY, rgbCurrent);
    }

    //redraw all the shape
    
    //rectangle
    for (int i = 0; i < RfromXG.size(); i++)
    {
        Rectangle(hdc, RfromXG[i], RfromYG[i], RtoXG[i], RtoYG[i]);
    }

    //ellipse
    for (int i = 0; i < EfromXG.size(); i++)
    {
        Ellipse(hdc, EfromXG[i], EfromYG[i], EtoXG[i], EtoYG[i]);
    }

    //line
    for (int i = 0; i < LfromXG.size(); i++)
    {
        MoveToEx(hdc, LfromXG[i], LfromYG[i], NULL);
        LineTo(hdc, LtoXG[i], LtoYG[i]);
    }

    //point
    for (int i = 0; i < Px.size(); i++)
    {
        SetPixel(hdc, Px[i], Py[i], rgbCurrent);
    }

    EndPaint(hwnd, &ps);
}

void OnLButtonDown(HWND hwnd, BOOL fDoubleClick, int x, int y, UINT keyFlags)
{
    isPreview = true;
    fromX = x;
    fromY = y;
    HDC hdc = GetDC(hwnd);

    //when draw a line
    if (theSwitch == POINT)
    {
        SetPixel(hdc, x, y, rgbCurrent);
    }
}

void OnLButtonUp(HWND hwnd, int x, int y, UINT keyFlags)
{
    isPreview = false;

    //Indicator to clear screen and redraw
    InvalidateRect(hwnd, NULL, TRUE);

    //depend on the switch to determine what shape and where to store the shape
    switch (theSwitch)
    {
    case LINE:
        LfromXG.push_back(fromX);
        LfromYG.push_back(fromY);
        LtoXG.push_back(toX);
        LtoYG.push_back(toY);
        break;

    case RECTANGLE:
        RfromXG.push_back(fromX);
        RfromYG.push_back(fromY);
        RtoXG.push_back(toX);
        RtoYG.push_back(toY);
        break;

    case ELLIPSE:
        EfromXG.push_back(fromX);
        EfromYG.push_back(fromY);
        EtoXG.push_back(toX);
        EtoYG.push_back(toY);
        break;

    case POINT:
        Px.push_back(fromX);
        Py.push_back(fromY);
        break;
    }
}

void OnMouseMove(HWND hwnd, int x, int y, UINT keyFlags)
{
    if (isPreview)  //when the left mouse is still being clicked
    {
        toX = x;
        toY = y;

        //Indicator to clear screen
        InvalidateRect(hwnd, NULL, TRUE);
    }
}