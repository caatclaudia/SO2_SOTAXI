#pragma once
#include <windows.h>
#include <tchar.h>
#include <fcntl.h>
#include <io.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define NQ_INICIAL 10
#define TAM 200
#define WAITTIMEOUT 1000

#define SHM_NAME TEXT("EspacoTaxis")
#define NOME_MUTEX TEXT("MutexTaxi")
#define EVENT_NOVOT TEXT("NovoTaxi")
#define EVENT_SAIUT TEXT("SaiuTaxi")
#define EVENT_MOVIMENTO TEXT("MovimentoTaxi")
#define EVENT_RESPOSTA TEXT("RespostaDoAdmin")
#define EVENT_SAIUA TEXT("SaiuAdmin")

typedef struct {
	TCHAR matricula[7];
	unsigned int X, Y, Xfinal, Yfinal;
	int disponivel;
	TCHAR idPassageiro[TAM];
	float velocidade;
	int autoResposta;
	int interessado;
	int terminar;
	int id_mapa;
} TAXI;
HANDLE hMutex;

HANDLE EspTaxis;	//FileMapping
TAXI* shared;
HANDLE novoTaxi;
HANDLE saiuTaxi;
HANDLE movimentoTaxi;
HANDLE respostaAdmin;
HANDLE saiuAdmin;

unsigned int NQ = NQ_INICIAL;

void ajuda();
void inicializaTaxi(TAXI* taxi);
DWORD WINAPI ThreadComandos(LPVOID param);
DWORD WINAPI ThreadMovimentaTaxi(LPVOID param);
DWORD WINAPI ThreadSaiuAdmin(LPVOID param);
DWORD WINAPI ThreadRespostaTransporte(LPVOID param);