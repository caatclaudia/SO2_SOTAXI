#pragma once
#include <windows.h>
#include <tchar.h>
#include <fcntl.h>
#include <io.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define TAM 200
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
	char caracter;
} MAPA;

typedef struct {
	TAXI* taxi;

	HANDLE novoTaxi;
	HANDLE saiuTaxi;
	HANDLE movimentoTaxi;
	HANDLE respostaAdmin;
	HANDLE infoAdmin;
	HANDLE saiuAdmin;

	HANDLE EspTaxis;	//FileMapping
	TAXI* shared;

	MAPA* mapa;
	HANDLE EspMapa;	//FileMapping
	MAPA* sharedMap;
} DADOS;

#define MAX_PASS 5
#define BUFFER_CIRCULAR TEXT("BufferCircular")
#define SEMAPHORE_MUTEX TEXT("SEM_MUTEX")
#define SEMAPHORE_ITENS TEXT("SEM_ITENS")
#define SEMAPHORE_VAZIOS TEXT("SEM_VAZIOS")
HANDLE sem_mutex, sem_itens, sem_vazios;

typedef struct {
	PASSAGEIRO Passageiros[MAX_PASS];
	int NextIn = 0, NextOut = 0;
} BUFFER;

HANDLE hMemoria;
BUFFER* BufferMemoria;

#ifdef DLL_EXPORTS
#define DLL_API __declspec(dllexport)//export
#else
#define DLL_API __declspec(dllimport)//import
#endif

//Funções a serem exportadas/
extern DLL_API void comunicacaoParaCentral(DADOS* dados);
extern DLL_API void avisaNovoTaxi(DADOS* dados);
extern DLL_API void recebeInfo(DADOS* dados);
extern DLL_API void avisaTaxiSaiu(DADOS* dados);
extern DLL_API void avisaMovimentoTaxi(DADOS* dados);
