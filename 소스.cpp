#include <windows.h>
#include <TCHAR.H>
#include <stdio.h>
#include <stdlib.h>
#include "resource.h"

#pragma comment(linker, "/entry:WinMainCRTStartup /subsystem:console")	//�ܼ�â ���
#pragma comment(lib, "msimg32.lib")
#pragma comment(lib,"winmm.lib")

#define itemNum 8	//�� ȭ�鿡 ��µǴ� ������ ����

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
void OnTimer(HWND, struct Character, BOOL*, struct Item**);		//�޸� ��ø� �̿��� hBit�� �̸� �׷� ���� �Լ�
BOOL CheckCollision(int n1, int n2, int m1, int m2);
void Gravity(struct Character* player, int t);
BOOL GameOver(int );
void ItemCollision(struct Item* item, struct Character player, int R);	//�����۰� �÷��̾��� �浹�� �����ϴ� �Լ�

HINSTANCE hInst;
BOOL MakeBitmap;
HBITMAP hPlayer[3],	//�÷��̾� �̹���
hFloor, hBackground,	//�ٴ�, ���
hItem;					//������
BITMAP bit, fbit, backgroundBit, itemBit;

struct Character
{
	int x, y;	//ĳ������ ��ǥ��
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
	TCHAR titleName[10] = L"���� �⸻����";
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


void OnTimer(HWND hWnd, struct Character player, BOOL* Jump, struct Item** itemPos)		//�޸� ��ø� �̿��� hBit�� �̸� �׷� ���� �Լ�
{
	HDC hDC, hMemDC, hMemDC2;
	static HBITMAP hOldBit, hBit;
	RECT crt;
	static int xi, Floor1X, Floor2X, FloorY;	//�÷��̾��� �̵���
	hDC = GetDC(hWnd);
	GetClientRect(hWnd, &crt);	//ȭ���� ������ ����ü���� ����
	const int R = crt.bottom / 8, Hole = crt.bottom/3;
	static int i, w, time, t, yi;
	static BOOL GRAVITY = FALSE;

	hBit = CreateCompatibleBitmap(hDC, crt.right, crt.bottom);	//ȭ�� ũ���� ��Ʈ���� ����

	hMemDC = CreateCompatibleDC(hDC);								//�޸𸮵�ÿ� ������� ��Ʈ���� ����
	hOldBit = (HBITMAP)SelectObject(hMemDC, hBit);					//oldBit�� ����
	hMemDC2 = CreateCompatibleDC(hDC);

	xi +=5;	//1�� ����
	Floor1X -= 10;
	Floor2X -= 10;

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
	if (*Jump==TRUE)
	{
		GRAVITY = FALSE;
		t++;
		yi = -40 * t + 5 * t * t / 2;
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

	//�ѤѤ� ȭ�鿡 ������ ��� �ѤѤ�

	SelectObject(hMemDC2, hItem);

	for (int i = 0; i < itemNum + 1; ++i)
	{
		(*itemPos)[i].x -= 10;
		if ((*itemPos)[i].x < 0 - R)
		{
			(*itemPos)[i].x = crt.right + R;
			(*itemPos)[i].visible = TRUE;
		}

		ItemCollision(&(*itemPos)[i], player, R);	//�浹üũ

		if ((*itemPos)[i].visible)
			TransparentBlt(hMemDC, (*itemPos)[i].x, (*itemPos)[i].y, R, R, hMemDC2, 0, 0, itemBit.bmWidth, itemBit.bmHeight, RGB(255, 0, 255));	//������ ���
	}

	//�ѤѤ� �÷��̾� ��� �ѤѤ�
	SelectObject(hMemDC2, hPlayer[i%3]);
	TransparentBlt(hMemDC, player.x - R, player.y - R, 2 * R, 2 * R, hMemDC2, 0, 0, bit.bmWidth, bit.bmHeight, RGB(255,0,255));
	i++;

	DeleteDC(hMemDC2);
	//�ѤѤѰ��� ���� �ѤѤ�
	if (GameOver(player.y) == TRUE)
	{
		KillTimer(hWnd, 1);
		PlaySound(MAKEINTRESOURCE(IDR_WAVE2), hInst, SND_RESOURCE | SND_ASYNC);
		time = 0;
	}
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
	RECT rt;
	static struct Character player;
	static BOOL Jump = FALSE;
	static Item* itemPos = (Item*)malloc(sizeof(Item) * (itemNum + 1));

	switch (uMsg)
	{
	case WM_SIZE:	//������ ũ�Ⱑ ����ɶ����� ���ҽ����� ��ǥ���� �缭��
	case WM_CREATE:		//(50,50)���� �����ؼ� 25�ʸ��� x������ 4�ȼ� y������ 5�ȼ� �� �����̰� ��
		GetClientRect(hWnd, &rt);	//ȭ���� ������ ����ü���� ����
		player.x = rt.right/5;
		player.y = rt.bottom/5*3;
		MakeBitmap = TRUE;

		hPlayer[0] = (HBITMAP)LoadBitmap(hInst, MAKEINTRESOURCE(IDB_BITMAP1));	//�÷��̾� �̹���1
		hPlayer[1] = (HBITMAP)LoadBitmap(hInst, MAKEINTRESOURCE(IDB_BITMAP2));	//�÷��̾� �̹���2
		hPlayer[2] = (HBITMAP)LoadBitmap(hInst, MAKEINTRESOURCE(IDB_BITMAP3));	//�÷��̾� �̹���3
		GetObject(hPlayer[0], sizeof(BITMAP), &bit);
		hFloor = (HBITMAP)LoadBitmap(hInst, MAKEINTRESOURCE(IDB_BITMAP4));		//�ٴ� �̹���
		GetObject(hFloor, sizeof(BITMAP), &fbit);
		hBackground = (HBITMAP)LoadBitmap(hInst, MAKEINTRESOURCE(IDB_BITMAP5));	//��� �̹���
		GetObject(hBackground, sizeof(BITMAP), &backgroundBit);
		hItem = (HBITMAP)LoadBitmap(hInst, MAKEINTRESOURCE(IDB_BITMAP6));	//������ �̹���
		GetObject(hItem, sizeof(BITMAP), &itemBit);

		for (int i = 0; i <= itemNum; ++i)
		{
			itemPos[i].x = rt.right / itemNum * i;	//�������� ��ġ ���ϱ�
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
			SetTimer(hWnd, 1, 25, NULL);	//Ÿ�̸� ��ȣ, �ֱ�, Ÿ�̸� �Լ�
			break;
		case 'q':
			KillTimer(hWnd, 1);
		}
		break;
	case WM_TIMER:
		OnTimer(hWnd, player, &Jump, &itemPos);	//OnTimer�Լ� ȣ��
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
	BOOL GAMEOVER = FALSE;
	if (y > 600)
		GAMEOVER = TRUE;
	return GAMEOVER;
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
	//printf("%d %d %d %d\n", item.x, player.x, item.y, player.y);
	//printf("%d %d\n", R * R * 9 / 4, (item.x - player.x) * (item.x - player.x) + (item.y - player.y) * (item.y - player.y));
	if ((R * R * 9 / 4) >= ((item->x - player.x) * (item->x - player.x) + (item->y - player.y) * (item->y - player.y)))
		item->visible = FALSE;

}