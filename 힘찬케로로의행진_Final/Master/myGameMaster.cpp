#include <windows.h>
#include <windowsx.h>
#include <ddraw.h>
#include <stdio.h>

#include "ddutil.h"

#include <dsound.h>
#include "dsutil.h"


#define _GetKeyState( vkey ) HIBYTE(GetAsyncKeyState( vkey ))
#define _GetKeyPush( vkey )  LOBYTE(GetAsyncKeyState( vkey ))

HWND MainHwnd;

LPDIRECTDRAW         DirectOBJ;
LPDIRECTDRAWSURFACE  RealScreen;
LPDIRECTDRAWSURFACE  BackScreen;
LPDIRECTDRAWSURFACE  SpriteImage;
LPDIRECTDRAWSURFACE  BackGround;
LPDIRECTDRAWSURFACE  Bullet;
LPDIRECTDRAWSURFACE  Monster;
LPDIRECTDRAWSURFACE  Number;

LPDIRECTDRAWCLIPPER	ClipScreen;

int gFullScreen = 0, Press = 0, Left = 0, Right = 0, Up = 0, Down = 0;
int gWidth = 720, gHeight = 405;
int posX = 100, posY = 300;
int score = 0;

////////////////////

LPDIRECTSOUND       SoundOBJ = NULL;
LPDIRECTSOUNDBUFFER SoundDSB = NULL;
DSBUFFERDESC        DSB_desc;

HSNDOBJ Sound[10];


BOOL _InitDirectSound(void)
{
	if (DirectSoundCreate(NULL, &SoundOBJ, NULL) == DS_OK)
	{
		if (SoundOBJ->SetCooperativeLevel(MainHwnd, DSSCL_PRIORITY) != DS_OK) return FALSE;

		memset(&DSB_desc, 0, sizeof(DSBUFFERDESC));
		DSB_desc.dwSize = sizeof(DSBUFFERDESC);
		DSB_desc.dwFlags = DSBCAPS_PRIMARYBUFFER | DSBCAPS_CTRLVOLUME | DSBCAPS_CTRLPAN;

		if (SoundOBJ->CreateSoundBuffer(&DSB_desc, &SoundDSB, NULL) != DS_OK) return FALSE;
		SoundDSB->SetVolume(DSBVOLUME_MAX); // DSBVOLUME_MIN
		SoundDSB->SetPan(DSBPAN_RIGHT);
		return TRUE;
	}
	return FALSE;
}

void _Play(int num)
{
	SndObjPlay(Sound[num], NULL);
}



BOOL Fail(HWND hwnd)
{
	ShowWindow(hwnd, SW_HIDE);
	MessageBox(hwnd, "DIRECT X �ʱ�ȭ�� �����߽��ϴ�.", "���� ������", MB_OK);
	DestroyWindow(hwnd);
	return FALSE;
}

void _ReleaseAll(void)
{
	if (DirectOBJ != NULL)
	{
		if (RealScreen != NULL)
		{
			RealScreen->Release();
			RealScreen = NULL;
		}
		if (SpriteImage != NULL)
		{
			SpriteImage->Release();
			SpriteImage = NULL;
		}
		if (BackGround != NULL)
		{
			BackGround->Release();
			BackGround = NULL;
		}
		DirectOBJ->Release();
		DirectOBJ = NULL;
	}
}

long FAR PASCAL WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int Step = 5;


	switch (message)
	{
	case	WM_KEYDOWN:
		switch (wParam)
		{
		case VK_ESCAPE:
		case VK_F12:
			PostMessage(hWnd, WM_CLOSE, 0, 0);
			return 0;
		case VK_UP:
			posY -= Step;
			Up = 1;
			return 0;

		case VK_DOWN:
			Down = 1;
			posY += Step;
			return 0;

		case VK_LEFT:
			posX -= Step;
			Left = 1;
			return 0;

		case VK_RIGHT:
			Right = 1;
			posX += Step;
			return 0;

		case VK_SPACE://공격
			Press = 1;
			_Play(2);
			break;
		}
		break;

	case    WM_DESTROY:  _ReleaseAll();
		PostQuitMessage(0);
		break;
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
}

BOOL _GameMode(HINSTANCE hInstance, int nCmdShow, int x, int y, int bpp)
{
	HRESULT result;
	WNDCLASS wc;
	DDSURFACEDESC ddsd;
	DDSCAPS ddscaps;
	LPDIRECTDRAW pdd;

	wc.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
	wc.lpfnWndProc = WindowProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInstance;
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = GetStockBrush(BLACK_BRUSH);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = "TEST";
	RegisterClass(&wc);


	if (gFullScreen) {
		if ((MainHwnd = CreateWindowEx(0, "TEST", NULL, WS_POPUP, 0, 0, GetSystemMetrics(SM_CXSCREEN),
			GetSystemMetrics(SM_CYSCREEN), NULL, NULL, hInstance, NULL)) == NULL)
			ExitProcess(1);
	}
	else {
		if ((MainHwnd = CreateWindow("TEST", "Master", WS_OVERLAPPEDWINDOW, 0, 0, x,
			y, NULL, NULL, hInstance, NULL)) == NULL)
			ExitProcess(1);
		SetWindowPos(MainHwnd, NULL, 100, 100, x, y, SWP_NOZORDER);
	}

	SetFocus(MainHwnd);
	ShowWindow(MainHwnd, nCmdShow);
	UpdateWindow(MainHwnd);
	//    ShowCursor( FALSE );

	result = DirectDrawCreate(NULL, &pdd, NULL);
	if (result != DD_OK) return Fail(MainHwnd);

	result = pdd->QueryInterface(IID_IDirectDraw, (LPVOID*)&DirectOBJ);
	if (result != DD_OK) return Fail(MainHwnd);


	if (gFullScreen) {
		result = DirectOBJ->SetCooperativeLevel(MainHwnd, DDSCL_EXCLUSIVE | DDSCL_FULLSCREEN);
		if (result != DD_OK) return Fail(MainHwnd);

		result = DirectOBJ->SetDisplayMode(x, y, bpp);
		if (result != DD_OK) return Fail(MainHwnd);

		memset(&ddsd, 0, sizeof(ddsd));
		ddsd.dwSize = sizeof(ddsd);
		ddsd.dwFlags = DDSD_CAPS | DDSD_BACKBUFFERCOUNT;
		ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE | DDSCAPS_FLIP | DDSCAPS_COMPLEX;
		ddsd.dwBackBufferCount = 1;

		result = DirectOBJ->CreateSurface(&ddsd, &RealScreen, NULL);
		if (result != DD_OK) return Fail(MainHwnd);

		memset(&ddscaps, 0, sizeof(ddscaps));
		ddscaps.dwCaps = DDSCAPS_BACKBUFFER;
		result = RealScreen->GetAttachedSurface(&ddscaps, &BackScreen);
		if (result != DD_OK) return Fail(MainHwnd);
	}
	else {
		result = DirectOBJ->SetCooperativeLevel(MainHwnd, DDSCL_NORMAL);
		if (result != DD_OK) return Fail(MainHwnd);

		memset(&ddsd, 0, sizeof(ddsd));
		ddsd.dwSize = sizeof(ddsd);
		ddsd.dwFlags = DDSD_CAPS;
		ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;
		ddsd.dwBackBufferCount = 0;

		result = DirectOBJ->CreateSurface(&ddsd, &RealScreen, NULL);
		if (result != DD_OK) return Fail(MainHwnd);

		memset(&ddsd, 0, sizeof(ddsd));
		ddsd.dwSize = sizeof(ddsd);
		ddsd.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH;
		ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN;
		ddsd.dwWidth = x;
		ddsd.dwHeight = y;
		result = DirectOBJ->CreateSurface(&ddsd, &BackScreen, NULL);
		if (result != DD_OK) return Fail(MainHwnd);

		result = DirectOBJ->CreateClipper(0, &ClipScreen, NULL);
		if (result != DD_OK) return Fail(MainHwnd);

		result = ClipScreen->SetHWnd(0, MainHwnd);
		if (result != DD_OK) return Fail(MainHwnd);

		result = RealScreen->SetClipper(ClipScreen);
		if (result != DD_OK) return Fail(MainHwnd);

		SetWindowPos(MainHwnd, NULL, 100, 100, x, y, SWP_NOZORDER | SWP_NOACTIVATE);
	}


	return TRUE;
}


extern void CommInit(int argc, char** argv);
extern void CommSend(char* sending);
extern void CommRecv(char* recvData);


void CALLBACK _GameProc(HWND hWnd, UINT message, UINT wParam, DWORD lParam)
{
	static int backFrame = 0;
	RECT BackRect1, BackRect2;
	RECT DispRect = { 0, 0, gWidth, gHeight };
	RECT SpriteRect, dstRect, WinRect, BulletRect;
	char sendData[200];
	BackRect1.left = backFrame;
	BackRect1.top = 0;
	BackRect1.right = 720;
	BackRect1.bottom = 405;
	BackRect2.left = 0;
	BackRect2.top = 0;
	BackRect2.right = backFrame;
	BackRect2.bottom = 405;
	backFrame = backFrame++;//배경이 이동하는 것 표현
	
	if (backFrame == 720) {
		backFrame = 0;
	}

	BackScreen->BltFast(0, 0, BackGround, &BackRect1, DDBLTFAST_WAIT | DDBLTFAST_NOCOLORKEY);
	BackScreen->BltFast(gWidth-backFrame, 0, BackGround, &BackRect2, DDBLTFAST_WAIT | DDBLTFAST_NOCOLORKEY);//배경을 2개 붙여 자연스럽게 움직이도록 표현



	static int Frame =1, FrameU, FrameD, FrameR, FrameL = 0;

	SpriteRect.left = (Frame-1) * 48;
	SpriteRect.top = 0;
	SpriteRect.right = SpriteRect.left + 48;
	SpriteRect.bottom = 64;
	if (Frame) {//가만히 있을 시 캐릭터의 프레임
		if (++Frame >= 3) {
			Frame = 0;
		}
		if (!Frame)
			Frame = 1;
	}
	if (Up) {//방향키 이동시 캐릭터 프레임 변화
		SpriteRect.left = FrameL * 48;
		SpriteRect.top = 192;
		SpriteRect.right = SpriteRect.left + 48;
		SpriteRect.bottom = 256;
		if (++FrameU >= 3) {
			FrameU = 0;
			Up = 0;
		}
	}
	if (Down) {
		SpriteRect.left = FrameD * 48;
		SpriteRect.top = 0;
		SpriteRect.right = SpriteRect.left + 48;
		SpriteRect.bottom = 64;
		if (++FrameD >= 3) {
			FrameD = 0;
			Down = 0;
		}
	}

	if (Right) {
		SpriteRect.left = FrameL * 48;
		SpriteRect.top = 128;
		SpriteRect.right = SpriteRect.left + 48;
		SpriteRect.bottom = 192;
		if (++FrameR >= 3) {
			FrameR = 0;
			Right = 0;
		}
	}

	if (Left) {
		SpriteRect.left = FrameR * 48;
		SpriteRect.top = 64;
		SpriteRect.right = SpriteRect.left + 48;
		SpriteRect.bottom = 128;
	
		if (++FrameL >= 3) {
			FrameL = 0;
			Left = 0;
		}
	}



	if (posX <= 50) posX = 50;
	if (posX > gWidth - 50) posX = gWidth - 50;
	if (posY <= 60) posY = 60;
	if (posY > gHeight - 60) posY = gHeight - 60;

	

	BackScreen->BltFast(posX - 50, posY - 35, SpriteImage, &SpriteRect, DDBLTFAST_WAIT | DDBLTFAST_SRCCOLORKEY);

	sprintf(sendData, "1,%d,%d,%d,%d,%d,%d,%d,%d", posX - 50, posY - 35, 0, 0, SpriteRect.left, SpriteRect.top, SpriteRect.right, SpriteRect.bottom);
	CommSend(sendData);


	static int stack = 0, SrcX = 0, SrcY = 0, Collision = 0, Hitted =0, Rd, BulletX = 0;
	int posx, posy, size, coll = 0, rd, bulletx = 0, bullety = 0, hit = 0;
	srand(10);

	if (Press) {
		BulletRect.left = 160;
		BulletRect.top = 5;
		BulletRect.right = BulletRect.left + 27;
		BulletRect.bottom = 25;
		bulletx = posX;
		bullety = posY;
		dstRect.left = bulletx - 20 + BulletX;
		dstRect.top = posY;
		dstRect.right = dstRect.left + 27;
		dstRect.bottom = dstRect.top + 20;
		BackScreen->BltFast(bulletx - 20 + BulletX, posY, Bullet, &BulletRect, DDBLTFAST_WAIT | DDBLTFAST_SRCCOLORKEY);
		sprintf(sendData, "2,%d,%d,%d,%d,%d,%d,%d,%d", dstRect.left, dstRect.top, dstRect.right, dstRect.bottom, BulletRect.left, BulletRect.top, BulletRect.right, BulletRect.bottom);
		CommSend(sendData);
		BulletX = BulletX + 5;
	}
	else {//총알의 초기화 (적과 닿을 경우)
		bulletx = 0;
		BulletX = 0;
	}




	for (int k = 0; k < 20; k++) {
		posx = (SrcX + rand()) % (gWidth - 60) + 30;
		posy = (SrcY + rand()) % (gWidth - 60) + 30;
		size = (rand() % 40) - 20;
		rd = rand() % 5;
		if (abs(posX - (posx + 30)) < 10 && abs(posY - (posy + 30)) < 10)
			coll = 1;

		if (abs((bulletx-20+BulletX) - (posx + 30)) < 40&& abs(posY - (posy + 30)) < 30) {
			hit = 1;
			if (((bulletx - 20 + BulletX) - (posx + 30)) >= 0)//적과 닿고 어느정도 시간차를 둔 후 Press초기화
				Press = 0;
			//Press = 0;
			score++;
			SpriteRect.left = 60 * rd;
			SpriteRect.top = 80;
			SpriteRect.right = 60 * rd + 60;
			SpriteRect.bottom = 140;
		}
		else {	
			SpriteRect.left = 60*rd;
			SpriteRect.top = 0;
			SpriteRect.right = 60*rd+60;
			SpriteRect.bottom = 80;
		}

		dstRect.left = posx;
		dstRect.top = posy;
		dstRect.right = dstRect.left + 60 + size;
		dstRect.bottom = dstRect.top + 80 + size;
		BackScreen->Blt(&dstRect, Monster, &SpriteRect, DDBLT_WAIT | DDBLT_KEYSRC, NULL);
		sprintf(sendData, "3,%d,%d,%d,%d,%d,%d,%d,%d", dstRect.left, dstRect.top, dstRect.right, dstRect.bottom, SpriteRect.left, SpriteRect.top, SpriteRect.right, SpriteRect.bottom);
		CommSend(sendData);
	}

	int one = score % 10;//점수표
	int ten = (score % 100) / 10;
	int hund = score / 100;
	if (one < 5) {
		SpriteRect.left = one * 60;
		SpriteRect.top = 0;
		SpriteRect.right = SpriteRect.left + 60;
		SpriteRect.bottom = 95;
	}
	else {
		SpriteRect.left = (one - 5) * 60;
		SpriteRect.top = 95;
		SpriteRect.right = SpriteRect.left + 60;
		SpriteRect.bottom = 190;
	}

	dstRect.left = 150;
	dstRect.top = 50;
	dstRect.right = dstRect.left + 30;
	dstRect.bottom = dstRect.top + 30;

	BackScreen->Blt(&dstRect, Number, &SpriteRect, DDBLT_WAIT | DDBLT_KEYSRC, NULL);

	sprintf(sendData, "4,%d,%d,%d,%d,%d,%d,%d,%d", dstRect.left, dstRect.top, dstRect.right, dstRect.bottom, SpriteRect.left, SpriteRect.top, SpriteRect.right, SpriteRect.bottom);
	CommSend(sendData);

	if (ten < 5) {
		SpriteRect.left = ten * 60;
		SpriteRect.top = 0;
		SpriteRect.right = SpriteRect.left + 60;
		SpriteRect.bottom = 95;
	}
	else {
		SpriteRect.left = (ten - 5) * 60;
		SpriteRect.top =95;
		SpriteRect.right = SpriteRect.left + 60;
		SpriteRect.bottom = 190;
	}
	dstRect.left = 100;
	dstRect.top = 50;
	dstRect.right = dstRect.left + 30;
	dstRect.bottom = dstRect.top + 30;

	BackScreen->Blt(&dstRect, Number, &SpriteRect, DDBLT_WAIT | DDBLT_KEYSRC, NULL);

	sprintf(sendData, "5,%d,%d,%d,%d,%d,%d,%d,%d", dstRect.left, dstRect.top, dstRect.right, dstRect.bottom, SpriteRect.left, SpriteRect.top, SpriteRect.right, SpriteRect.bottom);
	CommSend(sendData);

	if (hund < 5) {
		SpriteRect.left = hund * 60;
		SpriteRect.top = 0;
		SpriteRect.right = SpriteRect.left + 60;
		SpriteRect.bottom = 95;
	}
	else {
		SpriteRect.left = (hund - 5) * 60;
		SpriteRect.top = 95;
		SpriteRect.right = SpriteRect.left + 60;
		SpriteRect.bottom = 190;
	}

	dstRect.left = 50;
	dstRect.top = 50;
	dstRect.right = dstRect.left + 30;
	dstRect.bottom = dstRect.top + 30;

	BackScreen->Blt(&dstRect, Number, &SpriteRect, DDBLT_WAIT | DDBLT_KEYSRC, NULL);

	sprintf(sendData, "6,%d,%d,%d,%d,%d,%d,%d,%d", dstRect.left, dstRect.top, dstRect.right, dstRect.bottom, SpriteRect.left, SpriteRect.top, SpriteRect.right, SpriteRect.bottom);
	CommSend(sendData);


	if (coll) {//캐릭터와 적이 닿으면 게임이 종료됨.(PostMessage지우면 피격 소리 재생)
		if (!Collision) {
			Collision = 1;
			_Play(5);
		}
		PostMessage(hWnd, WM_CLOSE, 0, 0);
	}
	else
		Collision = 0;

	if (hit) {//총알과 적이 닿으면 피격소리 재생
		if (!Hitted) {
			Hitted = 1;
			_Play(4);
		}
	}
	else
		Hitted = 0;

	SrcX = SrcX - 3;

	if (gFullScreen)
		RealScreen->Flip(NULL, DDFLIP_WAIT);
	else {
		GetWindowRect(MainHwnd, &WinRect);
		RealScreen->Blt(&WinRect, BackScreen, &DispRect, DDBLT_WAIT, NULL);
	}
}




int PASCAL WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	MSG msg;

	if (!_GameMode(hInstance, nCmdShow, gWidth, gHeight, 32)) return FALSE;

	SpriteImage = DDLoadBitmap(DirectOBJ, "keroro1.BMP", 0, 0);
	BackGround = DDLoadBitmap(DirectOBJ, "back.BMP", 0, 0);
	Monster = DDLoadBitmap(DirectOBJ, "keroro2.BMP", 0, 0);
	Bullet = DDLoadBitmap(DirectOBJ, "bullet.BMP", 0, 0);
	Number = DDLoadBitmap(DirectOBJ, "num.BMP", 0, 0);

	DDSetColorKey(SpriteImage, RGB(0, 0, 0));
	DDSetColorKey(Bullet, RGB(0, 0, 0));
	DDSetColorKey(Monster, RGB(0, 0, 0));
	DDSetColorKey(Number, RGB(0, 0, 0));

	SetTimer(MainHwnd, 1, 30, _GameProc);

	CommInit(NULL, NULL);

	///////////////////

	if (_InitDirectSound())
	{
		Sound[0] = SndObjCreate(SoundOBJ, "music.wav", 1);
		Sound[1] = SndObjCreate(SoundOBJ, "LAND.WAV", 2);
		Sound[2] = SndObjCreate(SoundOBJ, "gun.WAV", 2);
		Sound[3] = SndObjCreate(SoundOBJ, "KNIFE1.WAV", 2);
		Sound[4] = SndObjCreate(SoundOBJ, "damage.WAV", 2);
		Sound[5] = SndObjCreate(SoundOBJ, "DAMAGE1.WAV", 2);

		        SndObjPlay( Sound[0], DSBPLAY_LOOPING );
	}
	//////////////////

	while (!_GetKeyState(VK_ESCAPE))
	{

		if (PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE))
		{
			if (!GetMessage(&msg, NULL, 0, 0)) return msg.wParam;

			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		//        else _GameProc();
	}
	DestroyWindow(MainHwnd);

	return TRUE;
}