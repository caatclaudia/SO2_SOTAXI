#pragma once

#include "resource.h"

#include <Windows.h>
#include <tchar.h>
#include <math.h>
#include <stdio.h>
#include <fcntl.h>
#include <conio.h>
#include <io.h>

#define PATH_DLL TEXT("..\\SO2_TP_DLL_32.dll")

#define TAM 50
#define WAITTIMEOUT 1000
int tamanhoMapa = -1;

#define NOME_MUTEXMAPA TEXT("MutexMapa")
typedef struct {
	char caracter;
} MAPA;
HANDLE hMutex;

typedef struct {
	MAPA* mapa;
	int terminar;
} DADOS;

#define SHM_NAME TEXT("EspacoMapa")
HANDLE EspMapa;	//FileMapping
MAPA* shared;

#define MAXTAXIS 10
#define MAXPASS 10
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

#define NOME_MUTEX_TAXI TEXT("MutexTaxi")
typedef struct {
	TCHAR matricula[7];
	unsigned int X, Y, Xfinal, Yfinal;
	int disponivel;
	int autoResposta;
	int interessado;
	int terminar;
	int id_mapa;
	float velocidade;
} TAXI;

typedef struct {
	TAXI taxis[MAXTAXIS];
	int ntaxis;
	PASSAGEIRO passageiros[MAXPASS];
	int npassageiros;
} INFO;

#define SHM_INFO TEXT("EspacoInfo")
HANDLE EspInfo;	//FileMapping
INFO* sharedInfo = NULL;

//SEMAFOROS
#define EVENT_ATUALIZAMAP TEXT("AtualizaMapa")
HANDLE atualizaMap;

void recebeMapa(DADOS* dados);
DWORD WINAPI ThreadAtualizaMapa(LPVOID param);