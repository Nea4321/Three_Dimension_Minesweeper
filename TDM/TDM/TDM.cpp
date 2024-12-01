// TDM.cpp : 애플리케이션에 대한 진입점을 정의합니다.

#include "framework.h"
#include "TDM.h"
#include "Func.h"

#include <windows.h>
#include <windowsx.h>
#include <memory>
#include <string>
#include <array>

#define MAX_LOADSTRING 100
#define CELL_SIZE 30
#define LAYER_GAP 15

// 전역 변수:
HINSTANCE hInst;                                // 현재 인스턴스입니다.
WCHAR szTitle[MAX_LOADSTRING];                  // 제목 표시줄 텍스트입니다.
WCHAR szWindowClass[MAX_LOADSTRING];            // 기본 창 클래스 이름입니다.

std::unique_ptr<Func> gameFunc;
std::array<std::array<std::array<RECT, SIZE>, SIZE>, SIZE> cellRect;
int lastHighlightZ = -1, lastHighlightX = -1, lastHighlightY = -1;
bool firstClick = true;

// 함수 선언:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
void                InitializeCellRects();
void                DrawBoard(HDC hdc);
void                HighlightCells(HWND hWnd, int z, int x, int y);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR    lpCmdLine,
    _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // 전역 문자열을 초기화합니다.
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_TDM, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // 애플리케이션 초기화를 수행합니다:
    if (!InitInstance(hInstance, nCmdShow))
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

    return (int)msg.wParam;
}

ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_TDM));
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_TDM);
    wcex.lpszClassName = szWindowClass;
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    hInst = hInstance;

    int windowWidth = CELL_SIZE * SIZE * 3 + LAYER_GAP * 2 + 16;
    int windowHeight = CELL_SIZE * SIZE * 3 + LAYER_GAP * 2 + 39;

    HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, 0, windowWidth, windowHeight, nullptr, nullptr, hInstance, nullptr);

    if (!hWnd)
    {
        return FALSE;
    }

    gameFunc = std::make_unique<Func>();
    gameFunc->placeMines();
    gameFunc->calcStuckMines();

    InitializeCellRects();

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    return TRUE;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_COMMAND:
    {
        int wmId = LOWORD(wParam);
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
    case WM_MOUSEMOVE:
    {
        int xPos = GET_X_LPARAM(lParam);
        int yPos = GET_Y_LPARAM(lParam);
        RECT clickedRect = { xPos, yPos, xPos + 1, yPos + 1 };
        RECT intersection;

        for (int z = 0; z < SIZE; z++) {
            for (int x = 0; x < SIZE; x++) {
                for (int y = 0; y < SIZE; y++) {
                    if (IntersectRect(&intersection, &clickedRect, &cellRect[z][x][y])) {
                        HighlightCells(hWnd, z, x, y);
                        return 0;
                    }
                }
            }
        }
        // 마우스가 셀 밖으로 나갔을 때
        if (lastHighlightZ != -1) {
            HighlightCells(hWnd, -1, -1, -1);
        }
        break;
    }
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);
        DrawBoard(hdc);
        EndPaint(hWnd, &ps);
    }
    break;
    case WM_LBUTTONDOWN:
    {
        int xPos = GET_X_LPARAM(lParam);
        int yPos = GET_Y_LPARAM(lParam);
        RECT clickedRect = { xPos, yPos, xPos + 1, yPos + 1 };
        RECT intersection;

        for (int z = 0; z < SIZE; z++) {
            for (int x = 0; x < SIZE; x++) {
                for (int y = 0; y < SIZE; y++) {
                    if (IntersectRect(&intersection, &clickedRect, &cellRect[z][x][y])) {
                        if (firstClick) {
                            gameFunc->safeFirstClick(z, x, y);
                            firstClick = false;
                        }
                        gameFunc->openCell(z, x, y);
                        InvalidateRect(hWnd, NULL, TRUE);
                        return 0;
                    }
                }
            }
        }
    }
    break;
    case WM_RBUTTONDOWN:
    {
        if (firstClick) break;
        int xPos = GET_X_LPARAM(lParam);
        int yPos = GET_Y_LPARAM(lParam);
        RECT clickedRect = { xPos, yPos, xPos + 1, yPos + 1 };
        RECT intersection;

        for (int z = 0; z < SIZE; z++) {
            for (int x = 0; x < SIZE; x++) {
                for (int y = 0; y < SIZE; y++) {
                    if (IntersectRect(&intersection, &clickedRect, &cellRect[z][x][y])) {
                        gameFunc->setFlag(z, x, y);
                        InvalidateRect(hWnd, NULL, TRUE);
                        return 0;
                    }
                }
            }
        }
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

void InitializeCellRects()
{
    for (int z = 0; z < SIZE; z++) {
        for (int x = 0; x < SIZE; x++) {
            for (int y = 0; y < SIZE; y++) {
                int left = x * CELL_SIZE + (z % 3) * (SIZE * CELL_SIZE + LAYER_GAP);
                int top = y * CELL_SIZE + (z / 3) * (SIZE * CELL_SIZE + LAYER_GAP);
                cellRect[z][x][y] = { left, top, left + CELL_SIZE, top + CELL_SIZE };
            }
        }
    }
}

void DrawBoard(HDC hdc)
{
    HBRUSH nullBrush = (HBRUSH)GetStockObject(NULL_BRUSH);
    HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, nullBrush);
    HBRUSH highlightBrush = CreateSolidBrush(RGB(255, 255, 224)); // 연노랑색

    for (int z = 0; z < SIZE; z++) {
        for (int x = 0; x < SIZE; x++) {
            for (int y = 0; y < SIZE; y++) {
                RECT rect = cellRect[z][x][y];
                mineCell cell = gameFunc->board[z][y][x];

                // 하이라이트 영역 체크
                bool isHighlight = (abs(z - lastHighlightZ) <= 1 && abs(x - lastHighlightX) <= 1 && abs(y - lastHighlightY) <= 1);

                if (isHighlight && lastHighlightZ != -1 && cell.status != 3) {
                    FillRect(hdc, &rect, highlightBrush);
                }
                else if (cell.status == 3) { // 열린 셀
                    FillRect(hdc, &rect, (HBRUSH)GetStockObject(WHITE_BRUSH));
                }
                else { // 닫힌 셀, 깃발, 물음표
                    FillRect(hdc, &rect, (HBRUSH)GetStockObject(GRAY_BRUSH));
                }

                // 셀 내용 그리기
                if (cell.status == 3 && cell.stuckMines > 0) {
                    WCHAR buffer[2];
                    wsprintf(buffer, L"%d", cell.stuckMines);
                    DrawText(hdc, buffer, -1, &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
                }
                else if (cell.status == 3 && cell.isMine) {
                    DrawText(hdc, L"*", -1, &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
                }
                else if (cell.status == 1) {
                    DrawText(hdc, L"F", -1, &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
                }
                else if (cell.status == 2) {
                    DrawText(hdc, L"?", -1, &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
                }

                // 셀 테두리 그리기
                Rectangle(hdc, rect.left, rect.top, rect.right, rect.bottom);
            }
        }
    }

    SelectObject(hdc, oldBrush);
    DeleteObject(nullBrush);
    DeleteObject(highlightBrush);
}

void HighlightCells(HWND hWnd, int z, int x, int y)
{
    RECT updateRect = { 0 };

    // 이전 하이라이트 영역 갱신
    // 이전 하이라이트 영역 갱신
    if (lastHighlightZ != -1) {
        for (int dz = -1; dz <= 1; dz++) {
            for (int dx = -1; dx <= 1; dx++) {
                for (int dy = -1; dy <= 1; dy++) {
                    int nz = lastHighlightZ + dz, nx = lastHighlightX + dx, ny = lastHighlightY + dy;
                    if (nz >= 0 && nz < SIZE && nx >= 0 && nx < SIZE && ny >= 0 && ny < SIZE) {
                        UnionRect(&updateRect, &updateRect, &cellRect[nz][nx][ny]);
                    }
                }
            }
        }
    }

    // 새로운 하이라이트 영역 갱신
    if (z != -1) {
        for (int dz = -1; dz <= 1; dz++) {
            for (int dx = -1; dx <= 1; dx++) {
                for (int dy = -1; dy <= 1; dy++) {
                    int nz = z + dz, nx = x + dx, ny = y + dy;
                    if (nz >= 0 && nz < SIZE && nx >= 0 && nx < SIZE && ny >= 0 && ny < SIZE) {
                        UnionRect(&updateRect, &updateRect, &cellRect[nz][nx][ny]);
                    }
                }
            }
        }
    }

    // 갱신 영역이 있으면 화면 갱신
    if (updateRect.left < updateRect.right && updateRect.top < updateRect.bottom) {
        InvalidateRect(hWnd, &updateRect, FALSE);
    }

    // 현재 하이라이트 위치 저장
    lastHighlightZ = z;
    lastHighlightX = x;
    lastHighlightY = y;
}
