#include <windows.h>
#include <TCHAR.H>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "resource.h"

//#pragma comment(linker, "/entry:WinMainCRTStartup /subsystem:console")	//�ܼ�â ���
#pragma comment(lib, "msimg32.lib")
#pragma comment(lib,"winmm.lib")

#define itemNum 8	//�� ȭ�鿡 ��µǴ� ������ ����

clock_t start, stop; 
int timer, total; 

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
void OnTimer(HWND, struct Character, BOOL*, struct Item**);		//�޸� ��ø� �̿��� hBit�� �̸� �׷� ���� �Լ�
BOOL CheckCollision(int n1, int n2, int m1, int m2);
void Gravity(struct Character* player, int t);
void GameOver(HWND hWnd, int* time);
void ItemCollision(struct Item* item, struct Character player, int R);	//�����۰� �÷��̾��� �浹�� �����ϴ� �Լ�
BOOL ObstacleCollision(struct Obstacle* ob, struct Character player, int pR, int oR);

HINSTANCE hInst;
BOOL GAMEOVER = FALSE, PrintScore = FALSE, DoubleJump = FALSE, ReStart = FALSE;
HBITMAP hPlayer[4],	//�÷��̾� �̹���
hFloor, hBackground,	//�ٴ�, ���
hItem,					//������
hScore;
HBITMAP hObstacle[3];	//��ֹ� �̹���
BITMAP obstacleBit[3];
BITMAP bit, fbit, backgroundBit, itemBit, scoreBit;
int item_count;	//���� ������ ����

struct Character
{
	int x, y;	//ĳ������ ��ǥ��
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
	TCHAR titleName[10] = L"���� �⸻����";
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

void OnTimer(HWND hWnd, struct Character player, BOOL* Jump, struct Item** itemPos, struct Obstacle** ob)		//�޸� ��ø� �̿��� hBit�� �̸� �׷� ���� �Լ�
{
	HDC hDC, hMemDC, hMemDC2;
	static HBITMAP hOldBit, hBit;
	RECT crt;
	static int xi, Floor1X, Floor2X, FloorY;	//�÷��̾��� �̵���
	hDC = GetDC(hWnd);
	GetClientRect(hWnd, &crt);	//ȭ���� ������ ����ü���� ����
	const int R = crt.bottom / 8, Hole = crt.bottom/4;	//������ �ʺ�
	static int i, w, time, t, yi, 
		speed = 15;//�ӵ�	20
	static BOOL GRAVITY = FALSE;
	TCHAR buf[100];

	hBit = CreateCompatibleBitmap(hDC, crt.right, crt.bottom);	//ȭ�� ũ���� ��Ʈ���� ����

	hMemDC = CreateCompatibleDC(hDC);								//�޸𸮵�ÿ� ������� ��Ʈ���� ����
	hOldBit = (HBITMAP)SelectObject(hMemDC, hBit);					//oldBit�� ����
	hMemDC2 = CreateCompatibleDC(hDC);

	//�ѤѤ� ����� �ѤѤ�
	if (ReStart == TRUE)
	{
		xi = 0, Floor1X = 0, Floor2X = 0, FloorY = 0;
		i = 0, w = 0, time = 0, t = 0, yi = 0;
		speed = 15;
		ReStart = FALSE;
	}
	xi += 5;	//1�� ����
	Floor1X -= speed;	//�ٴڼӵ�
	Floor2X -= speed;

	//�ѤѤ� ȭ�鿡 ��� ��� �ѤѤ�
	SelectObject(hMemDC2, hBackground);
	StretchBlt(hMemDC, -xi, 0, crt.right, crt.bottom, hMemDC2, 0, 0, backgroundBit.bmWidth, backgroundBit.bmHeight, SRCCOPY);
	StretchBlt(hMemDC, crt.right-xi, 0, crt.right, crt.bottom, hMemDC2, 0, 0, backgroundBit.bmWidth, backgroundBit.bmHeight, SRCCOPY);
	if (xi > crt.right)
		xi = 0;

	//�ѤѤ� ȭ�鿡 �ٴ� ��� �ѤѤ�
	FloorY = player.y + R;

	SelectObject(hMemDC2, hFloor);
	if (Floor1X + crt.right < crt.right)
		Floor2X = Floor1X + crt.right + Hole;
	if(Floor2X + crt.right<crt.right)
		Floor1X = Floor2X + crt.right + Hole;

	TransparentBlt(hMemDC, Floor1X, FloorY, crt.right, crt.bottom- FloorY, hMemDC2, 0, 0, fbit.bmWidth/4*3, fbit.bmHeight, RGB(255, 0, 255));
	TransparentBlt(hMemDC, Floor2X, FloorY, crt.right, crt.bottom - FloorY, hMemDC2, 0, 0, fbit.bmWidth / 4 * 3, fbit.bmHeight, RGB(255, 0, 255));


	//�ѤѤ� �÷��̾� ��ġ ���� �ѤѤ�
	if (CheckCollision(player.x, player.x, Floor1X, Floor1X + crt.right) ||	//�ٴڰ��� �浹����
		CheckCollision(player.x, player.x, Floor2X, Floor2X + crt.right))
		GRAVITY = FALSE;	//�ٴڵΰ��� �ϳ��� �浹�ߴٸ� �������� ����
	else
		GRAVITY = TRUE;

	//����
	if (DoubleJump == TRUE)
	{
		*Jump = FALSE;
		GRAVITY = FALSE;
		t++;
		yi = -45 * t + 5 * t * t / 2;	//�����ӵ� 50
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
		yi = -30 * t + 5 * t * t / 2;	//�����ӵ� 50
		player.y += yi;
		if (yi == 0)
		{
			*Jump = FALSE;
			GRAVITY = TRUE;
			t = 0;
		}
	}
	if (GRAVITY == TRUE)	//�߷��ۿ�
		Gravity(&player, time++);
	else time = 0;
	
	if (player.y + R > FloorY)
	{
		if ((player.x + R / 2 > Floor1X && Floor1X > 0) || (player.x + R / 2 > Floor2X && Floor2X > 0))
			GameOver(hWnd, &time); //�÷��̾��� ��ġ�� �ٴں��� ������ �ٴڰ� �ε����� ���ӿ���

	}
	//�ѤѤ� ȭ�鿡 ��ֹ� ��� �ѤѤ�
	for (int i = 0; i < 2; ++i)
	{
		SelectObject(hMemDC2, hObstacle[0]);

		(*ob)[i].x -= speed;		//�ٴڼӵ��� ����

		if ((*ob)[i].x < 0 - R)	//��ġ �� ����
			(*ob)[i].x = (*itemPos)[(*ob)[i].pos].x;

		TransparentBlt(hMemDC, (*ob)[i].x, (*ob)[i].y, R, R, hMemDC2, 0, 0, obstacleBit[0].bmWidth,
			obstacleBit[0].bmHeight, RGB(255, 0, 255));	//������ ���

		if (ObstacleCollision(&(*ob)[i], player, R, R / 2))
			GameOver(hWnd, &time);
	}


	//�ѤѤ� ȭ�鿡 ������ ��� �ѤѤ�

	SelectObject(hMemDC2, hItem);

	for (int i = 0; i <= itemNum ; ++i)
	{
		(*itemPos)[i].x -= speed;		//������ �ӵ� -�ٴڼӵ��� ����
		if ((*itemPos)[i].x < 0 - R)
		{
			(*itemPos)[i].x = crt.right + R;
			(*itemPos)[i].visible = TRUE;
		}


		if ((*itemPos)[i].visible)
		{
			ItemCollision(&(*itemPos)[i], player, R);	//�浹üũ
			TransparentBlt(hMemDC, (*itemPos)[i].x, (*itemPos)[i].y, R, R, hMemDC2, 0, 0, itemBit.bmWidth, 
				itemBit.bmHeight, RGB(255, 0, 255));	//������ ���
		}
	}

	//�ѤѤ� �÷��̾� ��� �ѤѤ�
	if(*Jump == TRUE||DoubleJump==TRUE)
		SelectObject(hMemDC2, hPlayer[3]);
	else
		SelectObject(hMemDC2, hPlayer[i%3]);
	TransparentBlt(hMemDC, player.x - R, player.y - R, 2 * R, 2 * R, hMemDC2, 0, 0, bit.bmWidth, bit.bmHeight, RGB(255,0,255));
	i++;

	//�ӵ� ����
	if (i > 300)
		speed = 19;
	if (i > 600)
		speed = 22;
	//�ѤѤ� ������ ��� �ѤѤ�
	if (PrintScore == TRUE)
	{
		stop = clock();

		timer = int(stop - start) / CLOCKS_PER_SEC;

		SelectObject(hMemDC2, hScore);
		StretchBlt(hMemDC, crt.right / 2 - 200, crt.bottom / 2 - 200, 400, 400, 
			hMemDC2, 0, 0, scoreBit.bmWidth, scoreBit.bmHeight, SRCCOPY);

		wsprintf(buf, _T("%d"), timer); //�޸� �ð�
		TextOut(hMemDC, 600, 210, buf, lstrlen(buf));
		TextOut(hMemDC, 601 + 7*lstrlen(buf), 210, TEXT("�� x 5��"), 6);

		wsprintf(buf, _T("%d"), item_count); //���� �� ����
		TextOut(hMemDC, 600, 283, buf, lstrlen(buf));
		printf("%d", lstrlen(buf));
		TextOut(hMemDC, 601 + 7*lstrlen(buf), 283, TEXT("�� x 5��"), 6);

		total = (timer * 5) + (item_count * 5);

		wsprintf(buf, _T("%d"), total); //���� �� ����
		TextOut(hMemDC, 600, 415, buf, lstrlen(buf));
	}

	DeleteDC(hMemDC2);

	//�ѤѤѰ��� ���� �ѤѤ�
	//if (player.y + R > 600)
	//	GameOver(hWnd, &time);

	//�ѤѤ� �ϼ��� �׸� ��� �ѤѤ�
	BitBlt(hDC, 0, 0, crt.right	, crt.bottom, hMemDC, 0, 0, SRCCOPY);

	SelectObject(hMemDC,hOldBit);
	DeleteObject(hBit);
	DeleteDC(hMemDC);
	ReleaseDC(hWnd, hDC);
	//InvalidateRect(hWnd, NULL, FALSE);
}

//��ֹ�, ������ ������ ���ϴ�� �������ϴ� ��� �˾Ƴ���.

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
	case WM_SIZE:	//������ ũ�Ⱑ ����ɶ����� ���ҽ����� ��ǥ���� �缭��
	case WM_CREATE:		//(50,50)���� �����ؼ� 25�ʸ��� x������ 4�ȼ� y������ 5�ȼ� �� �����̰� ��
		GetClientRect(hWnd, &rt);	//ȭ���� ������ ����ü���� ����
		player.x = rt.right/5;
		player.y = rt.bottom/5*3;

		hPlayer[0] = (HBITMAP)LoadBitmap(hInst, MAKEINTRESOURCE(IDB_BITMAP1));	//�÷��̾� �̹���1
		hPlayer[1] = (HBITMAP)LoadBitmap(hInst, MAKEINTRESOURCE(IDB_BITMAP2));	//�÷��̾� �̹���2
		hPlayer[2] = (HBITMAP)LoadBitmap(hInst, MAKEINTRESOURCE(IDB_BITMAP3));	//�÷��̾� �̹���3
		hPlayer[3] = (HBITMAP)LoadBitmap(hInst, MAKEINTRESOURCE(IDB_BITMAP8));	//�÷��̾� �̹���4
		GetObject(hPlayer[0], sizeof(BITMAP), &bit);
		hFloor = (HBITMAP)LoadBitmap(hInst, MAKEINTRESOURCE(IDB_BITMAP4));		//�ٴ� �̹���
		GetObject(hFloor, sizeof(BITMAP), &fbit);
		hBackground = (HBITMAP)LoadBitmap(hInst, MAKEINTRESOURCE(IDB_BITMAP5));	//��� �̹���
		GetObject(hBackground, sizeof(BITMAP), &backgroundBit);
		hItem = (HBITMAP)LoadBitmap(hInst, MAKEINTRESOURCE(IDB_BITMAP6));	//������ �̹���
		GetObject(hItem, sizeof(BITMAP), &itemBit);
		hObstacle[0] = (HBITMAP)LoadBitmap(hInst, MAKEINTRESOURCE(IDB_BITMAP7));	//��ֹ� �̹���1
		GetObject(hObstacle[0], sizeof(BITMAP), &obstacleBit[0]);
		hScore = (HBITMAP)LoadBitmap(hInst, MAKEINTRESOURCE(IDB_BITMAP9));	//��ֹ� �̹���1
		GetObject(hScore, sizeof(BITMAP), &scoreBit);
		break;
	case WM_CHAR:
		switch (wParam)
		{
		case 's':
		case 'S':
			PlaySound(MAKEINTRESOURCE(IDR_WAVE1), hInst, SND_RESOURCE | SND_ASYNC);
			start = clock();
			SetTimer(hWnd, 1, 25, NULL);	//Ÿ�̸� ��ȣ, �ֱ�, Ÿ�̸� �Լ�
			PrintScore = FALSE;
			item_count = 0;
			Jump = FALSE;
			DoubleJump = FALSE;
			ReStart = TRUE;
			//�ѤѤ� �������� ��ǥ�� ���� �ѤѤ�
			for (int i = 0; i <= itemNum; ++i)
			{
				itemPos[i].x = rt.right / itemNum * i;	//�������� ��ġ ���ϱ�
				itemPos[i].y = player.y;
			}
			itemPos[ObInfo[0].pos].y = 200;
			itemPos[ObInfo[1].pos].y = 200;
			//�ѤѤ� ��ֹ��� ��ǥ�� ���� �Ѥ�
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
		OnTimer(hWnd, player, &Jump, &itemPos, &ObInfo);	//OnTimer�Լ� ȣ��
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
	player->y += 5 * t * t / 2;	//�̵��Ÿ� ���
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
