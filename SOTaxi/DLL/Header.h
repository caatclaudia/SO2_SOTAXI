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
HANDLE hMutex;

typedef struct {
	TAXI* taxi;

	HANDLE novoTaxi;
	HANDLE saiuTaxi;
	HANDLE movimentoTaxi;
	HANDLE respostaAdmin;
	HANDLE saiuAdmin;

	HANDLE EspTaxis;	//FileMapping
	TAXI* shared;
} DADOS;

//Esta macro é definida pelo sistema caso estejamos na DLL (<DLL_IMP>_EXPORTS definida)
//ou na app (<DLL_IMP>_EXPORTS não definida) onde DLL_IMP é o nome deste projeto
#ifdef _WINDLL
#define DLL_IMP_API __declspec(dllexport)//export
#else
#define DLL_IMP_API __declspec(dllimport)//import
#endif
#ifdef __cplusplus
extern "C"
{
	//Funções a serem exportadas/
	extern DLL_IMP_API void comunicacaoParaCentral(DADOS *dados);
	extern DLL_IMP_API void avisaNovoTaxi(DADOS *dados);
	extern DLL_IMP_API void avisaTaxiSaiu(DADOS *dados);
	extern DLL_IMP_API void avisaMovimentoTaxi(DADOS *dados);
}
#endif