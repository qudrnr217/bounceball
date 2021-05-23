// 타이머를 발생시켜, 그때마다 스틱과 공의 위치를 계산하는(가속도 등이 전혀없는) 바운스 윈도우 프로그램
#pragma once
#include <windows.h>
#define R 20
#define BOUNCE 1
#define W 20
#define H 200


LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
DWORD WINAPI ThreadFunc(LPVOID prc);
HBITMAP hBit;
HINSTANCE g_hInst; //hinstance는 모듈의 baseImage주소를 나타내는 값
HWND hWndMain, hWnd2; //hwnd는 하나의 윈도를 의미하는 것, 내부에 여러요소, 라벨, 입력박스 등
HANDLE hThread;
LPCTSTR lpszClass = TEXT("Bounce2"); //Long Poiinter Constance(상수) TSTR:TCHAR의 포인터
//즉 const TCHAR*를 뜻한다.

RECT crt; //2차원 사각형 정보를 나타내기 위한 템플릿 클래스
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

//------------------함수 쓰는 곳---------------------

void moveBall();
void collision();
void DoCreateMain(HWND hWnd);
void DoButtonMain(HWND hWnd);
void DoPaintMain(HWND hWnd);
void OnTimer(HWND hWnd);
void sendmsg();


int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdParam, int nCmdShow) {
	//APIENTRY = FAR PASCAL = WINAPI, pascal은함수의 인자가 호출되는 순서를 정함.
	//pascal은 왼쪽에서 오른쪽 __cdecl은 반대 funcA(10,20,30)을 pascal로하면 메모리에 10 20 30수능로 넣음
	HWND hWnd;
	MSG Message;
	WNDCLASS WndClass; //윈도우 클래스는 윈도우의 특성을 정의한 구조체이다.
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
		CW_USEDEFAULT, CW_USEDEFAULT, 600, 600, //윈도우 크기가 600 600
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
//LPARAM은 x,y 좌표라고 할 수있는데 이좌표는 lParam으로 저장 이때 앞의 2바이트는 y 뒤의 2바이트는 x의 값을 저장
//wParam은 키보드, 마우스 등 여러가지가 같이 눌렸을대 동작 ex)WM_LBUTTONDOWN
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
	case WM_COPYDATA: //상대방의 정보를 받음
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
	HWND Dispatch = FindWindow(NULL, TEXT("bounce1")); //일단 findwindow를 여러가지 용도로 쓰겠지만 제가 사용한 목적은 특정 프로그램이 실행중인가를 알아내기 위해서입니다
	SendMessage(Dispatch, WM_COPYDATA, (WPARAM)(HWND)0, (LPARAM)(LPVOID)&cData);

}

void DoCreateMain(HWND hWnd)
{
	GetClientRect(hWnd, &crt);

	x = 80;	//공의 x 좌표 초기값
	y = 250;//공의 y 좌표 초기값

	xv = (rand() % 4) + 5; //x벡터 속도
	yv = (rand() % 4) + 5; //y벡터 속도
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
		MessageBox(hWnd, TEXT("프로그램이 실행되었습니다."), TEXT("OK"), MB_OK);
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
	Ellipse(hMemDC, x - R - 600, y - R, x + R - 600, y + R);	 //공그리기


	hBrush = CreateSolidBrush(RGB(255, 255, 0));
	OldBrush = (HBRUSH)SelectObject(hMemDC, hBrush);
	//Rectangle(hMemDC, sX[0], sY[0], sX[0] + W, sY[0] + H); //스틱 그리기
	//Rectangle(hMemDC, sX[1], sY[1], sX[1] + W, sY[1] + H); //스틱 그리기


	DeleteObject(SelectObject(hMemDC, OldPen));
	DeleteObject(SelectObject(hMemDC, OldBrush));

	wsprintf(str, TEXT("%d, %d, %d, %d"), x, y, xv, yv);
	TextOut(hMemDC, 100, 50, str, lstrlen(str));


	SelectObject(hMemDC, OldBit);
	DeleteDC(hMemDC);
	ReleaseDC(hWnd, hdc);
	InvalidateRect(hWnd, NULL, FALSE);

	//moveStick(); //스틱의 움직임
	moveBall();  //공의 움직임
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

	if (x - 600 < crt.left - R) //벗어난 경우
	{
		//flag = 0;
		//sendmsg();
	}
}


