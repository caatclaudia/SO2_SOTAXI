#pragma once
#include <Windows.h>
#include <tchar.h>
#include <math.h>
#include <stdio.h>
#include <fcntl.h>
#include <conio.h>
#include <io.h>

#define TAM 50
#define WAITTIMEOUT 1000
#define SHM_NAME TEXT("EspacoMapa")
#define EVENT_ATUALIZAMAP TEXT("AtualizaMapa")
#define NOME_MUTEXMAPA TEXT("MutexMapa")

typedef struct {
	char caracter;
} MAPA;

int tamanhoMapa = -1;

typedef struct {
	MAPA* mapa;
	int terminar;
} DADOS;

#define PATH TEXT("..\\mapa.txt")

HANDLE EspMapa;	//FileMapping
MAPA* shared;
HANDLE atualizaMap;
HANDLE hMutex;

void inicializaVariaveis();
void recebeMapa(DADOS* dados);
void mostraMapa(DADOS* dados);
DWORD WINAPI ThreadAtualizaMapa(LPVOID param);
DWORD WINAPI ThreadSair(LPVOID param);