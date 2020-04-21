#include <windows.h>
#include <tchar.h>
#include <fcntl.h>
#include <io.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define NQ_INICIAL 10
#define TAM 200
#define WAITTIMEOUT 2000

#define SHM_NAME TEXT("EspacoTaxis")
#define NOME_MUTEX TEXT("MutexTaxi")
#define EVENT_NAME TEXT("NovoTaxi")

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
	HANDLE hMutex;
} TAXI;

HANDLE EspTaxis;	//FileMapping
TAXI* shared;
HANDLE novoTaxi;

unsigned int NQ = NQ_INICIAL;

void ajuda();
void inicializaTaxi(TAXI* taxi);
DWORD WINAPI ThreadComandos(LPVOID param);
DWORD WINAPI ThreadMovimentaTaxi(LPVOID param);
DWORD WINAPI ThreadRespostaTransporte(LPVOID param);

int _tmain() {
	HANDLE hThreadComandos, hThreadMovimentaTaxi, hThreadRespostaTransporte;
	TAXI taxi;

#ifdef UNICODE
	_setmode(_fileno(stdin), _O_WTEXT);
	_setmode(_fileno(stdout), _O_WTEXT);
#endif

	inicializaTaxi(&taxi);

	hThreadComandos = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ThreadComandos, (LPVOID)&taxi, 0, NULL); //CREATE_SUSPENDED para nao comecar logo
	if (hThreadComandos == NULL) {
		_tprintf(TEXT("\nErro ao lançar Thread!\n"));
		return 0;
	}
	hThreadMovimentaTaxi = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ThreadMovimentaTaxi, (LPVOID)&taxi, 0, NULL); //CREATE_SUSPENDED para nao comecar logo
	if (hThreadMovimentaTaxi == NULL) {
		_tprintf(TEXT("\nErro ao lançar Thread!\n"));
		return 0;
	}
	//hThreadRespostaTransporte = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ThreadRespostaTransporte, (LPVOID)&taxi, 0, NULL); //CREATE_SUSPENDED para nao comecar logo
	//if (hThreadRespostaTransporte == NULL) {
	//	_tprintf(TEXT("\nErro ao lançar Thread!\n"));
	//	return 0;
	//}

	HANDLE ghEvents[2];
	ghEvents[0] = hThreadComandos;
	ghEvents[1] = hThreadMovimentaTaxi;
	//ghEvents[2] = hThreadRespostaTransporte;
	DWORD dwResultEspera;
	do {
		dwResultEspera = WaitForMultipleObjects(2, ghEvents, TRUE, WAITTIMEOUT);
		if (dwResultEspera == WAITTIMEOUT) {
			taxi.terminar = 1;
			_tprintf(TEXT("As Threads vao parar!\n"));
			break;
		}
	} while (1);

	_gettch();

	UnmapViewOfFile(shared);
	CloseHandle(EspTaxis);
	CloseHandle(novoTaxi);
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
	taxi->matricula[6]='\0';

	_tprintf(_T("\n Localizacao do Táxi (X Y) : "));
	_tscanf_s(_T("%d"), &taxi->X);
	_tscanf_s(_T("%d"), &taxi->Y);

	taxi->hMutex = CreateMutex(NULL, FALSE, NOME_MUTEX);
	if (taxi->hMutex == NULL) {
		_tprintf(TEXT("\nErro ao criar Mutex!\n"));
		return;
	}
	WaitForSingleObject(taxi->hMutex, INFINITE);

	taxi->disponivel = 1;
	taxi->velocidade = 1;
	taxi->autoResposta = 1;
	taxi->interessado = 0;
	taxi->terminar = 0;
	taxi->Xfinal = 0;
	taxi->Yfinal = 0;

	novoTaxi = CreateEvent(NULL, TRUE, FALSE, EVENT_NAME);
	if (novoTaxi == NULL) {
		_tprintf(TEXT("CreateEvent failed.\n"));
		return;
	}
	SetEvent(novoTaxi);
	ResetEvent(novoTaxi);

	EspTaxis = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(TAXI), SHM_NAME);
	if (EspTaxis == NULL)
	{
		_tprintf(TEXT("CreateFileMapping failed.\n"));
		return;
	}

	shared = (TAXI*)MapViewOfFile(EspTaxis, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(TAXI));
	if (shared == NULL)
	{
		_tprintf(TEXT("Terminal failure: MapViewOfFile.\n"));
		CloseHandle(EspTaxis);
		return;
	}

	//VAI AO ADMIN VER SE PODE CRIAR	
	CopyMemory(shared, taxi, sizeof(TAXI));					//VERIFICAR SE ESTA A MANDAR ISTO BEM
	//VERIFICAR QUE É UNICA
	ReleaseMutex(taxi->hMutex);

	SetEvent(novoTaxi);
	Sleep(500);
	ResetEvent(novoTaxi);

	return;
}

DWORD WINAPI ThreadComandos(LPVOID param) {
	TCHAR op[TAM];
	TAXI* taxi = ((TAXI*)param);

	do {
		_tprintf(_T("\n>>"));
		WaitForSingleObject(taxi->hMutex, INFINITE);	//ARRANJAR FORMA DE ELE NAO PARAR AQUI - SE FOR DPS ELE NAO DEIXA ESCREVER FRASE COMPLETA
		_fgetts(op, TAM, stdin);
		op[_tcslen(op) - 1] = '\0';
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
		ReleaseMutex(taxi->hMutex);
	} while (_tcscmp(op, TEXT("fim")));

	taxi->terminar = 1;

	ExitThread(0);
}

//ASSEGURAR TAMANHO DO MAPA
DWORD WINAPI ThreadMovimentaTaxi(LPVOID param) {	//MANDA TAXI AO ADMIN
	TAXI* taxi = ((TAXI*)param);
	int val, valido;

	srand((unsigned)time(NULL));

	do {
		valido = 0;
		WaitForSingleObject(taxi->hMutex, INFINITE);
		//MOVIMENTA
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
		ReleaseMutex(taxi->hMutex);
		//MANDA PARA ADMIN
		Sleep(1000);
	} while (1);

	ExitThread(0);
}

DWORD WINAPI ThreadRespostaTransporte(LPVOID param) {	//MANDA TAXI AO ADMIN E RECEBE PASSAGEIRO DO ADMIN -- BUFFER CIRCULAR
	TAXI* taxi = ((TAXI*)param);

	ExitThread(0);
}