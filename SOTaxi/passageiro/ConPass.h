#pragma once
#include <windows.h>
#include <tchar.h>
#include <fcntl.h>
#include <io.h>
#include <stdio.h>
#include <stdlib.h>

#define PATH_DLL TEXT("..\\SO2_TP_DLL_32.dll")

#define TAM 200
#define MAX_PASS 20
#define TAM_ID 10

typedef struct {
	unsigned int X, Y, Xfinal, Yfinal;
	int movimento;
	int terminar;
	char id_mapa;
	TCHAR matriculaTaxi[7];
	int tempoEspera;
	TCHAR id[TAM_ID];
} PASSAGEIRO;

typedef struct {
	int nPassageiros;
	PASSAGEIRO passageiros[MAX_PASS];
	int terminar;
} DADOS;

//SEMAFOROS
#define SEMAPHORE_NAME TEXT("SEMAPHORE_PASS")
HANDLE Semaphore;

#define PIPE_NAME TEXT("\\\\.\\pipe\\comunica")
HANDLE hPipe;
#define EVENT_NOVOP TEXT("NovoPass")
HANDLE novoPass;

void novoPassageiro(DADOS* dados);
DWORD WINAPI ThreadComandos(LPVOID param);
DWORD WINAPI ThreadMovimentaPassageiro(LPVOID param);
DWORD WINAPI ThreadRespostaTransporte(LPVOID param);