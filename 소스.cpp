#include <windows.h>
#include <TCHAR.H>
#include <stdio.h>
#include <stdlib.h>

#pragma comment(linker, "/entry:WinMainCRTStartup /subsystem:console")	//콘솔창 띄움

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
void DrawBitmap(HDC hdc, int x, int y, HBITMAP hBit);
void OnTimer(HWND, struct Character);		//메모리 디시를 이용해 hBit에 미리 그려 놓는 함수
HINSTANCE hInst;
HBITMAP hBit;
BOOL MakeBitmap;

struct Character
{
	int x, y;	//캐릭터의 좌표값
};

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
	LPSTR lpszCmdLine, int nCmdShow)
{
	HWND hWnd;
	MSG msg;
	WNDCLASS wndClass;
	TCHAR className[11] = L"Class Name";
	TCHAR titleName[10] = L"윈플 과제";
	hInst = hInstance;

	wndClass.style = CS_HREDRAW | CS_VREDRAW;
	wndClass.lpfnWndProc = WndProc;
	wndClass.cbClsExtra = 0;
	wndClass.cbWndExtra = 0;
	wndClass.hInstance = hInstance;
	wndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndClass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wndClass.lpszMenuName = NULL;
	wndClass.lpszClassName = className;
	RegisterClass(&wndClass);
	hWnd = CreateWindow(className, titleName, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX,
		50, 40, 1000, 600,
		NULL, NULL, hInstance, NULL);
	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return (int)msg.wParam;
}


void OnTimer(HWND hWnd, struct Character player)		//메모리 디시를 이용해 hBit에 미리 그려 놓는 함수
{
	HDC hDC, hMemDC;
	HBITMAP oldBit;
	RECT crt;
	static int xi = 5, yi;	//플레이어의 이동량
	hDC = GetDC(hWnd);
	GetClientRect(hWnd, &crt);	//화면의 정보를 구조체에다 담음

	const int R = crt.bottom / 20;

	if (MakeBitmap == TRUE)
	{
		hBit = CreateCompatibleBitmap(hDC, crt.right, crt.bottom);	//화면 크기의 비트맵을 생성
		MakeBitmap = FALSE;
	}
	hMemDC = CreateCompatibleDC(hDC);								//메모리디시에 만들어진 비트맵을 저장
	oldBit = (HBITMAP)SelectObject(hMemDC, hBit);					//oldBit에 저장

	xi += 10;	//10씩 증가
	player.x += xi;	//위치에 이동거리를 더해줌

	if (player.x - R >= crt.right)
		xi = -crt.right / 5 - R;

	FillRect(hMemDC, &crt, GetSysColorBrush(COLOR_WINDOW));

	//ㅡㅡㅡ 화면에 그림을 그림 ㅡㅡㅡ
	Ellipse(hMemDC, player.x - R, player.y - R, player.x + R, player.y + R);
	printf("%d", player.x);
	SelectObject(hMemDC, oldBit);
	DeleteDC(hMemDC);
	ReleaseDC(hWnd, hDC);
	InvalidateRect(hWnd, NULL, FALSE);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	HDC hDC;
	PAINTSTRUCT ps;
	RECT rt;
	static struct Character player;

	switch (uMsg)
	{
	case WM_SIZE:	//윈도우 크기가 변경될때마다 리소스들의 좌표값을 재서정
	case WM_CREATE:		//(50,50)에서 시작해서 25초마다 x축으로 4픽셀 y축으로 5픽셀 씩 움직이게 함
		GetClientRect(hWnd, &rt);	//화면의 정보를 구조체에다 담음
		player.x = rt.right/5;
		player.y = rt.bottom/5*3;
		MakeBitmap = TRUE;
		break;
	case WM_CHAR:
		switch (wParam)
		{
		case 's':
		case 'S':
			SetTimer(hWnd, 1, 25, NULL);	//타이머 번호, 주기, 타이머 함수
			break;
		}
		break;
	case WM_TIMER:
		OnTimer(hWnd, player);	//OnTimer함수 호출
		break;
	case WM_PAINT:
		hDC = BeginPaint(hWnd, &ps);
		if (hBit) DrawBitmap(hDC, 0, 0, hBit);		//DrawBitmap함수로 그려진 비트맵을 출력함
		EndPaint(hWnd, &ps);
		break;
	case WM_DESTROY:
		if (hBit) DeleteObject(hBit);
		PostQuitMessage(0);
		KillTimer(hWnd, 1);
		break;
	}
	return(DefWindowProc(hWnd, uMsg, wParam, lParam));
}

void DrawBitmap(HDC hdc, int x, int y, HBITMAP hBit)
{
	HDC MemDC;
	HBITMAP OldBitmap;
	int bx, by;
	BITMAP bit;

	MemDC = CreateCompatibleDC(hdc);
	OldBitmap = (HBITMAP)SelectObject(MemDC, hBit);

	GetObject(hBit, sizeof(BITMAP), &bit);
	bx = bit.bmWidth;
	by = bit.bmHeight;

	BitBlt(hdc, x, y, bx, by, MemDC, 0, 0, SRCCOPY);

	SelectObject(MemDC, OldBitmap);
	DeleteDC(MemDC);
}
