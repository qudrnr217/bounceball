// Ÿ�̸Ӹ� �߻�����, �׶����� ��ƽ�� ���� ��ġ�� ����ϴ�(���ӵ� ���� ��������) �ٿ ������ ���α׷�
#pragma once
#include <windows.h>
#define R 20
#define BOUNCE 1
#define W 20
#define H 200


LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
DWORD WINAPI ThreadFunc(LPVOID prc);
HBITMAP hBit;
HINSTANCE g_hInst; //hinstance�� ����� baseImage�ּҸ� ��Ÿ���� ��
HWND hWndMain, hWnd2; //hwnd�� �ϳ��� ������ �ǹ��ϴ� ��, ���ο� �������, ��, �Է¹ڽ� ��
HANDLE hThread;
LPCTSTR lpszClass = TEXT("Bounce2"); //Long Poiinter Constance(���) TSTR:TCHAR�� ������
//�� const TCHAR*�� ���Ѵ�.

RECT crt; //2���� �簢�� ������ ��Ÿ���� ���� ���ø� Ŭ����
int x, y;
int xv, yv;
int flag = 0;
int btnflag = 0;
int sX[2];
int sY[2];

typedef struct tagtrans_struct {
	int x;
	int y;
	int xv;
	int yv;
}trans_struct;
COPYDATASTRUCT cData;
PCOPYDATASTRUCT pcData;
trans_struct trans;

//------------------�Լ� ���� ��---------------------

void moveBall();
void collision();
void DoCreateMain(HWND hWnd);
void DoButtonMain(HWND hWnd);
void DoPaintMain(HWND hWnd);
void OnTimer(HWND hWnd);
void sendmsg();


int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdParam, int nCmdShow) {
	//APIENTRY = FAR PASCAL = WINAPI, pascal���Լ��� ���ڰ� ȣ��Ǵ� ������ ����.
	//pascal�� ���ʿ��� ������ __cdecl�� �ݴ� funcA(10,20,30)�� pascal���ϸ� �޸𸮿� 10 20 30���ɷ� ����
	HWND hWnd;
	MSG Message;
	WNDCLASS WndClass; //������ Ŭ������ �������� Ư���� ������ ����ü�̴�.
	g_hInst = hInstance;

	WndClass.cbClsExtra = 0;
	WndClass.cbWndExtra = 0;
	WndClass.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	WndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	WndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	WndClass.hInstance = hInstance;
	WndClass.lpfnWndProc = (WNDPROC)WndProc;
	WndClass.lpszClassName = lpszClass;
	WndClass.lpszMenuName = NULL;
	WndClass.style = CS_HREDRAW | CS_VREDRAW;
	RegisterClass(&WndClass);

	hWnd = CreateWindow(lpszClass, lpszClass, WS_BORDER | WS_CAPTION | WS_SYSMENU,
		CW_USEDEFAULT, CW_USEDEFAULT, 600, 600, //������ ũ�Ⱑ 600 600
		NULL, (HMENU)NULL, hInstance, NULL);
	ShowWindow(hWnd, nCmdShow);
	hWndMain = hWnd;

	while (GetMessage(&Message, 0, 0, 0)) {
		TranslateMessage(&Message);
		DispatchMessage(&Message);
	}
	return (int)Message.wParam;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam)
//LPARAM�� x,y ��ǥ��� �� ���ִµ� ����ǥ�� lParam���� ���� �̶� ���� 2����Ʈ�� y ���� 2����Ʈ�� x�� ���� ����
//wParam�� Ű����, ���콺 �� ���������� ���� �������� ���� ex)WM_LBUTTONDOWN
{
	COPYDATASTRUCT* pcds;

	switch (iMessage) {

	case WM_CREATE:
		DoCreateMain(hWnd);
		break;
	case WM_PAINT:
		DoPaintMain(hWnd);
		break;
	case WM_LBUTTONDOWN:
		DoButtonMain(hWnd);
		break;
	case WM_COPYDATA: //������ ������ ����
		pcData = (PCOPYDATASTRUCT)lParam;
		if (pcData->dwData)
		{
			x = (int)((trans_struct*)(pcData->lpData))->x;
			y = (int)((trans_struct*)(pcData->lpData))->y;
			xv = (int)((trans_struct*)(pcData->lpData))->xv;
			yv = (int)((trans_struct*)(pcData->lpData))->yv;
		}
		break;
	case WM_TIMER:
		OnTimer(hWnd);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		KillTimer(hWnd, 0);
		CloseHandle(hThread);
		return 0;
	}
	return(DefWindowProc(hWnd, iMessage, wParam, lParam));
}

void sendmsg()
{
	trans.x = x;
	trans.y = y;
	trans.xv = xv;
	trans.yv = yv;

	cData.dwData = 1;
	cData.cbData = sizeof(trans);
	cData.lpData = &trans;
	HWND Dispatch = FindWindow(NULL, TEXT("bounce1")); //�ϴ� findwindow�� �������� �뵵�� �������� ���� ����� ������ Ư�� ���α׷��� �������ΰ��� �˾Ƴ��� ���ؼ��Դϴ�
	SendMessage(Dispatch, WM_COPYDATA, (WPARAM)(HWND)0, (LPARAM)(LPVOID)&cData);

}

void DoCreateMain(HWND hWnd)
{
	GetClientRect(hWnd, &crt);

	x = 80;	//���� x ��ǥ �ʱⰪ
	y = 250;//���� y ��ǥ �ʱⰪ

	xv = (rand() % 4) + 5; //x���� �ӵ�
	yv = (rand() % 4) + 5; //y���� �ӵ�
}

void DoPaintMain(HWND hWnd)
{
	HDC hdc, hMemDC;
	PAINTSTRUCT ps;
	HBITMAP OldBit;

	hdc = BeginPaint(hWnd, &ps);

	hMemDC = CreateCompatibleDC(hdc);
	OldBit = (HBITMAP)SelectObject(hMemDC, hBit);
	BitBlt(hdc, 0, 0, crt.right, crt.bottom, hMemDC, 0, 0, SRCCOPY);

	SelectObject(hMemDC, OldBit);
	DeleteDC(hMemDC);
	EndPaint(hWnd, &ps);
	return;
}

void DoButtonMain(HWND hWnd)
{
	if (btnflag == 0) {
		MessageBox(hWnd, TEXT("���α׷��� ����Ǿ����ϴ�."), TEXT("OK"), MB_OK);
		SetTimer(hWnd, 1, 25, NULL);
	}
	if (btnflag > 0) {
		x = 250;
		y = 200;
		xv *= -(BOUNCE);
		flag = 0;
	}
	btnflag = 1;

}

void OnTimer(HWND hWnd)
{
	TCHAR str[123];
	HDC hdc, hMemDC;
	HBITMAP OldBit;
	HPEN hPen, OldPen;
	HBRUSH hBrush, OldBrush;

	hdc = GetDC(hWnd);
	GetClientRect(hWnd, &crt);
	if (hBit == NULL)
	{
		hBit = CreateCompatibleBitmap(hdc, crt.right, crt.bottom);
	}
	hMemDC = CreateCompatibleDC(hdc);
	OldBit = (HBITMAP)SelectObject(hMemDC, hBit);

	FillRect(hMemDC, &crt, GetSysColorBrush(COLOR_WINDOW));

	hPen = CreatePen(PS_INSIDEFRAME, 2, RGB(0, 0, 0));
	OldPen = (HPEN)SelectObject(hMemDC, hPen);
	hBrush = CreateSolidBrush(RGB(255, 120, 0));
	OldBrush = (HBRUSH)SelectObject(hMemDC, hBrush);
	Ellipse(hMemDC, x - R - 600, y - R, x + R - 600, y + R);	 //���׸���


	hBrush = CreateSolidBrush(RGB(255, 255, 0));
	OldBrush = (HBRUSH)SelectObject(hMemDC, hBrush);
	//Rectangle(hMemDC, sX[0], sY[0], sX[0] + W, sY[0] + H); //��ƽ �׸���
	//Rectangle(hMemDC, sX[1], sY[1], sX[1] + W, sY[1] + H); //��ƽ �׸���


	DeleteObject(SelectObject(hMemDC, OldPen));
	DeleteObject(SelectObject(hMemDC, OldBrush));

	wsprintf(str, TEXT("%d, %d, %d, %d"), x, y, xv, yv);
	TextOut(hMemDC, 100, 50, str, lstrlen(str));


	SelectObject(hMemDC, OldBit);
	DeleteDC(hMemDC);
	ReleaseDC(hWnd, hdc);
	InvalidateRect(hWnd, NULL, FALSE);

	//moveStick(); //��ƽ�� ������
	moveBall();  //���� ������
	//sendmsg();
	collision();
	//sendmsg();


}


void moveBall()
{
	if (y <= R || y >= crt.bottom - R) {
		yv *= -(BOUNCE);
		if (y <= R) y = R;
		if (y >= crt.bottom - R) y = crt.bottom - R;
	}
	x += (int)xv;
	y += (int)yv;
	//sendmsg();

}

void collision()
{
	//if (flag == 0)
	//{
	if (x - 600 > crt.right && y > crt.top && y < crt.bottom)
	{
		x -= ((R / 2));
		xv *= (-BOUNCE);
		sendmsg();
	}

	//	}

	if (x - 600 < crt.left - R) //��� ���
	{
		//flag = 0;
		//sendmsg();
	}
}


