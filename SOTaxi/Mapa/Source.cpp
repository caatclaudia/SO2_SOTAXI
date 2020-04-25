#include <Windows.h>
#include <tchar.h>
#include <math.h>
#include <stdio.h>
#include <fcntl.h>
#include <conio.h>
#include <io.h>

#define TAM 200
#define EVENT_ATUALIZAMAP TEXT("AtualizaMap")
#define NOME_MUTEXMAPA TEXT("MutexMapa")

typedef struct {
	TCHAR caracter;
} MAPA;

typedef struct {
	MAPA mapa[TAM][TAM];
	TCHAR* pView;
	int terminar;
} DADOS;

#define PATH TEXT("..\\mapa.txt")

HANDLE hFile, map;
HANDLE hMutex;

DWORD WINAPI ThreadPrincipal(LPVOID param);
DWORD WINAPI ThreadSair(LPVOID param);
void leFicheiro(DADOS* dados);
void mostraMapa(DADOS* dados);

int _tmain(int argc, TCHAR argv[]) {
	HANDLE hThreadSair, hThreadPrincipal;
	DADOS dados;
	TCHAR nome[100] = PATH;
	int x = 0, y = 0;
	dados.terminar = 0;

#ifdef UNICODE
	_setmode(_fileno(stdin), _O_WTEXT);
	_setmode(_fileno(stdout), _O_WTEXT);
#endif

	hMutex = CreateMutex(NULL, FALSE, NOME_MUTEXMAPA);
	if (hMutex == NULL) {
		_tprintf(TEXT("\n[ERRO] Erro ao criar Mutex!\n"));
		return -1;
	}
	WaitForSingleObject(hMutex, INFINITE);

	hFile = CreateFile(PATH, GENERIC_WRITE | GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	if (hFile == INVALID_HANDLE_VALUE)
	{
		_tprintf(TEXT("\n[ERRO] Erro ao Abrir Ficheiro!\n"));
		return -1;
	}

	map = CreateFileMapping(hFile, NULL, PAGE_READWRITE, 0, 50 * 52, NULL);

	if (map == NULL)
	{
		_tprintf(TEXT("\n[ERRO] Erro ao criar FileMapping!\n"));
		CloseHandle(hFile);
		return -1;
	}

	leFicheiro(&dados);
	ReleaseMutex(hMutex);

	hThreadPrincipal = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ThreadPrincipal, (LPVOID)&dados, 0, NULL); //CREATE_SUSPENDED para nao comecar logo
	if (hThreadPrincipal == NULL) {
		_tprintf(TEXT("\n[ERRO] Erro ao lançar Thread!\n"));
		return 0;
	}

	hThreadSair = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ThreadSair, (LPVOID)&dados, 0, NULL); //CREATE_SUSPENDED para nao comecar logo
	if (hThreadSair == NULL) {
		_tprintf(TEXT("\n[ERRO] Erro ao lançar Thread!\n"));
		return 0;
	}

	HANDLE ghEvents[2];
	ghEvents[0] = hThreadPrincipal;
	ghEvents[1] = hThreadSair;
	WaitForMultipleObjects(2, ghEvents, TRUE, INFINITE);
	

	_tprintf(TEXT("\nPrima uma tecla...\n"));
	_gettch();

	UnmapViewOfFile(dados.pView);
	CloseHandle(hFile);
	CloseHandle(map);

	return 0;
}

void leFicheiro(DADOS *dados) {
	dados->pView = (TCHAR*)MapViewOfFile(map, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, 50 * 52);
	if (dados->pView == NULL)
	{
		_tprintf(TEXT("\n[ERRO] Erro em MapViewOfFile!\n"));
		CloseHandle(hFile);
		CloseHandle(map);
		return;
	}

	TCHAR aux;
	int x=0, y = 0;
	for (int i = 0; i < 50 * 52; i++) {
		aux = dados->pView[i];
		_tprintf(TEXT("%c"), aux);
		if (aux == '\n') {
			x = 0;
			y++;
		}
		else {
			x++;
		}
		if (aux != '\n') {
			dados->mapa[y][x].caracter = aux;
		}
	}

	return;
}

void mostraMapa(DADOS* dados){
	for (int x = 0; x < 50; x++) {
		for (int y = 0; y < 52; y++)
			_tprintf(TEXT("%c"), dados->mapa[x][y].caracter);
		_tprintf(TEXT("\n"));
	}

	return;
}

DWORD WINAPI ThreadPrincipal(LPVOID param) {
	DADOS* dados = ((DADOS*)param);

	while (!dados->terminar) {
		WaitForSingleObject(hMutex, INFINITE);
		mostraMapa(dados);
		system("cls");
		ReleaseMutex(hMutex);
		
		Sleep(3000);
	}

	Sleep(500);

	ExitThread(0);
}

DWORD WINAPI ThreadSair(LPVOID param) {
	TCHAR op[TAM];
	DADOS* dados = ((DADOS*)param);

	do {
		_fgetts(op, TAM, stdin);
		op[_tcslen(op) - 1] = '\0';
		WaitForSingleObject(hMutex, INFINITE);
	} while (_tcscmp(op, TEXT("fim")));

	dados->terminar = 1;
	ReleaseMutex(hMutex);

	Sleep(1000);

	ExitThread(0);
}