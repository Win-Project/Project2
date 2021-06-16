#include <windows.h>
#include <TCHAR.H>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "resource.h"

//#pragma comment(linker, "/entry:WinMainCRTStartup /subsystem:console")	//콘솔창 띄움
#pragma comment(lib, "msimg32.lib")
#pragma comment(lib,"winmm.lib")

#define itemNum 8	//한 화면에 출력되는 아이템 개수

clock_t start, stop; 
int timer, total; 

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
void OnTimer(HWND, struct Character, BOOL*, struct Item**);		//메모리 디시를 이용해 hBit에 미리 그려 놓는 함수
BOOL CheckCollision(int n1, int n2, int m1, int m2);
void Gravity(struct Character* player, int t);
void GameOver(HWND hWnd, int* time);
void ItemCollision(struct Item* item, struct Character player, int R);	//아이템과 플레이어의 충돌을 감지하는 함수
BOOL ObstacleCollision(struct Obstacle* ob, struct Character player, int pR, int oR);

HINSTANCE hInst;
BOOL GAMEOVER = FALSE, PrintScore = FALSE, DoubleJump = FALSE, ReStart = FALSE;
HBITMAP hPlayer[4],	//플레이어 이미지
hFloor, hBackground,	//바닥, 배경
hItem,					//아이템
hScore;
HBITMAP hObstacle[3];	//장애물 이미지
BITMAP obstacleBit[3];
BITMAP bit, fbit, backgroundBit, itemBit, scoreBit;
int item_count;	//먹은 아이템 개수

struct Character
{
	int x, y;	//캐릭터의 좌표값
};

struct Item
{
	int x, y;
	BOOL visible = TRUE;
};

struct Obstacle
{
	int x, y;
	int obstaID = 0;
	int pos;
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

	wndClass.style = CS_HREDRAW | CS_VREDRAW |CS_DBLCLKS;
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
	hWnd = CreateWindow(className, titleName, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU ,
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

void OnTimer(HWND hWnd, struct Character player, BOOL* Jump, struct Item** itemPos, struct Obstacle** ob)		//메모리 디시를 이용해 hBit에 미리 그려 놓는 함수
{
	HDC hDC, hMemDC, hMemDC2;
	static HBITMAP hOldBit, hBit;
	RECT crt;
	static int xi, Floor1X, Floor2X, FloorY;	//플레이어의 이동량
	hDC = GetDC(hWnd);
	GetClientRect(hWnd, &crt);	//화면의 정보를 구조체에다 담음
	const int R = crt.bottom / 8, Hole = crt.bottom/4;	//구멍의 너비
	static int i, w, time, t, yi, 
		speed = 15;//속도	20
	static BOOL GRAVITY = FALSE;
	TCHAR buf[100];

	hBit = CreateCompatibleBitmap(hDC, crt.right, crt.bottom);	//화면 크기의 비트맵을 생성

	hMemDC = CreateCompatibleDC(hDC);								//메모리디시에 만들어진 비트맵을 저장
	hOldBit = (HBITMAP)SelectObject(hMemDC, hBit);					//oldBit에 저장
	hMemDC2 = CreateCompatibleDC(hDC);

	//ㅡㅡㅡ 재시작 ㅡㅡㅡ
	if (ReStart == TRUE)
	{
		xi = 0, Floor1X = 0, Floor2X = 0, FloorY = 0;
		i = 0, w = 0, time = 0, t = 0, yi = 0;
		speed = 15;
		ReStart = FALSE;
	}
	xi += 5;	//1씩 증가
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
	if (DoubleJump == TRUE)
	{
		*Jump = FALSE;
		GRAVITY = FALSE;
		t++;
		yi = -45 * t + 5 * t * t / 2;	//점프속도 50
		player.y += yi;
		if (yi == 0)
		{
			DoubleJump = FALSE;
			GRAVITY = TRUE;
			t = 0;
		}
	}

	if (*Jump==TRUE)
	{
		GRAVITY = FALSE;
		t++;
		yi = -30 * t + 5 * t * t / 2;	//점프속도 50
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
	
	if (player.y + R > FloorY)
	{
		if ((player.x + R / 2 > Floor1X && Floor1X > 0) || (player.x + R / 2 > Floor2X && Floor2X > 0))
			GameOver(hWnd, &time); //플레이어의 위치가 바닥보다 낮을때 바닥과 부딪히면 게임오버

	}
	//ㅡㅡㅡ 화면에 장애물 출력 ㅡㅡㅡ
	for (int i = 0; i < 2; ++i)
	{
		SelectObject(hMemDC2, hObstacle[0]);

		(*ob)[i].x -= speed;		//바닥속도와 동일

		if ((*ob)[i].x < 0 - R)	//위치 재 설정
			(*ob)[i].x = (*itemPos)[(*ob)[i].pos].x;

		TransparentBlt(hMemDC, (*ob)[i].x, (*ob)[i].y, R, R, hMemDC2, 0, 0, obstacleBit[0].bmWidth,
			obstacleBit[0].bmHeight, RGB(255, 0, 255));	//아이템 출력

		if (ObstacleCollision(&(*ob)[i], player, R, R / 2))
			GameOver(hWnd, &time);
	}


	//ㅡㅡㅡ 화면에 아이템 출력 ㅡㅡㅡ

	SelectObject(hMemDC2, hItem);

	for (int i = 0; i <= itemNum ; ++i)
	{
		(*itemPos)[i].x -= speed;		//아이템 속도 -바닥속도와 동일
		if ((*itemPos)[i].x < 0 - R)
		{
			(*itemPos)[i].x = crt.right + R;
			(*itemPos)[i].visible = TRUE;
		}


		if ((*itemPos)[i].visible)
		{
			ItemCollision(&(*itemPos)[i], player, R);	//충돌체크
			TransparentBlt(hMemDC, (*itemPos)[i].x, (*itemPos)[i].y, R, R, hMemDC2, 0, 0, itemBit.bmWidth, 
				itemBit.bmHeight, RGB(255, 0, 255));	//아이템 출력
		}
	}

	//ㅡㅡㅡ 플레이어 출력 ㅡㅡㅡ
	if(*Jump == TRUE||DoubleJump==TRUE)
		SelectObject(hMemDC2, hPlayer[3]);
	else
		SelectObject(hMemDC2, hPlayer[i%3]);
	TransparentBlt(hMemDC, player.x - R, player.y - R, 2 * R, 2 * R, hMemDC2, 0, 0, bit.bmWidth, bit.bmHeight, RGB(255,0,255));
	i++;

	//속도 증가
	if (i > 300)
		speed = 19;
	if (i > 600)
		speed = 22;
	//ㅡㅡㅡ 점수판 출력 ㅡㅡㅡ
	if (PrintScore == TRUE)
	{
		stop = clock();

		timer = int(stop - start) / CLOCKS_PER_SEC;

		SelectObject(hMemDC2, hScore);
		StretchBlt(hMemDC, crt.right / 2 - 200, crt.bottom / 2 - 200, 400, 400, 
			hMemDC2, 0, 0, scoreBit.bmWidth, scoreBit.bmHeight, SRCCOPY);

		wsprintf(buf, _T("%d"), timer); //달린 시간
		TextOut(hMemDC, 600, 210, buf, lstrlen(buf));
		TextOut(hMemDC, 601 + 7*lstrlen(buf), 210, TEXT("초 x 5점"), 6);

		wsprintf(buf, _T("%d"), item_count); //모은 솜 개수
		TextOut(hMemDC, 600, 283, buf, lstrlen(buf));
		printf("%d", lstrlen(buf));
		TextOut(hMemDC, 601 + 7*lstrlen(buf), 283, TEXT("개 x 5점"), 6);

		total = (timer * 5) + (item_count * 5);

		wsprintf(buf, _T("%d"), total); //모은 솜 개수
		TextOut(hMemDC, 600, 415, buf, lstrlen(buf));
	}

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
	static RECT rt;
	static struct Character player;
	static BOOL Jump = FALSE;
	static Item* itemPos = (Item*)malloc(sizeof(Item) * (itemNum + 1));
	static Obstacle* ObInfo = (Obstacle*)malloc(sizeof(Obstacle) * 2);

	ObInfo[0].pos = 0;
	ObInfo[1].pos = 5;

	switch (uMsg)
	{
	case WM_SIZE:	//윈도우 크기가 변경될때마다 리소스들의 좌표값을 재서정
	case WM_CREATE:		//(50,50)에서 시작해서 25초마다 x축으로 4픽셀 y축으로 5픽셀 씩 움직이게 함
		GetClientRect(hWnd, &rt);	//화면의 정보를 구조체에다 담음
		player.x = rt.right/5;
		player.y = rt.bottom/5*3;

		hPlayer[0] = (HBITMAP)LoadBitmap(hInst, MAKEINTRESOURCE(IDB_BITMAP1));	//플레이어 이미지1
		hPlayer[1] = (HBITMAP)LoadBitmap(hInst, MAKEINTRESOURCE(IDB_BITMAP2));	//플레이어 이미지2
		hPlayer[2] = (HBITMAP)LoadBitmap(hInst, MAKEINTRESOURCE(IDB_BITMAP3));	//플레이어 이미지3
		hPlayer[3] = (HBITMAP)LoadBitmap(hInst, MAKEINTRESOURCE(IDB_BITMAP8));	//플레이어 이미지4
		GetObject(hPlayer[0], sizeof(BITMAP), &bit);
		hFloor = (HBITMAP)LoadBitmap(hInst, MAKEINTRESOURCE(IDB_BITMAP4));		//바닥 이미지
		GetObject(hFloor, sizeof(BITMAP), &fbit);
		hBackground = (HBITMAP)LoadBitmap(hInst, MAKEINTRESOURCE(IDB_BITMAP5));	//배경 이미지
		GetObject(hBackground, sizeof(BITMAP), &backgroundBit);
		hItem = (HBITMAP)LoadBitmap(hInst, MAKEINTRESOURCE(IDB_BITMAP6));	//아이템 이미지
		GetObject(hItem, sizeof(BITMAP), &itemBit);
		hObstacle[0] = (HBITMAP)LoadBitmap(hInst, MAKEINTRESOURCE(IDB_BITMAP7));	//장애물 이미지1
		GetObject(hObstacle[0], sizeof(BITMAP), &obstacleBit[0]);
		hScore = (HBITMAP)LoadBitmap(hInst, MAKEINTRESOURCE(IDB_BITMAP9));	//장애물 이미지1
		GetObject(hScore, sizeof(BITMAP), &scoreBit);
		break;
	case WM_CHAR:
		switch (wParam)
		{
		case 's':
		case 'S':
			PlaySound(MAKEINTRESOURCE(IDR_WAVE1), hInst, SND_RESOURCE | SND_ASYNC);
			start = clock();
			SetTimer(hWnd, 1, 25, NULL);	//타이머 번호, 주기, 타이머 함수
			PrintScore = FALSE;
			item_count = 0;
			Jump = FALSE;
			DoubleJump = FALSE;
			ReStart = TRUE;
			//ㅡㅡㅡ 아이템의 좌표값 설정 ㅡㅡㅡ
			for (int i = 0; i <= itemNum; ++i)
			{
				itemPos[i].x = rt.right / itemNum * i;	//아이템의 위치 정하기
				itemPos[i].y = player.y;
			}
			itemPos[ObInfo[0].pos].y = 200;
			itemPos[ObInfo[1].pos].y = 200;
			//ㅡㅡㅡ 장애물의 좌표값 설정 ㅡㅡ
			ObInfo[0].y = rt.bottom / 5 * 3;
			ObInfo[1].y = rt.bottom / 5 * 3;
			ObInfo[0].x = itemPos[ObInfo[0].pos].x;
			ObInfo[1].x = itemPos[ObInfo[1].pos].x;
			break;
		case 'q':
			KillTimer(hWnd, 1);
		}
		break;
	case WM_TIMER:
		OnTimer(hWnd, player, &Jump, &itemPos, &ObInfo);	//OnTimer함수 호출
		break;
	case WM_LBUTTONDOWN:
		Jump = TRUE;
		break;
	case WM_LBUTTONDBLCLK:
		DoubleJump = TRUE;
		break;
	case WM_DESTROY:
		free(itemPos);
		PostQuitMessage(0);
		KillTimer(hWnd, 1);
		break;
	}
	return(DefWindowProc(hWnd, uMsg, wParam, lParam));
}


void GameOver(HWND hWnd, int* time)
{
	KillTimer(hWnd, 1);
	PlaySound(MAKEINTRESOURCE(IDR_WAVE2), hInst, SND_RESOURCE | SND_ASYNC);
	*time = 0;
	PrintScore = TRUE;
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
	if ((R * R) >= ((item->x - player.x) * (item->x - player.x) + (item->y - player.y) * (item->y - player.y)))
	{
		item_count++;
		item->visible = FALSE;
	}
}

BOOL ObstacleCollision(struct Obstacle* ob, struct Character player, int pR, int oR)
{
	BOOL Collision = FALSE;
	if ((pR * oR) >= ((ob->x - player.x) * (ob->x - player.x) + (ob->y - player.y) * (ob->y - player.y)))
		Collision = TRUE;
	return Collision;
}
