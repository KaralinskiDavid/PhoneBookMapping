// PhoneBookUI.cpp : Определяет точку входа для приложения.
//
#define _CRT_SECURE_NO_WARNINGS

#include "framework.h"
#include "PhoneBookUI.h"
#include <CommCtrl.h>
#include <vector>
#include <string>
#include <sstream>
#include "PhoneBookLine.h"

#pragma comment(lib, "ComCtl32.Lib")

#define MAX_LOADSTRING 100
#define loadPhoneBook "?loadPhonebook@@YA?AV?$vector@PEAVPhoneBookLine@@V?$allocator@PEAVPhoneBookLine@@@std@@@std@@V?$basic_string@_WU?$char_traits@_W@std@@V?$allocator@_W@2@@2@H@Z"
#define searchByindex "?searchByIndex@@YA?AV?$vector@PEAVPhoneBookLine@@V?$allocator@PEAVPhoneBookLine@@@std@@@std@@V?$basic_string@_WU?$char_traits@_W@std@@V?$allocator@_W@2@@2@HH@Z"
#define destroy "?destroyPhoneBook@@YAXXZ"

HINSTANCE hInst;                                
WCHAR szTitle[MAX_LOADSTRING];                  
WCHAR szWindowClass[MAX_LOADSTRING];            
HWND hMainWindow;
HWND hText;
HWND hwndNavButton1;
HWND hwndNavButton2;
std::wstring ListItem = L"Lastname";
const std::vector<std::wstring> columnsName{ L"Phonenumber", L"Lastname", L"Name", L"Surname", L"Street", L"House", L"Corpus", L"Flat" };
std::wstring path = L"..\\Database.txt";
std::vector<PhoneBookLine*> PhoneBook;
int currentPage = 0;
int currentResultPage = 0;
int currentIndexType = 0;
wstring currentIndexValue;
bool isSearchResult = false;


HMODULE hLib = LoadLibrary(L"PhoneBook.dll");
typedef std::vector<PhoneBookLine*>(*PhL)(std::wstring, int);
typedef std::vector<PhoneBookLine*>(*Srch)(std::wstring, int, int);
typedef void (*Dstr)();
Dstr destroyPhonebook = (Dstr)GetProcAddress(hLib, destroy);
PhL loadDatabase = (PhL)GetProcAddress(hLib, loadPhoneBook);
Srch searchByIndex = (Srch)GetProcAddress(hLib, searchByindex);


ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    setlocale(LC_ALL, "RU");
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_PHONEBOOKUI, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_PHONEBOOKUI));

    MSG msg;

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

HWND CreateListView(HWND hWndParent)
{
    HWND hWndListView;
    INITCOMMONCONTROLSEX icex;
    RECT rcClient;

    icex.dwICC = ICC_LISTVIEW_CLASSES;
    InitCommonControlsEx(&icex);
    GetClientRect(hWndParent, &rcClient);

    hWndListView = CreateWindowEx(NULL, WC_LISTVIEW, L"", WS_CHILD | LVS_REPORT | LVS_EDITLABELS, 0, 40, rcClient.right - rcClient.left, rcClient.bottom - rcClient.top, hWndParent, (HMENU)11111, GetModuleHandle(NULL), NULL);
    return hWndListView;
}

void CreateComboBox(HWND hWndParent)
{
    HWND hWndComboBox = CreateWindow(
        WC_COMBOBOX, TEXT(""),
        CBS_DROPDOWN | CBS_HASSTRINGS | WS_CHILD | WS_OVERLAPPED | WS_VISIBLE,
        5, 10, 250, 200,
        hWndParent,
        NULL,
        GetModuleHandle(NULL),
        NULL);
    ShowWindow(hWndComboBox, SW_SHOWDEFAULT);

    SendMessage(hWndComboBox, (UINT)CB_ADDSTRING, (WPARAM)0, (LPARAM)(L"Lastname"));
    SendMessage(hWndComboBox, (UINT)CB_ADDSTRING, (WPARAM)0, (LPARAM)(L"Street"));
    SendMessage(hWndComboBox, (UINT)CB_ADDSTRING, (WPARAM)0, (LPARAM)(L"Phone Number"));
    SendMessage(hWndComboBox, CB_SETCURSEL, (WPARAM)0, (LPARAM)0);
}

HWND CreateTextBox(HWND hWndParent)
{
    HWND hText = CreateWindow(
        WC_EDIT,
        NULL,
        WS_BORDER | WS_CHILD | WS_VISIBLE | NULL | NULL,
        290, 10, 200, 25,
        hWndParent,
        NULL,
        NULL,
        0);
    ShowWindow(hText, SW_SHOWDEFAULT);
    return hText;
}

void CreateSearchButton(HWND hWndParent)
{
    HWND hwndButton = CreateWindow(
        WC_BUTTON,
        L"Find",
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
        530, 10, 100, 25,
        hWndParent,
        (HMENU)11111,
        (HINSTANCE)GetWindowLongPtr(hWndParent, GWLP_HINSTANCE),
        NULL);
    ShowWindow(hwndButton, SW_SHOWDEFAULT);
}

void CreateShowAllButton(HWND hWndParent)
{
    HWND hwndButton = CreateWindow(
        WC_BUTTON,
        L"Show all",
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
        630, 10, 100, 25,
        hWndParent,
        (HMENU)11112,
        (HINSTANCE)GetWindowLongPtr(hWndParent, GWLP_HINSTANCE),
        NULL);
    ShowWindow(hwndButton, SW_SHOWDEFAULT);
}

void CreateNavigationButtons(HWND hWndParent)
{
    hwndNavButton1 = CreateWindow(
        WC_BUTTON,
        L"<",
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
        1350, 0, 20, 25,
        hWndParent,
        (HMENU)11113,
        (HINSTANCE)GetWindowLongPtr(hWndParent, GWLP_HINSTANCE), NULL);
    hwndNavButton2 = CreateWindow(
        WC_BUTTON,
        L">",
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
        1370, 0, 20, 25, hWndParent,
        (HMENU)11114,
        (HINSTANCE)GetWindowLongPtr(hWndParent, GWLP_HINSTANCE), NULL);
    ShowWindow(hwndNavButton1, SW_SHOWDEFAULT);
    ShowWindow(hwndNavButton2, SW_SHOWDEFAULT);
}

void CreateColumns(HWND hWndListView)
{
    RECT windowRect;
    LVCOLUMN lvc;

    GetClientRect(hMainWindow, &windowRect);
    lvc.mask = LVCF_WIDTH | LVCF_TEXT;
    lvc.cx = 150;
    for (int i = 0; i < columnsName.size(); i++) {
        lvc.pszText = const_cast<LPWSTR>(columnsName[i].c_str());
        ListView_InsertColumn(hWndListView, i, &lvc);
    }
}

std::wstring IntToWStr(int x) {
    std::wstringstream ss;
    ss << x;
    std::wstring str = ss.str();
    return str;
}

LPWSTR LPWSTRConvert(const std::wstring& s)
{
    LPWSTR ws = new wchar_t[s.size() + 1];
    copy(s.begin(), s.end(), ws);
    ws[s.size()] = 0;
    return ws;
}

void FillListView(HWND hWndListView, std::vector<PhoneBookLine*> lines)
{
    ListView_DeleteAllItems(hWndListView);
    LVITEM lvI;

    lvI.pszText = LPSTR_TEXTCALLBACK;
    lvI.mask = LVIF_TEXT;
    lvI.iSubItem = 0;
    for (int i = 0; i < lines.size(); i++) {
        lvI.iItem = i;

        ListView_InsertItem(hWndListView, &lvI);

        ListView_SetItemText(hWndListView, i, 0, LPWSTRConvert(lines[i]->phonenumber));
        ListView_SetItemText(hWndListView, i, 1, LPWSTRConvert(lines[i]->lastname));
        ListView_SetItemText(hWndListView, i, 2, LPWSTRConvert(lines[i]->firstname));
        ListView_SetItemText(hWndListView, i, 3, LPWSTRConvert(lines[i]->surname));
        ListView_SetItemText(hWndListView, i, 4, LPWSTRConvert(lines[i]->street));
        ListView_SetItemText(hWndListView, i, 5, LPWSTRConvert(IntToWStr(lines[i]->house)));
        ListView_SetItemText(hWndListView, i, 6, LPWSTRConvert(IntToWStr(lines[i]->corpus)));
        ListView_SetItemText(hWndListView, i, 7, LPWSTRConvert(IntToWStr(lines[i]->flat)));
    }
}


ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_PHONEBOOKUI));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_PHONEBOOKUI);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}


BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; 

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}


HWND hWndListView;

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_CREATE:
    {
        hWndListView = CreateListView(hWnd);
        CreateColumns(hWndListView);
        PhoneBook = loadDatabase(path, currentPage);
        FillListView(hWndListView, PhoneBook);
        ShowWindow(hWndListView, SW_SHOWDEFAULT);
        CreateComboBox(hWnd);

        hText = CreateTextBox(hWnd);
        CreateSearchButton(hWnd);
        CreateShowAllButton(hWnd);
        CreateNavigationButtons(hWnd);
        EnableWindow(hwndNavButton1, false);
    }
    case WM_COMMAND:
        if (HIWORD(wParam) == CBN_SELCHANGE)
        {
            int ItemIndex = SendMessage((HWND)lParam, (UINT)CB_GETCURSEL, (WPARAM)0, (LPARAM)0);
            SendMessage((HWND)lParam, (UINT)CB_GETLBTEXT, (WPARAM)ItemIndex, (LPARAM)ListItem.c_str());
        }
        if (LOWORD(wParam) == 11111)
        {
            TCHAR buff[1024];
            GetWindowText(hText, buff, 1024);
            ListView_DeleteAllItems(hWndListView);
            currentIndexValue = buff;
            if (lstrcmpW(ListItem.c_str(), L"Phone Number") == 0)
            {
                isSearchResult = true;
                currentIndexType = 2;
                PhoneBook = searchByIndex(currentIndexValue, currentIndexType, currentResultPage);
            }
            if (lstrcmpW(ListItem.c_str(), L"Street") == 0)
            {
                isSearchResult = true;
                currentIndexType = 1;
                PhoneBook = searchByIndex(currentIndexValue, currentIndexType, currentResultPage);
            }
            if (lstrcmpW(ListItem.c_str(), L"Lastname") == 0)
            {
                isSearchResult = true;
                currentIndexType = 0;
                PhoneBook = searchByIndex(currentIndexValue, currentIndexType, currentResultPage);
            }
            EnableWindow(hwndNavButton1, false);
            if(PhoneBook.size()>0)
                EnableWindow(hwndNavButton2, true);
            FillListView(hWndListView, PhoneBook);
            UpdateWindow(hWndListView);
        }
        if (LOWORD(wParam) == 11112)
        {
            PhoneBook = loadDatabase(path, currentPage);
            isSearchResult = false;
            currentResultPage = 0;
            if(currentPage>0)
                EnableWindow(hwndNavButton1, true);
            EnableWindow(hwndNavButton2, true);
            FillListView(hWndListView, PhoneBook);
            UpdateWindow(hWndListView);
        }
        if (LOWORD(wParam) == 11113)
        {
            if (!isSearchResult)
            {
                currentPage--;
                PhoneBook = loadDatabase(path, currentPage);
                if (currentPage == 0)
                    EnableWindow(hwndNavButton1, false);
                EnableWindow(hwndNavButton2, true);
            }
            else
            {
                currentResultPage--;
                PhoneBook = searchByIndex(currentIndexValue, currentIndexType, currentResultPage);
                if (currentResultPage == 0)
                    EnableWindow(hwndNavButton1, false);
                EnableWindow(hwndNavButton2, true);
            }
            FillListView(hWndListView, PhoneBook);
        }
        if (LOWORD(wParam) == 11114)
        {
            if (!isSearchResult)
            {
                currentPage++;
                PhoneBook = loadDatabase(path, currentPage);
                if (PhoneBook.size() == 0)
                {
                    currentPage--;
                    EnableWindow(hwndNavButton2, false);
                    PhoneBook = loadDatabase(path, currentPage);
                }
                if (currentPage > 0)
                    EnableWindow(hwndNavButton1, true);
            }
            else
            {
                currentResultPage++;
                PhoneBook = searchByIndex(currentIndexValue, currentIndexType, currentResultPage);
                if (PhoneBook.size() == 0)
                {
                    currentResultPage--;
                    EnableWindow(hwndNavButton2, false);
                    PhoneBook = searchByIndex(currentIndexValue, currentIndexType, currentResultPage);
                }
                if (currentResultPage > 0)
                    EnableWindow(hwndNavButton1, true);
            }
            FillListView(hWndListView, PhoneBook);
        }
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
    {
        destroyPhonebook();
        PostQuitMessage(0);
        break;
    }
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

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
