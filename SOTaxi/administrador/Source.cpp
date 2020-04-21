#include <windows.h>
#include <tchar.h>
#include <fcntl.h>
#include <io.h>
#include <stdio.h>
#include <stdlib.h>

#define TAM 200
#define MAXTAXIS 10
#define MAXPASS 10
#define TempoManifestacoes 5

#define SHM_NAME TEXT("EspacoTaxis")
#define NOME_MUTEX TEXT("MutexTaxi")

//CenTaxi
//1 instancia
//le o mapa e gere-o
//sabe dos taxis, posicoes e estado

typedef struct {
	TCHAR id[TAM];
	unsigned int X, Y, Xfinal, Yfinal;
	int movimento;
	int terminar;
} PASSAGEIRO;

typedef struct {
	TCHAR matricula[6];
	unsigned int X, Y, Xfinal, Yfinal;
	int disponivel;
	TCHAR idPassageiro[TAM];
	float velocidade;
	int autoResposta;
	int interessado;
	int terminar;
	HANDLE hMutex;
} TAXI;


typedef struct {
	int nTaxis;
	TAXI taxis[MAXTAXIS];
	int nPassageiros;
	PASSAGEIRO passageiros[MAXPASS];
	int respostaAuto;
	int esperaManifestacoes;
	int terminar;
	HANDLE EspTaxis;	//FileMapping
} DADOS;


void ajuda();
void listarTaxis(DADOS* dados);
void listarPassageiros(DADOS* dados);
boolean adicionaTaxi(DADOS* dados, TAXI novo);
boolean removeTaxi(DADOS* dados, TAXI novo);
boolean adicionaPassageiro(DADOS* dados, PASSAGEIRO novo);
boolean removePassageiro(DADOS* dados, PASSAGEIRO novo);
DWORD WINAPI ThreadNovoTaxi(LPVOID param);
DWORD WINAPI ThreadNovoPassageiro(LPVOID param);


int _tmain(int argc, LPTSTR argv[]) {
	TCHAR op[TAM];
	DADOS dados;
	dados.nTaxis = 0;
	dados.nPassageiros = 0;
	dados.terminar = 0;
	dados.respostaAuto = 1;
	dados.esperaManifestacoes = TempoManifestacoes;

#ifdef UNICODE
	_setmode(_fileno(stdin), _O_WTEXT);
	_setmode(_fileno(stdout), _O_WTEXT);
#endif

	do {
		_tprintf(_T("\n>>"));
		_fgetts(op, TAM, stdin);
		op[_tcslen(op) - 1] = '\0';
		if (_tcscmp(op, TEXT("expulsar"))) {		//EXPULSAR TAXI

		}
		else if (_tcscmp(op, TEXT("listar"))) {		//LISTAR TAXIS
			listarTaxis(&dados);
		}
		else if (_tcscmp(op, TEXT("aceitacaoT"))) {		//PAUSAR/RECOMECAR ACEITAÇÃO DE TAXIS
			if (dados.respostaAuto)
				dados.respostaAuto = 0;
			else
				dados.respostaAuto = 1;
			//ENVIAR INFORMAÇÃO AOS TAXIS
		}
		else if (_tcscmp(op, TEXT("manifestacoes"))) {		//DEFINIR INTERVALO DE TEMPO DURANTE O QUAL AGUARDA MANIFESTAÇOES DOS TAXIS
			_tprintf(_T("\nIntervalo de tempo durante o qual aguarda manifestações (em segundos): "));
			_tscanf_s(_T("%d"), &dados.esperaManifestacoes);
			if (dados.esperaManifestacoes <= 0)
				dados.esperaManifestacoes = TempoManifestacoes;
		}
		else if (_tcscmp(op, TEXT("ajuda"))) {		//AJUDA NOS COMANDOS
			ajuda();
		}
	} while (_tcscmp(op, TEXT("fim")));

	_tprintf(_T("\nAdministrador vai encerrar!"));
	return 0;
}

void ajuda() {
	_tprintf(_T("\n\n expulsar - EXPULSAR TAXI"));
	_tprintf(_T("\n listar - LISTAR TÁXIS"));
	_tprintf(_T("\n aceitaçãoT - PAUSAR/RECOMECAR ACEITAÇÃO DE TAXIS"));
	_tprintf(_T("\n manifestações - DEFINIR INTERVALO DE TEMPO DURANTE O QUAL AGUARDA MANIFESTAÇOES DOS TAXIS"));
	_tprintf(_T("\n fim - ENCERRAR TODO O SISTEMA"));
	return;
}

void listarTaxis(DADOS* dados) {
	for (int i = 0; i < dados->nTaxis; i++) {
		_tprintf(_T("\nTaxi %d : "), i);
		_tprintf(_T("\n (%d, %d) "), dados->taxis[i].X, dados->taxis[i].Y);
		if (dados->taxis[i].disponivel)
			_tprintf(_T("sem passageiro!\n"));
		else
			_tprintf(_T("com passageiro!\n"));
	}
	return;
}

void listarPassageiros(DADOS* dados) {
	for (int i = 0; i < dados->nPassageiros; i++) {
		_tprintf(_T("\nPassageiro %d : "), i);
		_tprintf(_T("\n (%d, %d) -> (%d, %d)\n"), dados->passageiros[i].X, dados->passageiros[i].Y, dados->passageiros[i].Xfinal, dados->passageiros[i].Yfinal);
	}
	return;
}

DWORD WINAPI ThreadNovoTaxi(LPVOID param) {		//VERIFICA SE HA NOVOS TAXIS
	DADOS* dados = ((DADOS*)param);

	return 0;
}

DWORD WINAPI ThreadNovoPassageiro(LPVOID param) {		//VERIFICA SE HA NOVOS PASSAGEIROS
	DADOS* dados = ((DADOS*)param);

	return 0;
}

boolean adicionaTaxi(DADOS* dados, TAXI novo) {
	if (dados->nTaxis >= MAXTAXIS)
		return FALSE;
	for (int i = 0; i < dados->nTaxis; i++)
		if (_tcscmp(novo.matricula, dados->taxis[i].matricula))
			return FALSE;

	dados->taxis[dados->nTaxis] = novo;
	dados->nTaxis++;
	return TRUE;
}

boolean removeTaxi(DADOS* dados, TAXI novo) {
	for (int i = 0; i < dados->nTaxis; i++) {
		if (_tcscmp(novo.matricula, dados->taxis[i].matricula)) {
			for (int k = i; k < dados->nTaxis - 1; k++) {
				dados->taxis[k] = dados->taxis[k + 1];
			}
			dados->nTaxis--;
			return TRUE;
		}
	}
	return FALSE;
}

boolean adicionaPassageiro(DADOS* dados, PASSAGEIRO novo) {
	if (dados->nPassageiros >= MAXPASS)
		return FALSE;

	dados->passageiros[dados->nPassageiros] = novo;
	dados->nPassageiros++;
	return TRUE;
}

boolean removePassageiro(DADOS* dados, PASSAGEIRO novo) {
	for (int i = 0; i < dados->nPassageiros; i++) {
		if (_tcscmp(novo.id, dados->passageiros[i].id)) {
			for (int k = i; k < dados->nPassageiros - 1; k++) {
				dados->passageiros[k] = dados->passageiros[k + 1];
			}
			dados->nPassageiros--;
			return TRUE;
		}
	}
	return FALSE;
}