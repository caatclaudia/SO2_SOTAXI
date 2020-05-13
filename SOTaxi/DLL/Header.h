#pragma once
#include <windows.h>
#include <tchar.h>
#include <fcntl.h>
#include <io.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define TAM 200

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

typedef struct {
	char caracter;
} MAPA;

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

#ifdef DLL_EXPORTS
#define DLL_API __declspec(dllexport)//export
#else
#define DLL_API __declspec(dllimport)//import
#endif

//Funções a serem exportadas/
extern DLL_API void comunicacaoParaCentral(DADOS* dados);
extern DLL_API void avisaNovoTaxi(DADOS* dados);
extern DLL_API void avisaTaxiSaiu(DADOS* dados);
extern DLL_API void avisaMovimentoTaxi(DADOS* dados);
