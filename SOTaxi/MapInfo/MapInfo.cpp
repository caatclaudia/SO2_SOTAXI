#include <windows.h>
#include <tchar.h>
#include <windowsx.h>
#include "MapInfo.h"

HANDLE hThreadAtualizaMapa;
DADOS dados;
INFO info;

void(*ptr_register)(TCHAR*, int);

LRESULT CALLBACK TrataEventos(HWND, UINT, WPARAM, LPARAM);

TCHAR szProgName[] = TEXT("MapInfo");

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR lpCmdLine, int nCmdShow) {
	dados.terminar = 0;

	HINSTANCE hLib;

	hLib = LoadLibrary(PATH_DLL);
	if (hLib == NULL)
		return 0;

	ptr_register = (void(*)(TCHAR*, int))GetProcAddress(hLib, "dll_register");

	hMutex = CreateMutex(NULL, FALSE, NOME_MUTEXMAPA);
	if (hMutex == NULL) {
		_tprintf(TEXT("\n[ERRO] Erro ao criar Mutex!\n"));
		return -1;
	}
	ptr_register((TCHAR*)NOME_MUTEXMAPA, 1);
	WaitForSingleObject(hMutex, INFINITE);
	ReleaseMutex(hMutex);

	EspMapa = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(dados.mapa), SHM_NAME);
	if (EspMapa == NULL)
	{
		_tprintf(TEXT("\n[ERRO] Erro ao criar FileMapping!\n"));
		return -1;
	}
	ptr_register((TCHAR*)SHM_NAME, 6);

	shared = (MAPA*)MapViewOfFile(EspMapa, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(dados.mapa));
	if (shared == NULL)
	{
		_tprintf(TEXT("\n[ERRO] Erro em MapViewOfFile!\n"));
		CloseHandle(EspMapa);
		return -1;
	}
	ptr_register((TCHAR*)SHM_NAME, 7);

	recebeMapa(&dados);

	hThreadAtualizaMapa = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ThreadAtualizaMapa, (LPVOID)&dados, 0, NULL);
	if (hThreadAtualizaMapa == NULL) {
		_tprintf(TEXT("\n[ERRO] Erro ao lançar Thread!\n"));
		return 0;
	}


	HWND hWnd;
	MSG lpMsg;
	WNDCLASSEX wcApp;

	wcApp.cbSize = sizeof(WNDCLASSEX);
	wcApp.hInstance = hInst;
	wcApp.lpszClassName = szProgName;
	wcApp.lpfnWndProc = TrataEventos;
	wcApp.style = CS_HREDRAW | CS_VREDRAW;
	wcApp.hIcon = LoadIcon(NULL, IDI_APPLICATION);

	wcApp.hIconSm = LoadIcon(NULL, IDI_INFORMATION);

	wcApp.hCursor = LoadCursor(NULL, IDC_ARROW);

	wcApp.lpszMenuName = NULL;

	wcApp.cbClsExtra = 0;
	wcApp.cbWndExtra = 0;
	wcApp.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);

	if (!RegisterClassEx(&wcApp))
		return(0);

	hWnd = CreateWindow(
		szProgName,
		TEXT("MapInfo"),
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		(HWND)HWND_DESKTOP,
		(HMENU)NULL,
		(HINSTANCE)hInst,
		0);

	ShowWindow(hWnd, nCmdShow);

	UpdateWindow(hWnd);

	while (GetMessage(&lpMsg, NULL, 0, 0)) {
		TranslateMessage(&lpMsg);
		DispatchMessage(&lpMsg);
	}

	HANDLE ghEvents[2];
	ghEvents[0] = hThreadAtualizaMapa;
	WaitForMultipleObjects(1, ghEvents, TRUE, INFINITE);



	_tprintf(TEXT("\nPrima uma tecla...\n"));
	_gettch();

	CloseHandle(EspMapa);
	CloseHandle(atualizaMap);
	FreeLibrary(hLib);

	return((int)lpMsg.wParam);
}

LRESULT CALLBACK TrataEventos(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam) {
	HDC hdc;
	RECT rect;
	PAINTSTRUCT ps;
	HFONT hFont;
	TCHAR caract;
	int xPos = 0, yPos = 0;

	hFont = CreateFont(12, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_OUTLINE_PRECIS,
		CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, VARIABLE_PITCH, TEXT("Impact"));

	switch (messg) {
	//SE FOR PASSAGEIRO MOSTRA DESTINO E (TAXI QUE O FOR BUSCAR)
	case WM_LBUTTONDOWN: {
		xPos = GET_X_LPARAM(lParam);
		yPos = GET_Y_LPARAM(lParam);

		int x = (xPos - 80) / 8;
		int y = (yPos - 15) / 9;
		for (int i = 0; i < info.npassageiros; i++) {
			if (info.passageiros[i].X == x && info.passageiros[i].Y == y) {
				//MOSTRA DESTINO E (TAXI QUE O FOR BUSCAR)
			}
		}

		break;
	}
	//SE FOR TAXI MOSTRA MATRICULA E (DESTINO)
	case WM_MOUSEMOVE: {
		xPos = GET_X_LPARAM(lParam);
		yPos = GET_Y_LPARAM(lParam);

		int x = (xPos - 80) / 8;
		int y = (yPos - 15) / 9;
		for (int i = 0; i < info.ntaxis; i++) {
			if (info.taxis[i].X == x && info.taxis[i].Y == y) {
				//MOSTRA MATRICULA E (DESTINO)
			}
		}

		break;
	}
	case WM_PAINT: {
		hdc = BeginPaint(hWnd, &ps);
		SetTextColor(hdc, RGB(0, 0, 0));
		SelectObject(hdc, hFont);
		SetBkMode(hdc, TRANSPARENT);
		xPos = 0;
		yPos = 0;
		for (int i = 0; i < tamanhoMapa * tamanhoMapa; i++) {
			caract = shared[i].caracter;
			rect.left = 80 + (8 * xPos);
			rect.top = 15 + (9 * yPos);
			DrawText(hdc, &caract, 1, &rect, DT_SINGLELINE | DT_NOCLIP);
			xPos++;
			if (xPos == (tamanhoMapa + 1)) {
				yPos++;
				xPos = 0;
			}
		}
		EndPaint(hWnd, &ps);
		break;
	}
	case WM_CLOSE: {
		int value = MessageBox(hWnd, TEXT("Tem a certeza que deseja sair?"), TEXT("Confirmação"), MB_ICONQUESTION | MB_YESNO);

		if (value == IDYES)
		{
			DestroyWindow(hWnd);
		}

		break;
	}
	case WM_DESTROY: // Destruir a janela e terminar o programa
		TerminateThread(hThreadAtualizaMapa, 0);
		PostQuitMessage(0);
		break;
	default:

		return DefWindowProc(hWnd, messg, wParam, lParam);
		break;
	}
	return(0);
}

void recebeMapa(DADOS* dados) {
	EspInfo = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(INFO), SHM_INFO);
	if (EspInfo == NULL)
	{
		_tprintf(TEXT("\n[ERRO] Erro ao criar FileMapping!\n"));
		return;
	}
	ptr_register((TCHAR*)SHM_INFO, 6);

	sharedInfo = (INFO*)MapViewOfFile(EspInfo, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(INFO));
	if (sharedInfo == NULL)
	{
		_tprintf(TEXT("\n[ERRO] Erro em MapViewOfFile!\n"));
		return;
	}
	ptr_register((TCHAR*)SHM_INFO, 7);

	MAPA* aux = NULL;
	CopyMemory(&aux, shared, sizeof(shared));
	for (int i = 0; tamanhoMapa == -1; i++)
		if (shared[i].caracter == '\n')
			tamanhoMapa = i;
	dados->mapa = (MAPA*)malloc(sizeof(MAPA) * tamanhoMapa * tamanhoMapa);
	CopyMemory(dados->mapa, &aux, sizeof(dados->mapa));

	CopyMemory(&info, sharedInfo, sizeof(INFO));
	return;
}

DWORD WINAPI ThreadAtualizaMapa(LPVOID param) {
	DADOS* dados = ((DADOS*)param);

	atualizaMap = CreateEvent(NULL, TRUE, FALSE, EVENT_ATUALIZAMAP);
	if (atualizaMap == NULL) {
		return 0;
	}
	SetEvent(atualizaMap);
	Sleep(500);
	ResetEvent(atualizaMap);

	while (1) {
		WaitForSingleObject(atualizaMap, INFINITE);

		if (dados->terminar)
			return 0;

		CopyMemory(dados->mapa, shared, sizeof(dados->mapa));

		CopyMemory(&info, sharedInfo, sizeof(INFO));

		Sleep(3000);
	}
	Sleep(500);

	ExitThread(0);
}