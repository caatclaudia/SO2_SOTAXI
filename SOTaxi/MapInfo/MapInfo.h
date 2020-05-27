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

//SEMAFOROS
#define EVENT_ATUALIZAMAP TEXT("AtualizaMapa")
HANDLE atualizaMap;

void recebeMapa(DADOS* dados);
DWORD WINAPI ThreadAtualizaMapa(LPVOID param);