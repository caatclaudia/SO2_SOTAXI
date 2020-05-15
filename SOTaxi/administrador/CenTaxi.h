#pragma once
#include <windows.h>
#include <tchar.h>
#include <fcntl.h>
#include <io.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

void(*ptr_register)(TCHAR*, int);
void(*ptr_log)(TCHAR*);

#define TAM 50
#define MAXTAXIS 10
#define MAXPASS 10
#define TempoManifestacoes 5
#define WAITTIMEOUT 1000
#define PATH TEXT("..\\mapa.txt")
#define PATH_DLL TEXT("..\\SO2_TP_DLL_32.dll")

int MaxPass = MAXPASS;
int MaxTaxi = MAXTAXIS;

int id_mapa_taxi = 1;
char id_mapa_pass = 'A';

int tamanhoMapa = -1;

#define EVENT_TRANSPORTE TEXT("Transporte")
HANDLE transporte;

//1 INSTANCIA APENAS
#define SEMAPHORE_NAME TEXT("SEMAPHORE_ADMIN")
HANDLE Semaphore;

#define NOME_MUTEX_MAPA TEXT("MutexMapa")
typedef struct {
	char caracter;
} MAPA;

typedef struct {
	unsigned int X, Y, Xfinal, Yfinal;
	int movimento;
	int terminar;
	char id_mapa;
	TCHAR id[TAM];
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

#define MAX_PASS 5
#define BUFFER_CIRCULAR TEXT("BufferCircular")
#define SEMAPHORE_MUTEX TEXT("SEM_MUTEX")
#define SEMAPHORE_ITENS TEXT("SEM_ITENS")
#define SEMAPHORE_VAZIOS TEXT("SEM_VAZIOS")
HANDLE sem_mutex, sem_itens, sem_vazios;

HANDLE hTimer;
int acabouTempo = 0;

typedef struct {
	PASSAGEIRO Passageiros[MAX_PASS];
	int NextIn = 0, NextOut = 0;
} BUFFER;

HANDLE hMemoria;
BUFFER* BufferMemoria;

//SEMAFOROS
#define EVENT_NOVOT TEXT("NovoTaxi")
#define EVENT_SAIUT TEXT("SaiuTaxi")
#define EVENT_MOVIMENTO TEXT("MovimentoTaxi")
#define EVENT_RESPOSTA TEXT("RespostaDoAdmin")
#define EVENT_SAIUA TEXT("InfoAdmin")

#define EVENT_ATUALIZAMAP TEXT("AtualizaMapa")

#define SHM_TAXI TEXT("EspacoTaxis")
#define SHM_MAPA TEXT("EspacoMapa")

#define NOME_MUTEX_DADOS TEXT("MutexDados")
typedef struct {
	int nTaxis;
	TAXI taxis[MAXTAXIS];
	HANDLE EspTaxis;	//FileMapping
	HANDLE hMutexDados;
	TAXI* sharedTaxi;
	HANDLE novoTaxi;
	HANDLE saiuTaxi;
	HANDLE movimentoTaxi;
	HANDLE respostaAdmin;
	int aceitacaoT;
	int esperaManifestacoes;

	int nPassageiros;
	PASSAGEIRO passageiros[MAXPASS];

	MAPA* mapa;
	HANDLE hFile;
	HANDLE EspMapa;	//FileMapping
	MAPA* sharedMapa = NULL;
	HANDLE atualizaMap;

	int terminar;
	HANDLE infoAdmin;
} DADOS;

void inicializaBuffer();
void inicializaVariaveis();
void ajuda();
void listarTaxis(DADOS* dados);
void listarPassageiros(DADOS* dados);
void leMapa(DADOS* dados);
boolean adicionaTaxi(DADOS* dados, TAXI novo);
boolean removeTaxi(DADOS* dados, TAXI novo);
boolean adicionaPassageiro(DADOS* dados, PASSAGEIRO novo);
boolean removePassageiro(DADOS* dados, PASSAGEIRO novo);
void eliminaIdMapa(DADOS* dados, char id);
void expulsarTaxi(DADOS* dados, TCHAR* matr);
void transporteAceite(DADOS* dados);
void enviaTaxi(DADOS* dados, TAXI* taxi);
void newPassageiro(DADOS* dados);
DWORD WINAPI ThreadTempoTransporte(LPVOID param);
DWORD WINAPI ThreadComandos(LPVOID param);
DWORD WINAPI ThreadNovoTaxi(LPVOID param);
DWORD WINAPI ThreadSaiuTaxi(LPVOID param);
DWORD WINAPI ThreadMovimento(LPVOID param);
DWORD WINAPI ThreadNovoPassageiro(LPVOID param);