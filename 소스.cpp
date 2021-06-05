#include <windows.h>
#include <TCHAR.H>
#include <stdio.h>
#include <stdlib.h>
#include "resource.h"

#pragma comment(linker, "/entry:WinMainCRTStartup /subsystem:console")	//�ܼ�â ���

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
void DrawBitmap(HDC hdc, int x, int y, HBITMAP hBit);
void OnTimer(HWND, struct Character);		//�޸� ��ø� �̿��� hBit�� �̸� �׷� ���� �Լ�
HINSTANCE hInst;
HBITMAP hBit;
BOOL MakeBitmap;

struct Character
{
	int x, y;	//ĳ������ ��ǥ��
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


void OnTimer(HWND hWnd, struct Character player)		//�޸� ��ø� �̿��� hBit�� �̸� �׷� ���� �Լ�
{
	HDC hDC, hMemDC, hMemDC2;
	HBITMAP oldBit;
	RECT crt;
	BITMAP bit;
	static HBITMAP hPlayer[3];	//�÷��̾� �̹���
	static int xi = 5, yi;	//�÷��̾��� �̵���
	hDC = GetDC(hWnd);
	GetClientRect(hWnd, &crt);	//ȭ���� ������ ����ü���� ����
	GetObject(hPlayer[0], sizeof(BITMAP), &bit);
	const int R = crt.bottom / 10;
	static int i;

	hPlayer[0] = (HBITMAP)LoadBitmap(hInst, MAKEINTRESOURCE(IDB_BITMAP1));	//�÷��̾� �̹���1
	hPlayer[1] = (HBITMAP)LoadBitmap(hInst, MAKEINTRESOURCE(IDB_BITMAP2));	//�÷��̾� �̹���2
	hPlayer[2] = (HBITMAP)LoadBitmap(hInst, MAKEINTRESOURCE(IDB_BITMAP3));	//�÷��̾� �̹���3

	if (MakeBitmap == TRUE)
	{
		hBit = CreateCompatibleBitmap(hDC, crt.right, crt.bottom);	//ȭ�� ũ���� ��Ʈ���� ����
		MakeBitmap = FALSE;
	}

	hMemDC = CreateCompatibleDC(hDC);								//�޸𸮵�ÿ� ������� ��Ʈ���� ����
	oldBit = (HBITMAP)SelectObject(hMemDC, hBit);					//oldBit�� ����
	hMemDC2 = CreateCompatibleDC(hDC);

	xi += 10;	//10�� ����
	player.x += xi;	//��ġ�� �̵��Ÿ��� ������

	if (player.x - R >= crt.right)
		xi = -crt.right / 5 - R;

	FillRect(hMemDC, &crt, GetSysColorBrush(COLOR_WINDOW));

	//�ѤѤ� ȭ�鿡 �÷��̾� ��� �ѤѤ�
	SelectObject(hMemDC2, hPlayer[i%3]);
	StretchBlt(hMemDC, player.x - R, player.y - R, 2 * R, 2 * R, hMemDC2, 0, 0, bit.bmWidth, bit.bmHeight, SRCCOPY);
	i++;
	//Ellipse(hMemDC, player.x - R, player.y - R, player.x + R, player.y + R);

	//�ѤѤ� �ϼ��� �׸� ��� �ѤѤ�
	BitBlt(hDC, 0, 0, crt.right	, crt.bottom, hMemDC, 0, 0, SRCCOPY);

	SelectObject(hMemDC, oldBit);
	DeleteDC(hMemDC);
	ReleaseDC(hWnd, hDC);
	//InvalidateRect(hWnd, NULL, FALSE);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	HDC hDC;
	PAINTSTRUCT ps;
	RECT rt;
	static struct Character player;

	switch (uMsg)
	{
	case WM_SIZE:	//������ ũ�Ⱑ ����ɶ����� ���ҽ����� ��ǥ���� �缭��
	case WM_CREATE:		//(50,50)���� �����ؼ� 25�ʸ��� x������ 4�ȼ� y������ 5�ȼ� �� �����̰� ��
		GetClientRect(hWnd, &rt);	//ȭ���� ������ ����ü���� ����
		player.x = rt.right/5;
		player.y = rt.bottom/5*3;
		MakeBitmap = TRUE;
		break;
	case WM_CHAR:
		switch (wParam)
		{
		case 's':
		case 'S':
			SetTimer(hWnd, 1, 25, NULL);	//Ÿ�̸� ��ȣ, �ֱ�, Ÿ�̸� �Լ�
			break;
		}
		break;
	case WM_TIMER:
		OnTimer(hWnd, player);	//OnTimer�Լ� ȣ��
		break;
	case WM_PAINT:
		hDC = BeginPaint(hWnd, &ps);
		//if (hBit) DrawBitmap(hDC, 0, 0, hBit);		//DrawBitmap�Լ��� �׷��� ��Ʈ���� �����
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
