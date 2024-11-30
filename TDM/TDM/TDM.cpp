// TDM.cpp : 애플리케이션에 대한 진입점을 정의합니다.
//

#include "framework.h"
#include "TDM.h"
#include "Func.h"

#include <windows.h>
#include <windowsx.h>
#include <memory>
#include <string>

#define CELL_SIZE 30
#define BOARD_SIZE 9
#define LAYER_SIZE 3
#define LAYER_SPACING (CELL_SIZE / 2)  // 레이어 간 간격
#define WINDOW_WIDTH 900
#define WINDOW_HEIGHT 900

#define MAX_LOADSTRING 100

// 전역 변수:
HINSTANCE hInst;                                // 현재 인스턴스입니다.
WCHAR szTitle[MAX_LOADSTRING];                  // 제목 표시줄 텍스트입니다.
WCHAR szWindowClass[MAX_LOADSTRING];            // 기본 창 클래스 이름입니다.

// 이 코드 모듈에 포함된 함수의 선언을 전달합니다:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

std::unique_ptr<Func> gameLogic;
void DrawBoard(HDC hdc);
void HandleMouseMove(HWND hWnd, int x, int y);
bool firstClick = true;
POINT g_mousePos = { -1, -1 };
POINT g_prevMousePos = { -1, -1 };

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: 여기에 코드를 입력합니다.

    // 전역 문자열을 초기화합니다.
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_TDM, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // 애플리케이션 초기화를 수행합니다:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_TDM));

    MSG msg;

    // 기본 메시지 루프입니다:
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



//
//  함수: MyRegisterClass()
//
//  용도: 창 클래스를 등록합니다.
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
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_TDM));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_TDM);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   함수: InitInstance(HINSTANCE, int)
//
//   용도: 인스턴스 핸들을 저장하고 주 창을 만듭니다.
//
//   주석:
//
//        이 함수를 통해 인스턴스 핸들을 전역 변수에 저장하고
//        주 프로그램 창을 만든 다음 표시합니다.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // 인스턴스 핸들을 전역 변수에 저장합니다.

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   gameLogic = std::make_unique<Func>();
   gameLogic->placeMines();
   gameLogic->calcStuckMines();


   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  함수: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  용도: 주 창의 메시지를 처리합니다.
//
//  WM_COMMAND  - 애플리케이션 메뉴를 처리합니다.
//  WM_PAINT    - 주 창을 그립니다.
//  WM_DESTROY  - 종료 메시지를 게시하고 반환합니다.
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_CREATE:
    {
        RECT rc;
        rc.left = 0;
        rc.top = 0;
        rc.right = WINDOW_WIDTH;
        rc.bottom = WINDOW_HEIGHT;
        AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, TRUE);
        SetWindowPos(hWnd, NULL, 0, 0, rc.right - rc.left, rc.bottom - rc.top, SWP_NOZORDER | SWP_NOMOVE);
        break;
    }
    case WM_MOUSEMOVE:
    {
        int xPos = GET_X_LPARAM(lParam);
        int yPos = GET_Y_LPARAM(lParam);
        HandleMouseMove(hWnd, xPos, yPos);
        break;
    }
    case WM_LBUTTONDOWN:
    case WM_RBUTTONDOWN:
    {
        int xPos = LOWORD(lParam);
        int yPos = HIWORD(lParam);
        POINT pt = { xPos, yPos };

        for (int layer = 0; layer < 9; layer++)
        {
            int layerX = (layer % LAYER_SIZE) * (BOARD_SIZE * CELL_SIZE + LAYER_SPACING) + 30;
            int layerY = (layer / LAYER_SIZE) * (BOARD_SIZE * CELL_SIZE + LAYER_SPACING) + 30;

            for (int x = 0; x < BOARD_SIZE; x++)
            {
                for (int y = 0; y < BOARD_SIZE; y++)
                {
                    RECT cellRect = {
                        layerX + x * CELL_SIZE,
                        layerY + y * CELL_SIZE,
                        layerX + (x + 1) * CELL_SIZE,
                        layerY + (y + 1) * CELL_SIZE
                    };

                    if (PtInRect(&cellRect, pt))
                    {
                        if (message == WM_LBUTTONDOWN)
                        {
                            if (firstClick)
                            {
                                gameLogic->placeMines(20);
                                gameLogic->calcStuckMines();
                                firstClick = false;
                            }

                            if (gameLogic->openCell(layer, x, y))
                            {
                                MessageBoxW(hWnd, L"게임 오버!", L"3D 지뢰찾기", MB_OK);
                                gameLogic = std::make_unique<Func>();
                                firstClick = true;
                            }
                        }
                        else if (message == WM_RBUTTONDOWN)
                        {
                            gameLogic->setFlag(layer, x, y);
                        }

                        InvalidateRect(hWnd, NULL, TRUE);
                        return 0;
                    }
                }
            }
        }
        break;
    }
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // 메뉴 선택을 구문 분석합니다:
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
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
            // TODO: 여기에 hdc를 사용하는 그리기 코드를 추가합니다...
            DrawBoard(hdc);
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// 정보 대화 상자의 메시지 처리기입니다.
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


void DrawBoard(HDC hdc)
{
    for (int layer = 0; layer < 9; layer++)
    {
        int layerX = (layer % LAYER_SIZE) * (BOARD_SIZE * CELL_SIZE + LAYER_SPACING) + 30;
        int layerY = (layer / LAYER_SIZE) * (BOARD_SIZE * CELL_SIZE + LAYER_SPACING) + 30;

        for (int x = 0; x < BOARD_SIZE; x++)
        {
            for (int y = 0; y < BOARD_SIZE; y++)
            {
                mineCell& cell = gameLogic->board[layer][x][y];
                RECT rect = {
                    layerX + x * CELL_SIZE,
                    layerY + y * CELL_SIZE,
                    layerX + (x + 1) * CELL_SIZE,
                    layerY + (y + 1) * CELL_SIZE
                };

                // 마우스가 현재 셀 또는 인접한 셀 위에 있는지 확인
                bool isHighlighted = false;
                for (int dx = -1; dx <= 1; dx++)
                {
                    for (int dy = -1; dy <= 1; dy++)
                    {
                        RECT adjacentRect = {
                            layerX + (x + dx) * CELL_SIZE,
                            layerY + (y + dy) * CELL_SIZE,
                            layerX + (x + dx + 1) * CELL_SIZE,
                            layerY + (y + dy + 1) * CELL_SIZE
                        };
                        if (PtInRect(&adjacentRect, g_mousePos))
                        {
                            isHighlighted = true;
                            break;
                        }
                    }
                    if (isHighlighted) break;
                }

                // 배경 칠하기
                if (isHighlighted)
                {
                    HBRUSH hBrush = CreateSolidBrush(RGB(255, 255, 200));  // 연노랑
                    FillRect(hdc, &rect, hBrush);
                    DeleteObject(hBrush);
                }
                else if (firstClick || cell.status != 3) // 첫 클릭 전이거나 닫힌 셀
                {
                    HBRUSH hBrush = CreateSolidBrush(RGB(220, 220, 220));  // 밝은 회색
                    FillRect(hdc, &rect, hBrush);
                    DeleteObject(hBrush);
                }
                else // 열린 셀
                {
                    FillRect(hdc, &rect, (HBRUSH)GetStockObject(WHITE_BRUSH));
                }

                // 테두리 그리기
                HPEN hPen = CreatePen(PS_SOLID, 1, RGB(128, 128, 128));
                HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);
                HBRUSH hOldBrush = (HBRUSH)SelectObject(hdc, GetStockObject(NULL_BRUSH));
                Rectangle(hdc, rect.left, rect.top, rect.right, rect.bottom);
                SelectObject(hdc, hOldBrush);
                SelectObject(hdc, hOldPen);
                DeleteObject(hPen);

                // 셀 내용 그리기
                if (cell.status == 1) // 깃발
                {
                    DrawTextW(hdc, L"F", -1, &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
                }
                else if (cell.status == 2) // 물음표
                {
                    DrawTextW(hdc, L"?", -1, &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
                }
                else if (cell.status == 3) // 열린 셀
                {
                    if (cell.stuckMines > 0)
                    {
                        wchar_t text[2];
                        _itow_s(cell.stuckMines, text, 10);
                        DrawTextW(hdc, text, -1, &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
                    }
                    else if (cell.isMine)
                    {
                        DrawTextW(hdc, L"X", -1, &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
                    }
                }
            }
        }
    }
}

void HandleMouseMove(HWND hWnd, int x, int y)
{
    g_prevMousePos = g_mousePos;
    g_mousePos.x = x;
    g_mousePos.y = y;

    for (int layer = 0; layer < 9; layer++)
    {
        int layerX = (layer % LAYER_SIZE) * (BOARD_SIZE * CELL_SIZE + LAYER_SPACING) + 30;
        int layerY = (layer / LAYER_SIZE) * (BOARD_SIZE * CELL_SIZE + LAYER_SPACING) + 30;

        for (int cellX = 0; cellX < BOARD_SIZE; cellX++)
        {
            for (int cellY = 0; cellY < BOARD_SIZE; cellY++)
            {
                RECT cellRect = {
                    layerX + cellX * CELL_SIZE,
                    layerY + cellY * CELL_SIZE,
                    layerX + (cellX + 1) * CELL_SIZE,
                    layerY + (cellY + 1) * CELL_SIZE
                };

                RECT highlightRect = {
                    cellRect.left - CELL_SIZE,
                    cellRect.top - CELL_SIZE,
                    cellRect.right + CELL_SIZE,
                    cellRect.bottom + CELL_SIZE
                };

                if (PtInRect(&highlightRect, g_mousePos) || PtInRect(&highlightRect, g_prevMousePos))
                {
                    InvalidateRect(hWnd, &highlightRect, FALSE);
                }
            }
        }
    }
}