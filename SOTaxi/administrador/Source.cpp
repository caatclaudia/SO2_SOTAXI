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
#define WAITTIMEOUT 1000

#define SHM_TAXI TEXT("EspacoTaxis")
#define NOME_MUTEX_TAXI TEXT("MutexTaxi")
#define EVENT_NOVOT TEXT("NovoTaxi")
#define EVENT_SAIUT TEXT("SaiuTaxi")
#define EVENT_MOVIMENTO TEXT("MovimentoTaxi")
#define EVENT_RESPOSTA TEXT("RespostaDoAdmin")
#define EVENT_SAIUA TEXT("SaiuAdmin")

#define SHM_MAPA TEXT("EspacoMapa")
#define EVENT_RECEBEMAP TEXT("MapaInicial")
#define NOME_MUTEX_MAPA TEXT("MutexMapa")

//CenTaxi
//1 instancia
//le o mapa e gere-o
//sabe dos taxis, posicoes e estado

typedef struct {
	char caracter;
} MAPA;

typedef struct {
	TCHAR id[TAM];
	unsigned int X, Y, Xfinal, Yfinal;
	int movimento;
	int terminar;
} PASSAGEIRO;

typedef struct {
	TCHAR matricula[7];
	unsigned int X, Y, Xfinal, Yfinal;
	int disponivel;
	TCHAR idPassageiro[TAM];
	float velocidade;
	int autoResposta;
	int interessado;
	int terminar;
} TAXI;


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
	int respostaAuto;
	int esperaManifestacoes;

	int nPassageiros;
	PASSAGEIRO passageiros[MAXPASS];

	MAPA mapa[TAM][TAM];
	HANDLE EspMapa;	//FileMapping
	MAPA* sharedMapa;
	HANDLE recebeMap;

	int terminar;
	HANDLE saiuAdmin;
} DADOS;


void ajuda();
void listarTaxis(DADOS* dados);
void listarPassageiros(DADOS* dados);
void recebeMapa(DADOS* dados);
boolean adicionaTaxi(DADOS* dados, TAXI novo);
boolean removeTaxi(DADOS* dados, TAXI novo);
boolean adicionaPassageiro(DADOS* dados, PASSAGEIRO novo);
boolean removePassageiro(DADOS* dados, PASSAGEIRO novo);
DWORD WINAPI ThreadComandos(LPVOID param);
DWORD WINAPI ThreadNovoTaxi(LPVOID param);
DWORD WINAPI ThreadSaiuTaxi(LPVOID param);
DWORD WINAPI ThreadMovimento(LPVOID param);
DWORD WINAPI ThreadNovoPassageiro(LPVOID param);


int _tmain(int argc, LPTSTR argv[]) {
	HANDLE hThreadComandos, hThreadNovoTaxi, hThreadSaiuTaxi, hThreadMovimento, hThreadNovoPassageiro;
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

	dados.hMutexDados = CreateMutex(NULL, FALSE, TEXT("MutexDados"));
	if (dados.hMutexDados == NULL) {
		_tprintf(TEXT("\n[ERRO] Erro ao criar Mutex!\n"));
		return 0;
	}
	WaitForSingleObject(dados.hMutexDados, INFINITE);
	ReleaseMutex(dados.hMutexDados);

	dados.novoTaxi = CreateEvent(NULL, TRUE, FALSE, EVENT_NOVOT);
	if (dados.novoTaxi == NULL) {
		_tprintf(TEXT("\n[ERRO] Erro ao criar Evento!\n"));
		return 0;
	}
	SetEvent(dados.novoTaxi);
	ResetEvent(dados.novoTaxi);

	dados.saiuTaxi = CreateEvent(NULL, TRUE, FALSE, EVENT_SAIUT);
	if (dados.saiuTaxi == NULL) {
		_tprintf(TEXT("\n[ERRO] Erro ao criar Evento!\n"));
		return 0;
	}
	SetEvent(dados.saiuTaxi);
	ResetEvent(dados.saiuTaxi);

	dados.movimentoTaxi = CreateEvent(NULL, TRUE, FALSE, EVENT_MOVIMENTO);
	if (dados.movimentoTaxi == NULL) {
		_tprintf(TEXT("\n[ERRO] Erro ao criar Evento!\n"));
		return 0;
	}
	SetEvent(dados.movimentoTaxi);
	ResetEvent(dados.movimentoTaxi);

	dados.respostaAdmin = CreateEvent(NULL, TRUE, FALSE, EVENT_RESPOSTA);
	if (dados.respostaAdmin == NULL) {
		_tprintf(TEXT("\n[ERRO] Erro ao criar Evento!\n"));
		return 0;
	}

	dados.saiuAdmin = CreateEvent(NULL, TRUE, FALSE, EVENT_SAIUA);
	if (dados.saiuAdmin == NULL) {
		_tprintf(TEXT("\n[ERRO] Erro ao criar Evento!\n"));
		return 0;
	}

	dados.EspTaxis = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(TAXI), SHM_TAXI);
	if (dados.EspTaxis == NULL)
	{
		_tprintf(TEXT("\n[ERRO] Erro ao criar FileMapping!\n"));
		return 0;
	}

	dados.sharedTaxi = (TAXI*)MapViewOfFile(dados.EspTaxis, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(TAXI));
	if (dados.sharedTaxi == NULL)
	{
		_tprintf(TEXT("\n[ERRO] Erro em MapViewOfFile!\n"));
		CloseHandle(dados.EspTaxis);
		return 0;
	}

	recebeMapa(&dados);

	hThreadComandos = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ThreadComandos, (LPVOID)&dados, 0, NULL); //CREATE_SUSPENDED para nao comecar logo
	if (hThreadComandos == NULL) {
		_tprintf(TEXT("\n[ERRO] Erro ao lançar Thread!\n"));
		return 0;
	}
	hThreadNovoTaxi = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ThreadNovoTaxi, (LPVOID)&dados, 0, NULL); //CREATE_SUSPENDED para nao comecar logo
	if (hThreadNovoTaxi == NULL) {
		_tprintf(TEXT("\n[ERRO] Erro ao lançar Thread!\n"));
		return 0;
	}
	hThreadSaiuTaxi = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ThreadSaiuTaxi, (LPVOID)&dados, 0, NULL); //CREATE_SUSPENDED para nao comecar logo
	if (hThreadSaiuTaxi == NULL) {
		_tprintf(TEXT("\n[ERRO] Erro ao lançar Thread!\n"));
		return 0;
	}
	hThreadMovimento = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ThreadMovimento, (LPVOID)&dados, 0, NULL); //CREATE_SUSPENDED para nao comecar logo
	if (hThreadMovimento == NULL) {
		_tprintf(TEXT("\n[ERRO] Erro ao lançar Thread!\n"));
		return 0;
	}
	//hThreadNovoPassageiro = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ThreadNovoPassageiro, (LPVOID)&dados, 0, NULL); //CREATE_SUSPENDED para nao comecar logo
	//if (hThreadNovoPassageiro == NULL) {
	//	_tprintf(TEXT("\nErro ao lançar Thread!\n"));
	//	return 0;
	//}

	HANDLE ghEvents[4];
	ghEvents[0] = hThreadComandos;
	ghEvents[1] = hThreadNovoTaxi;
	ghEvents[2] = hThreadSaiuTaxi;
	ghEvents[3] = hThreadMovimento;
	//ghEvents[2] = hThreadNovoPassageiro;
	WaitForMultipleObjects(4, ghEvents, TRUE, INFINITE);

	WaitForSingleObject(dados.hMutexDados, INFINITE);

	for (int i = 0; i < dados.nTaxis; i++)
		dados.taxis[i].terminar = 1;
	for (int i = 0; i < dados.nPassageiros; i++)
		dados.passageiros[i].terminar = 1;

	SetEvent(dados.saiuAdmin);
	Sleep(500);
	ResetEvent(dados.saiuAdmin);

	ReleaseMutex(dados.hMutexDados);
	Sleep(1000);

	_tprintf(_T("Administrador vai encerrar!\n"));
	_tprintf(TEXT("Prima uma tecla...\n"));
	_gettch();

	UnmapViewOfFile(dados.sharedTaxi);
	CloseHandle(dados.EspTaxis);
	CloseHandle(dados.novoTaxi);
	CloseHandle(dados.saiuTaxi);
	CloseHandle(dados.movimentoTaxi);
	CloseHandle(dados.respostaAdmin);
	CloseHandle(dados.saiuAdmin);
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
		_tprintf(_T("\n[LISTAR TAXIS] Taxi %d : "), i);
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
		_tprintf(_T("\n[LISTAR PASSAGEIRO] Passageiro %d : "), i);
		_tprintf(_T("\n (%d, %d) -> (%d, %d)\n"), dados->passageiros[i].X, dados->passageiros[i].Y, dados->passageiros[i].Xfinal, dados->passageiros[i].Yfinal);
	}
	return;
}

void recebeMapa(DADOS* dados) {
	dados->EspMapa = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(MAPA), SHM_MAPA);
	if (dados->EspMapa == NULL)
	{
		_tprintf(TEXT("\n[ERRO] Erro ao criar FileMapping!\n"));
		return;
	}

	dados->sharedMapa = (MAPA*)MapViewOfFile(dados->EspMapa, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(MAPA));
	if (dados->sharedMapa == NULL)
	{
		_tprintf(TEXT("\n[ERRO] Erro em MapViewOfFile!\n"));
		CloseHandle(dados->EspMapa);
		return;
	}

	dados->recebeMap = CreateEvent(NULL, TRUE, FALSE, EVENT_RECEBEMAP);
	if (dados->recebeMap == NULL) {
		_tprintf(TEXT("\n[ERRO] Erro ao criar Evento!\n"));
		return;
	}
	SetEvent(dados->recebeMap);
	ResetEvent(dados->recebeMap);

	WaitForSingleObject(dados->recebeMap, WAITTIMEOUT);

	WaitForSingleObject(dados->hMutexDados, INFINITE);

	CopyMemory(&dados->mapa, dados->sharedMapa, sizeof(MAPA));								//NÃO RECEBE BEM OS VALORES 
	_tprintf(TEXT("\n[MAPA] Mapa lido com sucesso!\n"));
	for (int x = 0; x < 50; x++) {
		for (int y = 0; y < 51; y++)
			_tprintf(TEXT("%c"), dados->mapa[x][y].caracter);
		_tprintf(TEXT("\n"));
	}

	ReleaseMutex(dados->hMutexDados);
	Sleep(1000);

	return;
}

DWORD WINAPI ThreadComandos(LPVOID param) {
	TCHAR op[TAM];
	DADOS* dados = ((DADOS*)param);

	do {
		_tprintf(_T("\n>>"));
		_fgetts(op, TAM, stdin);
		op[_tcslen(op) - 1] = '\0';
		WaitForSingleObject(dados->hMutexDados, INFINITE);
		if (_tcscmp(op, TEXT("expulsar"))) {		//EXPULSAR TAXI

		}
		else if (_tcscmp(op, TEXT("listar"))) {		//LISTAR TAXIS
			listarTaxis(dados);
		}
		else if (_tcscmp(op, TEXT("aceitacaoT"))) {		//PAUSAR/RECOMECAR ACEITAÇÃO DE TAXIS
			if (dados->respostaAuto) {
				dados->respostaAuto = 0;
				_tprintf(_T("\n[COMANDO] Pausar aceitação automática de passageiros"));
			}
			else {
				dados->respostaAuto = 1;
				_tprintf(_T("\n[COMANDO] Recomeçar aceitação automática de passageiros"));
			}
			//ENVIAR INFORMAÇÃO AOS TAXIS
		}
		else if (_tcscmp(op, TEXT("manifestacoes"))) {		//DEFINIR INTERVALO DE TEMPO DURANTE O QUAL AGUARDA MANIFESTAÇOES DOS TAXIS
			_tprintf(_T("\n[COMANDO] Intervalo de tempo durante o qual aguarda manifestações (em segundos): "));
			_tscanf_s(_T("%d"), &dados->esperaManifestacoes);
			if (dados->esperaManifestacoes <= 0)
				dados->esperaManifestacoes = TempoManifestacoes;
		}
		else if (_tcscmp(op, TEXT("ajuda"))) {		//AJUDA NOS COMANDOS
			ajuda();
		}
		if (_tcscmp(op, TEXT("fim")))
			ReleaseMutex(dados->hMutexDados);
	} while (_tcscmp(op, TEXT("fim")));

	dados->terminar = 1;

	ReleaseMutex(dados->hMutexDados);

	ExitThread(0);
}

DWORD WINAPI ThreadNovoTaxi(LPVOID param) {		//VERIFICA SE HA NOVOS TAXIS
	DADOS* dados = ((DADOS*)param);
	TAXI novo;

	while (1) {
		while (1) {
			WaitForSingleObject(dados->novoTaxi, WAITTIMEOUT);
			if (dados->terminar)
				return 0;
		}

		if (dados->terminar)
			return 0;
		WaitForSingleObject(dados->hMutexDados, INFINITE);

		CopyMemory(&novo, dados->sharedTaxi, sizeof(TAXI));
		//adicionaTaxi(dados, novo);
		if (adicionaTaxi(dados, novo)) {
			_tprintf(TEXT("Novo Taxi: %s\n"), dados->taxis[dados->nTaxis - 1].matricula);
			CopyMemory(dados->sharedTaxi, &dados->taxis[dados->nTaxis - 1], sizeof(TAXI));
		}
		else {
			novo.terminar = 1;
			CopyMemory(dados->sharedTaxi, &novo, sizeof(TAXI));
		}
		SetEvent(dados->respostaAdmin);
		Sleep(500);
		ResetEvent(dados->respostaAdmin);

		ReleaseMutex(dados->hMutexDados);

		Sleep(1000);
	}

	ExitThread(0);
}

DWORD WINAPI ThreadSaiuTaxi(LPVOID param) {		//VERIFICA SE SAIRAM TAXIS
	DADOS* dados = ((DADOS*)param);
	TAXI novo;

	while (1) {
		while (1) {
			WaitForSingleObject(dados->saiuTaxi, WAITTIMEOUT);
			if (dados->terminar)
				return 0;
		}

		if (dados->terminar)
			return 0;
		WaitForSingleObject(dados->hMutexDados, INFINITE);

		CopyMemory(&novo, dados->sharedTaxi, sizeof(TAXI));
		removeTaxi(dados, novo);

		ReleaseMutex(dados->hMutexDados);

		Sleep(1000);
	}

	ExitThread(0);
}

DWORD WINAPI ThreadMovimento(LPVOID param) {
	DADOS* dados = ((DADOS*)param);
	TAXI novo;

	while (1) {
		while (1) {
			WaitForSingleObject(dados->movimentoTaxi, WAITTIMEOUT);
			if (dados->terminar)
				return 0;
		}

		if (dados->terminar)
			return 0;
		WaitForSingleObject(dados->hMutexDados, INFINITE);

		CopyMemory(&novo, dados->sharedTaxi, sizeof(TAXI));
		for (int i = 0; i < dados->nTaxis; i++)
			if (!_tcscmp(novo.matricula, dados->taxis[i].matricula)) {
				dados->taxis[i] = novo;
				_tprintf(_T("\n[MOVIMENTO] Taxi %s -> (%d,%d)"), novo.matricula, novo.X, novo.Y);
			}

		ReleaseMutex(dados->hMutexDados);

		Sleep(1000);
	}

	ExitThread(0);
}

DWORD WINAPI ThreadNovoPassageiro(LPVOID param) {		//VERIFICA SE HA NOVOS PASSAGEIROS
	DADOS* dados = ((DADOS*)param);

	ExitThread(0);
}

boolean adicionaTaxi(DADOS* dados, TAXI novo) {
	if (dados->nTaxis >= MAXTAXIS)
		return FALSE;
	for (int i = 0; i < dados->nTaxis; i++)
		if (_tcscmp(novo.matricula, dados->taxis[i].matricula))
			return FALSE;

	dados->taxis[dados->nTaxis] = novo;
	dados->nTaxis++;
	_tprintf(TEXT("[NOVO TAXI] Novo Taxi: %s\n"), novo.matricula);
	return TRUE;
}

boolean removeTaxi(DADOS* dados, TAXI novo) {
	for (int i = 0; i < dados->nTaxis; i++) {
		if (_tcscmp(novo.matricula, dados->taxis[i].matricula)) {
			for (int k = i; k < dados->nTaxis - 1; k++) {
				dados->taxis[k] = dados->taxis[k + 1];
			}
			dados->nTaxis--;
			_tprintf(TEXT("[SAIU TAXI] Saiu Taxi: %s\n"), novo.matricula);
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
	_tprintf(TEXT("[NOVO PASSAGEIRO] Novo Passageiro: %s\n"), novo.id);
	return TRUE;
}

boolean removePassageiro(DADOS* dados, PASSAGEIRO novo) {
	for (int i = 0; i < dados->nPassageiros; i++) {
		if (_tcscmp(novo.id, dados->passageiros[i].id)) {
			for (int k = i; k < dados->nPassageiros - 1; k++) {
				dados->passageiros[k] = dados->passageiros[k + 1];
			}
			dados->nPassageiros--;
			_tprintf(TEXT("[SAIU PASSAGEIRO] Saiu Passageiro: %s\n"), novo.id);
			return TRUE;
		}
	}
	return FALSE;
}