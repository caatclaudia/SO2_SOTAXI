#pragma once
#include <windows.h>
#include <tchar.h>
#include <fcntl.h>
#include <io.h>
#include <stdio.h>
#include <stdlib.h>

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

//1 INSTANCIA APENAS
#define SEMAPHORE_NAME TEXT("SEMAPHORE_ADMIN")
HANDLE Semaphore;

#define NOME_MUTEX_MAPA TEXT("MutexMapa")
typedef struct {
	char caracter;
} MAPA;

typedef struct {
	TCHAR id[TAM];
	unsigned int X, Y, Xfinal, Yfinal;
	int movimento;
	int terminar;
	char id_mapa;
} PASSAGEIRO;

#define NOME_MUTEX_TAXI TEXT("MutexTaxi")
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

//SEMAFOROS
#define EVENT_NOVOT TEXT("NovoTaxi")
#define EVENT_SAIUT TEXT("SaiuTaxi")
#define EVENT_MOVIMENTO TEXT("MovimentoTaxi")
#define EVENT_RESPOSTA TEXT("RespostaDoAdmin")
#define EVENT_SAIUA TEXT("SaiuAdmin")

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
	HANDLE saiuAdmin;
} DADOS;

void ajuda();
void listarTaxis(DADOS* dados);
void listarPassageiros(DADOS* dados);
void leMapa(DADOS* dados);
boolean adicionaTaxi(DADOS* dados, TAXI novo);
boolean removeTaxi(DADOS* dados, TAXI novo);
boolean adicionaPassageiro(DADOS* dados, PASSAGEIRO novo);
boolean removePassageiro(DADOS* dados, PASSAGEIRO novo);
void eliminaIdMapa(DADOS* dados, char id);
DWORD WINAPI ThreadComandos(LPVOID param);
DWORD WINAPI ThreadNovoTaxi(LPVOID param);
DWORD WINAPI ThreadSaiuTaxi(LPVOID param);
DWORD WINAPI ThreadMovimento(LPVOID param);
DWORD WINAPI ThreadNovoPassageiro(LPVOID param);