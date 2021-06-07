#include <windows.h>
#include <TCHAR.H>
#include <stdio.h>
#include <stdlib.h>
#include "resource.h"

#pragma comment(linker, "/entry:WinMainCRTStartup /subsystem:console")	//콘솔창 띄움
#pragma comment(lib, "msimg32.lib")

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
//void DrawBitmap(HDC hdc, int x, int y, HBITMAP hBit);
void OnTimer(HWND, struct Character);		//메모리 디시를 이용해 hBit에 미리 그려 놓는 함수
HINSTANCE hInst;
BOOL MakeBitmap;
HBITMAP hPlayer[3],	//플레이어 이미지
hFloor, hBackground;	//바닥, 배경
BITMAP bit, fbit, backgroundBit;


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
	TCHAR titleName[10] = L"윈플 기말과제";
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
	HDC hDC, hMemDC, hMemDC2;
	static HBITMAP hOldBit, hBit;
	RECT crt;
	static int xi, xj;	//플레이어의 이동량
	hDC = GetDC(hWnd);
	GetClientRect(hWnd, &crt);	//화면의 정보를 구조체에다 담음
	const int R = crt.bottom / 8;
	static int i, w;

	//if (MakeBitmap == TRUE)
	//{
		hBit = CreateCompatibleBitmap(hDC, crt.right, crt.bottom);	//화면 크기의 비트맵을 생성
	//	MakeBitmap = FALSE;
	//}

	hMemDC = CreateCompatibleDC(hDC);								//메모리디시에 만들어진 비트맵을 저장
	hOldBit = (HBITMAP)SelectObject(hMemDC, hBit);					//oldBit에 저장
	hMemDC2 = CreateCompatibleDC(hDC);

	xi +=5;	//1씩 증가
	xj += 10;
	//if (player.x - R >= crt.right)
	//	xi = -crt.right / 5 - R;
	 

	//ㅡㅡㅡ 화면에 배경 출력 ㅡㅡㅡ
	SelectObject(hMemDC2, hBackground);
	StretchBlt(hMemDC, -xi, 0, crt.right, crt.bottom, hMemDC2, 0, 0, backgroundBit.bmWidth, backgroundBit.bmHeight, SRCCOPY);
	StretchBlt(hMemDC, crt.right-xi, 0, crt.right, crt.bottom, hMemDC2, 0, 0, backgroundBit.bmWidth, backgroundBit.bmHeight, SRCCOPY);
	if (xi > crt.right)
		xi = 0;
	//ㅡㅡㅡ 화면에 바닥 출력 ㅡㅡㅡ
	SelectObject(hMemDC2, hFloor);
	TransparentBlt(hMemDC, -xj, player.y+R, crt.right, crt.bottom-player.y-R, hMemDC2, 0, 0, fbit.bmWidth/4*3, fbit.bmHeight, RGB(255, 0, 255));
	TransparentBlt(hMemDC, crt.right - xj, player.y + R, crt.right, crt.bottom - player.y - R, hMemDC2, 0, 0, fbit.bmWidth / 4 * 3, fbit.bmHeight, RGB(255, 0, 255));
	if (xj > crt.right)
		xj = 0;
	//ㅡㅡㅡ 화면에 플레이어 출력 ㅡㅡㅡ
	SelectObject(hMemDC2, hPlayer[i%3]);
	TransparentBlt(hMemDC, player.x - R, player.y - R, 2 * R, 2 * R, hMemDC2, 0, 0, bit.bmWidth, bit.bmHeight, RGB(255,0,255));
	i++;

	DeleteDC(hMemDC2);

	//ㅡㅡㅡ 완성된 그림 출력 ㅡㅡㅡ
	BitBlt(hDC, 0, 0, crt.right	, crt.bottom, hMemDC, 0, 0, SRCCOPY);

	SelectObject(hMemDC,hOldBit);
	DeleteObject(hBit);
	DeleteDC(hMemDC);
	ReleaseDC(hWnd, hDC);
	//InvalidateRect(hWnd, NULL, FALSE);
}

//캐릭터한테 중력생기게 하기.
//장애물, 아이템 지정한 패턴대로 나오게하는 방법 알아내기. 
//느려지는 원인 찾기

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

		hPlayer[0] = (HBITMAP)LoadBitmap(hInst, MAKEINTRESOURCE(IDB_BITMAP1));	//플레이어 이미지1
		hPlayer[1] = (HBITMAP)LoadBitmap(hInst, MAKEINTRESOURCE(IDB_BITMAP2));	//플레이어 이미지2
		hPlayer[2] = (HBITMAP)LoadBitmap(hInst, MAKEINTRESOURCE(IDB_BITMAP3));	//플레이어 이미지3
		GetObject(hPlayer[0], sizeof(BITMAP), &bit);
		hFloor = (HBITMAP)LoadBitmap(hInst, MAKEINTRESOURCE(IDB_BITMAP4));	//바닥 이미지
		GetObject(hFloor, sizeof(BITMAP), &fbit);
		hBackground = (HBITMAP)LoadBitmap(hInst, MAKEINTRESOURCE(IDB_BITMAP5));	//배경 이미지
		GetObject(hBackground, sizeof(BITMAP), &backgroundBit);
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
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		KillTimer(hWnd, 1);
		break;
	}
	return(DefWindowProc(hWnd, uMsg, wParam, lParam));
}
//
//void DrawBitmap(HDC hdc, int x, int y, HBITMAP hBit)
//{
//	HDC MemDC;
//	HBITMAP OldBitmap;
//	int bx, by;
//	BITMAP bit;
//
//	MemDC = CreateCompatibleDC(hdc);
//	OldBitmap = (HBITMAP)SelectObject(MemDC, hBit);
//
//	GetObject(hBit, sizeof(BITMAP), &bit);
//	bx = bit.bmWidth;
//	by = bit.bmHeight;
//
//	BitBlt(hdc, x, y, bx, by, MemDC, 0, 0, SRCCOPY);
//
//	SelectObject(MemDC, OldBitmap);
//	DeleteDC(MemDC);
//}
