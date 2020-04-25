#include <Windows.h>
#include <tchar.h>
#include <math.h>
#include <stdio.h>
#include <fcntl.h>
#include <conio.h>
#include <io.h>

#define TAM 200

typedef struct {
	TCHAR caracter;
} MAPA;

#define PATH TEXT("..\\mapa.txt")

HANDLE hFile, map;
TCHAR* pView;
MAPA mapa[TAM][TAM];

void leFicheiro();
void mostraMapa();

int _tmain(int argc, TCHAR argv[]) {
	
	TCHAR nome[100] = PATH;
	int x = 0, y = 0;

#ifdef UNICODE
	_setmode(_fileno(stdin), _O_WTEXT);
	_setmode(_fileno(stdout), _O_WTEXT);
#endif

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

	leFicheiro();

	while (1) {
		mostraMapa();
		
		system("cls");
		Sleep(3000);
	}

	_tprintf(TEXT("\nPrima uma tecla...\n"));
	_gettch();

	UnmapViewOfFile(pView);
	CloseHandle(hFile);
	CloseHandle(map);

	return 0;
}

void leFicheiro() {
	pView = (TCHAR*)MapViewOfFile(map, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, 50 * 52);
	if (pView == NULL)
	{
		_tprintf(TEXT("\n[ERRO] Erro em MapViewOfFile!\n"));
		CloseHandle(hFile);
		CloseHandle(map);
		return;
	}

	TCHAR aux;
	int x=0, y = 0;
	for (int i = 0; i < 50 * 52; i++) {
		aux = pView[i];
		_tprintf(TEXT("%c"), aux);
		if (aux == '\n') {
			x = 0;
			y++;
		}
		else {
			x++;
		}
		if (aux != '\n') {
			mapa[y][x].caracter = aux;
		}
	}

	return;
}

void mostraMapa(){
	for (int x = 0; x < 50; x++) {
		for (int y = 0; y < 52; y++)
			_tprintf(TEXT("%c"), mapa[x][y].caracter);
		_tprintf(TEXT("\n"));
	}

	return;
}