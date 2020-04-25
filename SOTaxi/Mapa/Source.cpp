#include <Windows.h>
#include <tchar.h>
#include <math.h>
#include <stdio.h>
#include <fcntl.h>
#include <conio.h>
#include <io.h>

typedef struct {
	TCHAR caracter;
} MAPA;

#define PATH TEXT("..\\mapa.txt")

int _tmain(int argc, TCHAR argv[]) {
	HANDLE hFile, map;
	MAPA* pView;
	TCHAR aux;
	TCHAR nome[100] = PATH;

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

	pView = (MAPA*)MapViewOfFile(map, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, 50 * 52);
	if (pView == NULL)
	{
		_tprintf(TEXT("\n[ERRO] Erro em MapViewOfFile!\n"));
		CloseHandle(hFile);
		CloseHandle(map);
		return -1;
	}

	while (1) {
		for (int i = 0; i < 50 * 52; i++) {
			aux = pView[i].caracter;
			_tprintf(TEXT("%c"), aux);
		}
		system("cls");
		Sleep(1000);
	}

	_tprintf(TEXT("\nPrima uma tecla...\n"));
	_gettch();

	UnmapViewOfFile(pView);
	CloseHandle(hFile);
	CloseHandle(map);

	return 0;
}