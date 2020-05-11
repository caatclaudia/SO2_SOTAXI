#pragma once
#include <windows.h>
#include <tchar.h>
#include <fcntl.h>
#include <io.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

typedef struct {
	char caracter;
} MAPA;

#define TAM 200

#define NOME_MUTEX TEXT("MutexTaxi")
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

#define SHM_NAME TEXT("EspacoTaxis")
#define SHM_NAME_MAP TEXT("EspacoMapa")

//SEMAFOROS
#define EVENT_NOVOT TEXT("NovoTaxi")
#define EVENT_SAIUT TEXT("SaiuTaxi")
#define EVENT_MOVIMENTO TEXT("MovimentoTaxi")
#define EVENT_RESPOSTA TEXT("RespostaDoAdmin")
#define EVENT_SAIUA TEXT("SaiuAdmin")

typedef struct {
	TAXI* taxi;

	HANDLE novoTaxi;
	HANDLE saiuTaxi;
	HANDLE movimentoTaxi;
	HANDLE respostaAdmin;
	HANDLE saiuAdmin;

	HANDLE EspTaxis;	//FileMapping
	TAXI* shared;

	MAPA* mapa;
	HANDLE EspMapa;	//FileMapping
	MAPA* sharedMap;
} DADOS;

#define PATH_DLL TEXT("..\\SO2_TP_DLL_32.dll")
#define PATH_MY_DLL TEXT("..\\Debug\\DLL.dll")

#define NQ_INICIAL 10
#define WAITTIMEOUT 1000
unsigned int NQ = NQ_INICIAL;

void ajuda();
int calculaDistancia(int inicioX, int inicioY, int fimX, int fimY);
void inicializaTaxi(DADOS* dados);
void leMapa(DADOS* dados);
DWORD WINAPI ThreadComandos(LPVOID param);
DWORD WINAPI ThreadMovimentaTaxi(LPVOID param);
DWORD WINAPI ThreadSaiuAdmin(LPVOID param);
DWORD WINAPI ThreadRespostaTransporte(LPVOID param);