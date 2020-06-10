#pragma once
#include "..\\DLL\Header.h"

void(*ptr_register)(TCHAR*, int);

#define NOME_MUTEX TEXT("MutexTaxi")
HANDLE hMutex;
HANDLE pipeT;

#define SHM_NAME TEXT("EspacoTaxis")
#define SHM_NAME_MAP TEXT("EspacoMapa")

//SEMAFOROS
#define EVENT_NOVOT TEXT("NovoTaxi")
#define EVENT_SAIUT TEXT("SaiuTaxi")
#define EVENT_MOVIMENTO TEXT("MovimentoTaxi")
#define EVENT_RESPOSTA TEXT("RespostaDoAdmin")
#define EVENT_INFOA TEXT("InfoAdmin")
#define EVENT_SAIUA TEXT("SaiuAdmin")

#define PATH_DLL TEXT("..\\SO2_TP_DLL_32.dll")
#define PATH_MY_DLL TEXT("..\\Debug\\DLL.dll")

#define NQ_INICIAL 10
#define WAITTIMEOUT 1000
unsigned int NQ = NQ_INICIAL;

int tamanhoMapa = -1;
int irParaX = -1, irParaY = -1;

void ajuda();
int calculaDistancia(int inicioX, int inicioY, int fimX, int fimY);
void inicializaBuffer();
void inicializaTaxi(DADOS* dados);
void leMapa(DADOS* dados);
DWORD WINAPI ThreadComandos(LPVOID param);
DWORD WINAPI ThreadMovimentaTaxi(LPVOID param);
DWORD WINAPI ThreadInfoAdmin(LPVOID param);
DWORD WINAPI ThreadVerificaAdmin(LPVOID param);
DWORD WINAPI ThreadRespostaTransporte(LPVOID param);
DWORD WINAPI ThreadTransporte(LPVOID param);