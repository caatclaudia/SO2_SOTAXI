#pragma once
#include <windows.h>
#include <tchar.h>
#include <fcntl.h>
#include <io.h>
#include <stdio.h>
#include <stdlib.h>

#define TAM 200
#define MAX_PASS 20

#define SEMAPHORE_NAME TEXT("SEMAPHORE_PASS")
HANDLE Semaphore;

typedef struct {
	TCHAR id[TAM];
	unsigned int X, Y, Xfinal, Yfinal;
	int movimento;
	int terminar;
	char id_mapa;
} PASSAGEIRO;

typedef struct {
	int nPassageiros;
	PASSAGEIRO passageiros[MAX_PASS];
	int terminar;
} DADOS;

void novoPassageiro(DADOS* dados);
DWORD WINAPI ThreadComandos(LPVOID param);
DWORD WINAPI ThreadMovimentaPassageiro(LPVOID param);
DWORD WINAPI ThreadRespostaTransporte(LPVOID param);