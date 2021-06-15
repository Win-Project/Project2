#include <windows.h>
#include <TCHAR.H>
#include <stdio.h>
#include <stdlib.h>
#include "resource.h"

#pragma comment(linker, "/entry:WinMainCRTStartup /subsystem:console")	//콘솔창 띄움
#pragma comment(lib, "msimg32.lib")
#pragma comment(lib,"winmm.lib")

#define itemNum 8	//한 화면에 출력되는 아이템 개수

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
void OnTimer(HWND, struct Character, BOOL*, struct Item**);		//메모리 디시를 이용해 hBit에 미리 그려 놓는 함수
BOOL CheckCollision(int n1, int n2, int m1, int m2);
void Gravity(struct Character* player, int t);
void GameOver(HWND hWnd, int* time);
void ItemCollision(struct Item* item, struct Character player, int R);	//아이템과 플레이어의 충돌을 감지하는 함수

HINSTANCE hInst;
BOOL MakeBitmap, GAMEOVER = FALSE;
HBITMAP hPlayer[3],	//플레이어 이미지
hFloor, hBackground,	//바닥, 배경
hItem;					//아이템
BITMAP bit, fbit, backgroundBit, itemBit;

struct Character
{
	int x, y;	//캐릭터의 좌표값
};

struct Item
{
	int x, y;
	BOOL visible = TRUE;
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


void OnTimer(HWND hWnd, struct Character player, BOOL* Jump, struct Item** itemPos)		//메모리 디시를 이용해 hBit에 미리 그려 놓는 함수
{
	HDC hDC, hMemDC, hMemDC2;
	static HBITMAP hOldBit, hBit;
	RECT crt;
	static int xi, Floor1X, Floor2X, FloorY;	//플레이어의 이동량
	hDC = GetDC(hWnd);
	GetClientRect(hWnd, &crt);	//화면의 정보를 구조체에다 담음
	const int R = crt.bottom / 8, Hole = crt.bottom/3;	//구멍의 너비
	static int i, w, time, t, yi, 
		speed = 15;//속도	20
	static BOOL GRAVITY = FALSE;

	hBit = CreateCompatibleBitmap(hDC, crt.right, crt.bottom);	//화면 크기의 비트맵을 생성

	hMemDC = CreateCompatibleDC(hDC);								//메모리디시에 만들어진 비트맵을 저장
	hOldBit = (HBITMAP)SelectObject(hMemDC, hBit);					//oldBit에 저장
	hMemDC2 = CreateCompatibleDC(hDC);

	xi +=5;	//1씩 증가
	Floor1X -= speed;	//바닥속도
	Floor2X -= speed;

	//ㅡㅡㅡ 화면에 배경 출력 ㅡㅡㅡ
	SelectObject(hMemDC2, hBackground);
	StretchBlt(hMemDC, -xi, 0, crt.right, crt.bottom, hMemDC2, 0, 0, backgroundBit.bmWidth, backgroundBit.bmHeight, SRCCOPY);
	StretchBlt(hMemDC, crt.right-xi, 0, crt.right, crt.bottom, hMemDC2, 0, 0, backgroundBit.bmWidth, backgroundBit.bmHeight, SRCCOPY);
	if (xi > crt.right)
		xi = 0;

	//ㅡㅡㅡ 화면에 바닥 출력 ㅡㅡㅡ
	FloorY = player.y + R;

	SelectObject(hMemDC2, hFloor);
	if (Floor1X + crt.right < crt.right)
		Floor2X = Floor1X + crt.right + Hole;
	if(Floor2X + crt.right<crt.right)
		Floor1X = Floor2X + crt.right + Hole;

	TransparentBlt(hMemDC, Floor1X, FloorY, crt.right, crt.bottom- FloorY, hMemDC2, 0, 0, fbit.bmWidth/4*3, fbit.bmHeight, RGB(255, 0, 255));
	TransparentBlt(hMemDC, Floor2X, FloorY, crt.right, crt.bottom - FloorY, hMemDC2, 0, 0, fbit.bmWidth / 4 * 3, fbit.bmHeight, RGB(255, 0, 255));


	//ㅡㅡㅡ 플레이어 위치 지정 ㅡㅡㅡ
	if (CheckCollision(player.x, player.x, Floor1X, Floor1X + crt.right) ||	//바닥과의 충돌판정
		CheckCollision(player.x, player.x, Floor2X, Floor2X + crt.right))
		GRAVITY = FALSE;	//바닥두개중 하나라도 충돌했다면 떨어지지 않음
	else
		GRAVITY = TRUE;

	//점프
	if (*Jump==TRUE)
	{
		GRAVITY = FALSE;
		t++;
		yi = -50 * t + 5 * t * t / 2;	//점프속도 50
		player.y += yi;
		if (yi == 0)
		{
			*Jump = FALSE;
			GRAVITY = TRUE;
			t = 0;
		}
	}

	if (GRAVITY == TRUE)	//중력작용
		Gravity(&player, time++);
	else time = 0;
	
	printf("%d %d %d %d %d\n", player.y + R, FloorY, player.x, Floor1X, Floor2X);
	if (player.y + R > FloorY)
	{
		if ((player.x+R/2> Floor1X&&Floor1X>0)|| (player.x + R / 2 > Floor2X && Floor2X > 0))
			GameOver(hWnd, &time); //플레이어의 위치가 바닥보다 낮을때 바닥과 부딪히면 게임오버
		//else if(player.x + R < Floor2X && Floor1X < 0)
		//	GameOver(hWnd, &time); //플레이어의 위치가 바닥보다 낮을때 바닥과 부딪히면 게임오버
	}

	//ㅡㅡㅡ 화면에 아이템 출력 ㅡㅡㅡ

	SelectObject(hMemDC2, hItem);

	for (int i = 0; i < itemNum + 1; ++i)
	{
		(*itemPos)[i].x -= speed;		//아이템 속도 -바닥속도와 동일
		if ((*itemPos)[i].x < 0 - R)
		{
			(*itemPos)[i].x = crt.right + R;
			(*itemPos)[i].visible = TRUE;
		}

		ItemCollision(&(*itemPos)[i], player, R);	//충돌체크

		if ((*itemPos)[i].visible)
			TransparentBlt(hMemDC, (*itemPos)[i].x, (*itemPos)[i].y, R, R, hMemDC2, 0, 0, itemBit.bmWidth, itemBit.bmHeight, RGB(255, 0, 255));	//아이템 출력
	}

	//ㅡㅡㅡ 플레이어 출력 ㅡㅡㅡ
	SelectObject(hMemDC2, hPlayer[i%3]);
	TransparentBlt(hMemDC, player.x - R, player.y - R, 2 * R, 2 * R, hMemDC2, 0, 0, bit.bmWidth, bit.bmHeight, RGB(255,0,255));
	i++;

	DeleteDC(hMemDC2);

	//ㅡㅡㅡ게임 오버 ㅡㅡㅡ
	//if (player.y + R > 600)
	//	GameOver(hWnd, &time);

	//ㅡㅡㅡ 완성된 그림 출력 ㅡㅡㅡ
	BitBlt(hDC, 0, 0, crt.right	, crt.bottom, hMemDC, 0, 0, SRCCOPY);

	SelectObject(hMemDC,hOldBit);
	DeleteObject(hBit);
	DeleteDC(hMemDC);
	ReleaseDC(hWnd, hDC);
	//InvalidateRect(hWnd, NULL, FALSE);
}

//장애물, 아이템 지정한 패턴대로 나오게하는 방법 알아내기.

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	HDC hDC;
	PAINTSTRUCT ps;
	RECT rt;
	static struct Character player;
	static BOOL Jump = FALSE;
	static Item* itemPos = (Item*)malloc(sizeof(Item) * (itemNum + 1));

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
		hFloor = (HBITMAP)LoadBitmap(hInst, MAKEINTRESOURCE(IDB_BITMAP4));		//바닥 이미지
		GetObject(hFloor, sizeof(BITMAP), &fbit);
		hBackground = (HBITMAP)LoadBitmap(hInst, MAKEINTRESOURCE(IDB_BITMAP5));	//배경 이미지
		GetObject(hBackground, sizeof(BITMAP), &backgroundBit);
		hItem = (HBITMAP)LoadBitmap(hInst, MAKEINTRESOURCE(IDB_BITMAP6));	//아이템 이미지
		GetObject(hItem, sizeof(BITMAP), &itemBit);

		for (int i = 0; i <= itemNum; ++i)
		{
			itemPos[i].x = rt.right / itemNum * i;	//아이템의 위치 정하기
			itemPos[i].y = player.y;
		}
		itemPos[0].y = 100;
		break;
	case WM_CHAR:
		switch (wParam)
		{
		case 's':
		case 'S':
			PlaySound(MAKEINTRESOURCE(IDR_WAVE1), hInst, SND_RESOURCE | SND_ASYNC);
			SetTimer(hWnd, 1, 25, NULL);	//타이머 번호, 주기, 타이머 함수
			break;
		case 'q':
			KillTimer(hWnd, 1);
		}
		break;
	case WM_TIMER:
		OnTimer(hWnd, player, &Jump, &itemPos);	//OnTimer함수 호출
		break;
	case WM_LBUTTONDOWN:
		Jump = TRUE;
		break;
	case WM_DESTROY:
		free(itemPos);
		PostQuitMessage(0);
		KillTimer(hWnd, 1);
		break;
	}
	return(DefWindowProc(hWnd, uMsg, wParam, lParam));
}

BOOL GameOver(int y)
{
	BOOL Gameover = FALSE;

		Gameover = TRUE;
	return Gameover;
}
void GameOver(HWND hWnd, int* time)
{
	KillTimer(hWnd, 1);
	PlaySound(MAKEINTRESOURCE(IDR_WAVE2), hInst, SND_RESOURCE | SND_ASYNC);
	*time = 0;
}

BOOL CheckCollision(int n1, int n2, int m1, int m2)
{
	BOOL Collision = TRUE;
	if (n1 > m2 || n2 < m1)
		Collision = FALSE;

	return Collision;
}

void Gravity(struct Character* player, int t)
{
	player->y += 5 * t * t / 2;	//이동거리 계산
}

void ItemCollision(struct Item* item, struct Character player, int R)
{
	if ((R * R * 9 / 4) >= ((item->x - player.x) * (item->x - player.x) + (item->y - player.y) * (item->y - player.y)))
		item->visible = FALSE;
}