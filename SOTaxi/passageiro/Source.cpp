#include <windows.h>
#include <tchar.h>
#include <fcntl.h>
#include <io.h>
#include <stdio.h>
#include <stdlib.h>

#define TAM 200
#define MAX_PASS 20
#define WAITTIMEOUT 2000

//ConPass
//1 instancia
//Passageiros tem um id(assumir que já é unico)
//Indica à central a existência de novo passageiro que está em X, Y e pertende ir para X, Y
//Recebe da central : Táxi atribuido ao passageiro X; Táxi recolheu passageiro X; Táxi entregou o passageiro X

typedef struct {
	TCHAR id[TAM];
	unsigned int X, Y, Xfinal, Yfinal;
	int movimento;
	int terminar;
} PASSAGEIRO;

typedef struct {
	int nPassageiros;
	PASSAGEIRO passageiros[MAX_PASS];
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

#ifdef UNICODE
	_setmode(_fileno(stdin), _O_WTEXT);
	_setmode(_fileno(stdout), _O_WTEXT);
#endif

	hThreadComandos = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ThreadComandos, (LPVOID)&dados, 0, NULL); //CREATE_SUSPENDED para nao comecar logo
	if (hThreadComandos == NULL) {
		_tprintf(TEXT("\nErro ao lançar Thread!\n"));
		return 0;
	}
	//hThreadMovimentaPassageiro = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ThreadMovimentaPassageiro, (LPVOID)&dados, 0, NULL); //CREATE_SUSPENDED para nao comecar logo
	//if (hThreadMovimentaPassageiro == NULL) {
	//	_tprintf(TEXT("\nErro ao lançar Thread!\n"));
	//	return 0;
	//}
	//hThreadRespostaTransporte = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ThreadRespostaTransporte, (LPVOID)&dados, 0, NULL); //CREATE_SUSPENDED para nao comecar logo
	//if (hThreadRespostaTransporte == NULL) {
	//	_tprintf(TEXT("\nErro ao lançar Thread!\n"));
	//	return 0;
	//}

	HANDLE ghEvents[1];
	ghEvents[0] = hThreadComandos;
	/*ghEvents[1] = hThreadMovimentaPassageiro;
	ghEvents[2] = hThreadRespostaTransporte;*/
	DWORD dwResultEspera;
	do {
		dwResultEspera = WaitForMultipleObjects(1, ghEvents, TRUE, WAITTIMEOUT);
		if (dwResultEspera == WAITTIMEOUT) {
			for (int i = 0; i < dados.nPassageiros; i++)
				dados.passageiros[i].terminar = 1;
			_tprintf(TEXT("As Threads vao parar!\n"));
			break;
		}
	} while (1);

	_gettch();

	return 0;
}

void novoPassageiro(DADOS* dados) {

	_tprintf(_T("\n Id do Passageiro: "));
	_fgetts(dados->passageiros[dados->nPassageiros].id, TAM, stdin);
	dados->passageiros[dados->nPassageiros].id[_tcslen(dados->passageiros[dados->nPassageiros].id) - 1] = '\0';

	_tprintf(_T("\n Localizacao do Passageiro (X Y) : "));
	_tscanf_s(_T("%d"), &dados->passageiros[dados->nPassageiros].X);
	_tscanf_s(_T("%d"), &dados->passageiros[dados->nPassageiros].Y);

	_tprintf(_T("\n Local de destino do Passageiro (X Y) : "));
	_tscanf_s(_T("%d"), &dados->passageiros[dados->nPassageiros].Xfinal);
	_tscanf_s(_T("%d"), &dados->passageiros[dados->nPassageiros].Yfinal);
	dados->passageiros[dados->nPassageiros].movimento = 0;
	dados->passageiros[dados->nPassageiros].terminar = 0;

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