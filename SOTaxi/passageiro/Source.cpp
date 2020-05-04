#include <windows.h>
#include <tchar.h>
#include <fcntl.h>
#include <io.h>
#include <stdio.h>
#include <stdlib.h>

#define TAM 200
#define MAX_PASS 20

#define SEMAPHORE_NAME TEXT("SEMAPHORE_PASS")
HANDLE Semaphore;

typedef struct {
	TCHAR id[TAM];
	unsigned int X, Y, Xfinal, Yfinal;
	int movimento;
	int terminar;
	char id_mapa;
} PASSAGEIRO;

typedef struct {
	int nPassageiros;
	PASSAGEIRO passageiros[MAX_PASS];
	int terminar;
} DADOS;

void novoPassageiro(DADOS* dados);
DWORD WINAPI ThreadComandos(LPVOID param);
DWORD WINAPI ThreadMovimentaPassageiro(LPVOID param);
DWORD WINAPI ThreadRespostaTransporte(LPVOID param);

int _tmain() {
	HANDLE hThreadComandos, hThreadMovimentaPassageiro, hThreadRespostaTransporte;
	TCHAR op[TAM];
	DADOS dados;
	dados.nPassageiros = 0;
	dados.terminar = 0;

#ifdef UNICODE
	_setmode(_fileno(stdin), _O_WTEXT);
	_setmode(_fileno(stdout), _O_WTEXT);
#endif

	Semaphore = CreateSemaphore(NULL, 1, 1, SEMAPHORE_NAME);
	if (Semaphore == NULL) {
		_tprintf(TEXT("CreateSemaphore failed.\n"));
		return FALSE;
	}
	_tprintf(TEXT("\nAinda não tenho autorização para entrar! Esperar...\n"));
	WaitForSingleObject(Semaphore, INFINITE);
	_tprintf(TEXT("\nEntrei!\n"));

	hThreadComandos = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ThreadComandos, (LPVOID)&dados, 0, NULL);
	if (hThreadComandos == NULL) {
		_tprintf(TEXT("\n[ERRO] Erro ao lançar Thread!\n"));
		return 0;
	}
	//hThreadMovimentaPassageiro = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ThreadMovimentaPassageiro, (LPVOID)&dados, 0, NULL);
	//if (hThreadMovimentaPassageiro == NULL) {
	//	_tprintf(TEXT("\nErro ao lançar Thread!\n"));
	//	return 0;
	//}
	//hThreadRespostaTransporte = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ThreadRespostaTransporte, (LPVOID)&dados, 0, NULL);
	//if (hThreadRespostaTransporte == NULL) {
	//	_tprintf(TEXT("\nErro ao lançar Thread!\n"));
	//	return 0;
	//}

	HANDLE ghEvents[1];
	ghEvents[0] = hThreadComandos;
	/*ghEvents[1] = hThreadMovimentaPassageiro;
	ghEvents[2] = hThreadRespostaTransporte;*/
	WaitForMultipleObjects(1, ghEvents, TRUE, INFINITE);

	_tprintf(TEXT("Passageiros vão sair!\n"));
	_tprintf(TEXT("Prima uma tecla...\n"));
	_gettch();

	ReleaseSemaphore(Semaphore, 1, NULL);

	CloseHandle(Semaphore);

	return 0;
}

void novoPassageiro(DADOS* dados) {

	_tprintf(_T("\n[NOVO] Id do Passageiro: "));
	_fgetts(dados->passageiros[dados->nPassageiros].id, TAM, stdin);
	dados->passageiros[dados->nPassageiros].id[_tcslen(dados->passageiros[dados->nPassageiros].id) - 1] = '\0';

	_tprintf(_T("\n[NOVO]  Localizacao do Passageiro (X Y) : "));
	_tscanf_s(_T("%d"), &dados->passageiros[dados->nPassageiros].X);
	_tscanf_s(_T("%d"), &dados->passageiros[dados->nPassageiros].Y);

	_tprintf(_T("\n[NOVO]  Local de destino do Passageiro (X Y) : "));
	_tscanf_s(_T("%d"), &dados->passageiros[dados->nPassageiros].Xfinal);
	_tscanf_s(_T("%d"), &dados->passageiros[dados->nPassageiros].Yfinal);
	dados->passageiros[dados->nPassageiros].movimento = 0;
	dados->passageiros[dados->nPassageiros].terminar = 0;
	dados->passageiros[dados->nPassageiros].id_mapa = TEXT('.');

	//VAI AO ADMIN VER SE PODE CRIAR

	return;
}

DWORD WINAPI ThreadComandos(LPVOID param) {
	TCHAR op[TAM];
	DADOS* dados = ((DADOS*)param);

	do {
		_tprintf(_T("\n>>"));
		_fgetts(op, TAM, stdin);
		op[_tcslen(op) - 1] = '\0';
		if (_tcscmp(op, TEXT("novo"))) {		//NOVO PASSAGEIRO
			novoPassageiro(dados);
			dados->nPassageiros++;
		}
	} while (_tcscmp(op, TEXT("fim")));

	for (int i = 0; i < MAX_PASS; i++)
		dados->passageiros[i].terminar = 1;

	dados->terminar = 1;

	ExitThread(0);
}

DWORD WINAPI ThreadMovimentoPassageiro(LPVOID param) {	//ADMIN MANDA PASSAGEIRO
	DADOS* dados = ((DADOS*)param);

	ExitThread(0);
}

DWORD WINAPI ThreadRespostaTransporte(LPVOID param) {	//ADMIN MANDA PASSAGEIRO
	DADOS* dados = ((DADOS*)param);

	ExitThread(0);
}