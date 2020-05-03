#include <windows.h>
#include <tchar.h>
#include <fcntl.h>
#include <io.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define NQ_INICIAL 10
#define TAM 200
#define WAITTIMEOUT 1000

#define SHM_NAME TEXT("EspacoTaxis")
#define NOME_MUTEX TEXT("MutexTaxi")
#define EVENT_NOVOT TEXT("NovoTaxi")
#define EVENT_SAIUT TEXT("SaiuTaxi")
#define EVENT_MOVIMENTO TEXT("MovimentoTaxi")
#define EVENT_RESPOSTA TEXT("RespostaDoAdmin")
#define EVENT_SAIUA TEXT("SaiuAdmin")

//ConTaxi
//1 instancia por taxi
//Movimenta taxi aleatoriamente
//MOSTRA: NOVO PASSAGEIRO, INTERESSE EM TRANSPORTAR ENVIADO, CONFIRMACAO RECEBIDA, RECOLHA DE PASSAGEIRO INICIADA, PASSAGEIRO RECOLHIDO, PASSAGEIRO ENTREGUE

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
HANDLE hMutex;

HANDLE EspTaxis;	//FileMapping
TAXI* shared;
HANDLE novoTaxi;
HANDLE saiuTaxi;
HANDLE movimentoTaxi;
HANDLE respostaAdmin;
HANDLE saiuAdmin;

unsigned int NQ = NQ_INICIAL;

void ajuda();
void inicializaTaxi(TAXI* taxi);
DWORD WINAPI ThreadComandos(LPVOID param);
DWORD WINAPI ThreadMovimentaTaxi(LPVOID param);
DWORD WINAPI ThreadSaiuAdmin(LPVOID param);
DWORD WINAPI ThreadRespostaTransporte(LPVOID param);

int _tmain() {
	HANDLE hThreadComandos, hThreadMovimentaTaxi, hThreadRespostaTransporte, hThreadSaiuAdmin;
	TAXI taxi;

#ifdef UNICODE
	_setmode(_fileno(stdin), _O_WTEXT);
	_setmode(_fileno(stdout), _O_WTEXT);
#endif

	inicializaTaxi(&taxi);
	if (!taxi.terminar) {


		//WaitForSingleObject(taxi.hMutex, INFINITE);

		hThreadComandos = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ThreadComandos, (LPVOID)&taxi, 0, NULL); //CREATE_SUSPENDED para nao comecar logo
		if (hThreadComandos == NULL) {
			_tprintf(TEXT("\n[ERRO] Erro ao lançar Thread!\n"));
			return 0;
		}
		hThreadMovimentaTaxi = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ThreadMovimentaTaxi, (LPVOID)&taxi, 0, NULL); //CREATE_SUSPENDED para nao comecar logo
		if (hThreadMovimentaTaxi == NULL) {
			_tprintf(TEXT("\n[ERRO] Erro ao lançar Thread!\n"));
			return 0;
		}
		hThreadSaiuAdmin = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ThreadSaiuAdmin, (LPVOID)&taxi, 0, NULL); //CREATE_SUSPENDED para nao comecar logo
		if (hThreadSaiuAdmin == NULL) {
			_tprintf(TEXT("\n[ERRO] Erro ao lançar Thread!\n"));
			return 0;
		}
		//hThreadRespostaTransporte = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ThreadRespostaTransporte, (LPVOID)&taxi, 0, NULL); //CREATE_SUSPENDED para nao comecar logo
		//if (hThreadRespostaTransporte == NULL) {
		//	_tprintf(TEXT("\nErro ao lançar Thread!\n"));
		//	return 0;
		//}

		HANDLE ghEvents[3];
		ghEvents[0] = hThreadComandos;
		ghEvents[1] = hThreadMovimentaTaxi;
		ghEvents[2] = hThreadSaiuAdmin;
		//ghEvents[3] = hThreadRespostaTransporte;
		WaitForMultipleObjects(3, ghEvents, FALSE, INFINITE);
		TerminateThread(hThreadSaiuAdmin, 0);
	}

	_tprintf(TEXT("\nTaxi a sair!"));
	_tprintf(TEXT("\nPrima uma tecla...\n"));
	_gettch();

	UnmapViewOfFile(shared);
	CloseHandle(EspTaxis);
	CloseHandle(novoTaxi);
	CloseHandle(saiuTaxi);
	CloseHandle(movimentoTaxi);
	CloseHandle(respostaAdmin);
	CloseHandle(saiuAdmin);
	return 0;
}

void ajuda() {
	_tprintf(_T("\n\n aumentaV - AUMENTA 0.5 DE VELOCIDADE"));
	_tprintf(_T("\n diminuiV - DIMINUI 0.5 DE VELOCIDADE"));
	_tprintf(_T("\n numQuad - DEFINIR NQ"));
	_tprintf(_T("\n respostaAuto - LIGAR/DESLIGAR RESPOSTA AUTOMÁTICA AOS PEDIDOS DE TRANSPORTE"));
	_tprintf(_T("\n pass - TRANSPORTAR PASSAGEIRO"));
	_tprintf(_T("\n fim - ENCERRAR TODO O SISTEMA"));
	return;
}

void inicializaTaxi(TAXI* taxi) {
	int num;

	do {
		num = 0;
		_tprintf(_T("\n Matricula do Táxi: "));
		_tscanf_s(_T("%s"), taxi->matricula, sizeof(taxi->matricula));
		for (int i = 0; i < 6; i++)
			if (isalpha(taxi->matricula[i]))
				num++;
	} while (num != 2);
	taxi->matricula[6] = '\0';

	_tprintf(_T("\n Localizacao do Táxi (X Y) : "));
	_tscanf_s(_T("%d %d"), &taxi->X, &taxi->Y);

	hMutex = CreateMutex(NULL, FALSE, NOME_MUTEX);
	if (hMutex == NULL) {
		_tprintf(TEXT("\n[ERRO] Erro ao criar Mutex!\n"));
		return;
	}
	WaitForSingleObject(hMutex, INFINITE);

	taxi->disponivel = 1;
	taxi->velocidade = 1;
	taxi->autoResposta = 1;
	taxi->interessado = 0;
	taxi->terminar = 0;
	taxi->Xfinal = 0;
	taxi->Yfinal = 0;

	novoTaxi = CreateEvent(NULL, TRUE, FALSE, EVENT_NOVOT);
	if (novoTaxi == NULL) {
		_tprintf(TEXT("\n[ERRO] Erro ao criar Evento!\n"));
		return;
	}

	saiuTaxi = CreateEvent(NULL, TRUE, FALSE, EVENT_SAIUT);
	if (saiuTaxi == NULL) {
		_tprintf(TEXT("\n[ERRO] Erro ao criar Evento!\n"));
		return;
	}

	movimentoTaxi = CreateEvent(NULL, TRUE, FALSE, EVENT_MOVIMENTO);
	if (movimentoTaxi == NULL) {
		_tprintf(TEXT("\n[ERRO] Erro ao criar Evento!\n"));
		return;
	}

	respostaAdmin = CreateEvent(NULL, TRUE, FALSE, EVENT_RESPOSTA);
	if (respostaAdmin == NULL) {
		_tprintf(TEXT("\n[ERRO] Erro ao criar Evento!\n"));
		return;
	}
	SetEvent(respostaAdmin);
	Sleep(500);
	ResetEvent(respostaAdmin);

	saiuAdmin = CreateEvent(NULL, TRUE, FALSE, EVENT_SAIUA);
	if (saiuAdmin == NULL) {
		_tprintf(TEXT("\n[ERRO] Erro ao criar Evento!\n"));
		return;
	}
	SetEvent(saiuAdmin);
	Sleep(500);
	ResetEvent(saiuAdmin);

	EspTaxis = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(TAXI), SHM_NAME);
	if (EspTaxis == NULL)
	{
		_tprintf(TEXT("\n[ERRO] Erro ao criar FileMapping!\n"));
		return;
	}

	shared = (TAXI*)MapViewOfFile(EspTaxis, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(TAXI));
	if (shared == NULL)
	{
		_tprintf(TEXT("\n[ERRO] Erro em MapViewOfFile!\n"));
		CloseHandle(EspTaxis);
		return;
	}

	//VAI AO ADMIN VER SE PODE CRIAR	
	CopyMemory(shared, taxi, sizeof(TAXI));
	FlushViewOfFile(novoTaxi, 0);
	//VERIFICAR QUE É UNICA

	SetEvent(novoTaxi);
	Sleep(500);
	ResetEvent(novoTaxi);

	WaitForSingleObject(respostaAdmin, INFINITE);

	CopyMemory(taxi, shared, sizeof(TAXI));
	if (!taxi->terminar)
		_tprintf(TEXT("\nBem Vindo!\n"));

	ReleaseMutex(hMutex);

	Sleep(1000);

	return;
}

DWORD WINAPI ThreadComandos(LPVOID param) {
	TCHAR op[TAM];
	TAXI* taxi = ((TAXI*)param);

	do {
		_tprintf(_T("\n>>"));
		_fgetts(op, TAM, stdin);
		op[_tcslen(op) - 1] = '\0';
		WaitForSingleObject(hMutex, INFINITE);
		if (!_tcscmp(op, TEXT("aumentaV"))) {		//AUMENTA 0.5 DE VELOCIDADE
			taxi->velocidade += 0.5;
			_tprintf(_T("\n[COMANDO] Velocidade atual : %f"), taxi->velocidade);
		}
		else if (!_tcscmp(op, TEXT("diminuiV"))) {		//DIMINUI 0.5 DE VELOCIDADE
			if (taxi->velocidade >= 0.5)
				taxi->velocidade -= 0.5;
			_tprintf(_T("\n[COMANDO] Velocidade atual : %f"), taxi->velocidade);
		}
		else if (!_tcscmp(op, TEXT("numQuad"))) {		//DEFINIR NQ
			_tprintf(_T("\n[COMANDO] Valor de NQ (número de quadriculas até ao passageiro) : "));
			_tscanf_s(_T("%d"), &NQ);
			if (NQ < 0)
				NQ = NQ_INICIAL;
		}
		else if (!_tcscmp(op, TEXT("respostaAuto"))) {		//LIGAR/DESLIGAR RESPOSTA AUTOMÁTICA AOS PEDIDOS DE TRANSPORTE
			if (taxi->autoResposta) {
				taxi->autoResposta = 0;
				_tprintf(_T("\n[COMANDO] Desligada resposta automática a pedidos de transporte"));
			}
			else {
				taxi->autoResposta = 1;
				_tprintf(_T("\n[COMANDO] Ligada resposta automática a pedidos de transporte"));
			}
		}
		else if (!_tcscmp(op, TEXT("pass"))) {		//TRANSPORTAR PASSAGEIRO
			if (!taxi->autoResposta)
				taxi->interessado = 1;
			_tprintf(_T("\n[COMANDO] Tenho interesse neste passageiro!"));
		}
		else if (!_tcscmp(op, TEXT("ajuda"))) {		//AJUDA NOS COMANDOS
			_tprintf(_T("\n[COMANDO] Aqui vai uma ajuda..."));
			ajuda();
		}
		if (_tcscmp(op, TEXT("fim")))
			ReleaseMutex(hMutex);
	} while (_tcscmp(op, TEXT("fim")));

	taxi->terminar = 1;

	SetEvent(saiuTaxi);
	ResetEvent(saiuTaxi);

	CopyMemory(shared, taxi, sizeof(TAXI));

	ReleaseMutex(hMutex);

	SetEvent(saiuTaxi);
	Sleep(500);
	ResetEvent(saiuTaxi);

	Sleep(1000);

	ExitThread(0);
}

//ASSEGURAR TAMANHO DO MAPA
DWORD WINAPI ThreadMovimentaTaxi(LPVOID param) {	//MANDA TAXI AO ADMIN
	TAXI* taxi = ((TAXI*)param);
	int val, valido;

	srand((unsigned)time(NULL));

	do {
		valido = 0;
		WaitForSingleObject(hMutex, INFINITE);
		if (taxi->terminar)
			return 0;
		//MOVIMENTA
		if (taxi->velocidade != 0) {
			do {
				val = rand() % 4;
				switch (val) {
				case 1:	//DIREITA
					_tprintf(_T("\n[MOVIMENTO] (%d,%d) -> (%d,%d)"), taxi->X, taxi->Y, taxi->X++, taxi->Y);
					valido = 1;
					break;
				case 2: //ESQUERDA
					if (taxi->X > 0) {
						_tprintf(_T("\n[MOVIMENTO] (%d,%d) -> (%d,%d)"), taxi->X, taxi->Y, taxi->X--, taxi->Y);
						valido = 1;
					}
					break;
				case 3: //CIMA
					if (taxi->Y > 0) {
						_tprintf(_T("\n[MOVIMENTO] (%d,%d) -> (%d,%d)"), taxi->X, taxi->Y, taxi->X, taxi->Y--);
						valido = 1;
					}
					break;
				case 4: //BAIXO
					_tprintf(_T("\n[MOVIMENTO] (%d,%d) -> (%d,%d)"), taxi->X, taxi->Y, taxi->X, taxi->Y++);
					valido = 1;
					break;
				}
			} while (!valido);

			CopyMemory(shared, taxi, sizeof(TAXI));


			SetEvent(movimentoTaxi);
			Sleep(500);
			ResetEvent(movimentoTaxi);
		}
		
		ReleaseMutex(hMutex);

		Sleep(1000);
		//MANDA PARA ADMIN
	} while (!taxi->terminar);

	ExitThread(0);
}

DWORD WINAPI ThreadSaiuAdmin(LPVOID param) { //NAMED PIPES
	TAXI* taxi = ((TAXI*)param);

	WaitForSingleObject(saiuAdmin, INFINITE);

	WaitForSingleObject(hMutex, INFINITE);

	taxi->terminar = 1;

	ReleaseMutex(hMutex);
	_tprintf(_T("\n[AVISO] Administrador encerrou!"));

	Sleep(1000);

	ExitThread(0);
}

DWORD WINAPI ThreadRespostaTransporte(LPVOID param) {	//MANDA TAXI AO ADMIN E RECEBE PASSAGEIRO DO ADMIN -- BUFFER CIRCULAR
	TAXI* taxi = ((TAXI*)param);

	ExitThread(0);
}