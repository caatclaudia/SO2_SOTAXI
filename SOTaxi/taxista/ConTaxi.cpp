#include "ConTaxi.h"

int _tmain() {
	HANDLE hThreadComandos, hThreadMovimentaTaxi, hThreadRespostaTransporte, hThreadSaiuAdmin;
	DADOS dados;
	TAXI taxi;
	HINSTANCE hLib;
	dados.taxi = &taxi;

	hLib = LoadLibrary(PATH_DLL);
	if (hLib == NULL)
		return 0;

	srand((unsigned)time(NULL));

#ifdef UNICODE
	_setmode(_fileno(stdin), _O_WTEXT);
	_setmode(_fileno(stdout), _O_WTEXT);
	_setmode(_fileno(stderr), _O_WTEXT);
#endif

	ptr_register = (void(*)(TCHAR*, int))GetProcAddress(hLib, "dll_register");
		
	leMapa(&dados);
	if (dados.mapa == NULL) {
		UnmapViewOfFile(dados.sharedMap);
		CloseHandle(dados.EspMapa);
		return 0;
	}

	inicializaTaxi(&dados);
	if (!dados.taxi->terminar) {
		//WaitForSingleObject(taxi.hMutex, INFINITE);

		hThreadComandos = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ThreadComandos, (LPVOID)&dados, 0, NULL);
		if (hThreadComandos == NULL) {
			_tprintf(TEXT("\n[ERRO] Erro ao lançar Thread!\n"));
			return 0;
		}
		hThreadMovimentaTaxi = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ThreadMovimentaTaxi, (LPVOID)&dados, 0, NULL);
		if (hThreadMovimentaTaxi == NULL) {
			_tprintf(TEXT("\n[ERRO] Erro ao lançar Thread!\n"));
			return 0;
		}
		hThreadSaiuAdmin = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ThreadSaiuAdmin, (LPVOID)&dados, 0, NULL);
		if (hThreadSaiuAdmin == NULL) {
			_tprintf(TEXT("\n[ERRO] Erro ao lançar Thread!\n"));
			return 0;
		}
		//hThreadRespostaTransporte = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ThreadRespostaTransporte, (LPVOID)&taxi, 0, NULL);
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

	UnmapViewOfFile(dados.shared);
	UnmapViewOfFile(dados.sharedMap);
	CloseHandle(dados.EspMapa);
	CloseHandle(dados.EspTaxis);
	CloseHandle(dados.novoTaxi);
	CloseHandle(dados.saiuTaxi);
	CloseHandle(dados.movimentoTaxi);
	CloseHandle(dados.respostaAdmin);
	CloseHandle(dados.saiuAdmin);
	FreeLibrary(hLib);

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

int calculaDistancia(int inicioX, int inicioY, int fimX, int fimY) {
	int distancia = 0;
	if (inicioX >= fimX)
		distancia = inicioX - fimX;
	else
		distancia = fimX - inicioX;
	if (inicioY >= fimY)
		distancia += inicioY - fimY;
	else
		distancia += fimY - inicioY;
	return distancia;
}

void inicializaTaxi(DADOS* dados) {
	int num;
	int VALIDO;
	int x, y;

	do {
		num = 0;
		_tprintf(_T("\n Matricula do Táxi: "));
		_tscanf_s(_T("%s"), dados->taxi->matricula, sizeof(dados->taxi->matricula));
		for (int i = 0; i < 6; i++)
			if (isalpha(dados->taxi->matricula[i]))
				num++;
	} while (num != 2);
	dados->taxi->matricula[6] = '\0';

	do {
		VALIDO = 1;
		_tprintf(_T("\n Localizacao do Táxi (X Y) : "));
		_tscanf_s(_T("%d %d"), &x, &y);
		if (dados->mapa[tamanhoMapa * y + y + x].caracter != '_') {
			_tprintf(_T("\n[ATENÇÃO] Isso não é estrada!\n\n"));
			VALIDO = 0;
		}
	} while (!VALIDO);
	dados->taxi->X = x;
	dados->taxi->Y = y;

	hMutex = CreateMutex(NULL, FALSE, NOME_MUTEX);
	if (hMutex == NULL) {
		_tprintf(TEXT("\n[ERRO] Erro ao criar Mutex!\n"));
		return;
	}
	ptr_register((TCHAR*)NOME_MUTEX, 1);
	WaitForSingleObject(hMutex, INFINITE);

	dados->taxi->disponivel = 1;
	dados->taxi->velocidade = 1;
	dados->taxi->autoResposta = 1;
	dados->taxi->interessado = 0;
	dados->taxi->terminar = 0;
	dados->taxi->Xfinal = 0;
	dados->taxi->Yfinal = 0;
	dados->taxi->id_mapa = 0;

	dados->novoTaxi = CreateEvent(NULL, TRUE, FALSE, EVENT_NOVOT);
	if (dados->novoTaxi == NULL) {
		_tprintf(TEXT("\n[ERRO] Erro ao criar Evento!\n"));
		return;
	}
	ptr_register((TCHAR*)EVENT_NOVOT, 4);

	dados->saiuTaxi = CreateEvent(NULL, TRUE, FALSE, EVENT_SAIUT);
	if (dados->saiuTaxi == NULL) {
		_tprintf(TEXT("\n[ERRO] Erro ao criar Evento!\n"));
		CloseHandle(dados->novoTaxi);
		return;
	}
	ptr_register((TCHAR*)EVENT_SAIUT, 4);

	dados->movimentoTaxi = CreateEvent(NULL, TRUE, FALSE, EVENT_MOVIMENTO);
	if (dados->movimentoTaxi == NULL) {
		_tprintf(TEXT("\n[ERRO] Erro ao criar Evento!\n"));
		CloseHandle(dados->novoTaxi);
		CloseHandle(dados->saiuTaxi);
		return;
	}
	ptr_register((TCHAR*)EVENT_MOVIMENTO, 4);

	dados->respostaAdmin = CreateEvent(NULL, TRUE, FALSE, EVENT_RESPOSTA);
	if (dados->respostaAdmin == NULL) {
		_tprintf(TEXT("\n[ERRO] Erro ao criar Evento!\n"));
		CloseHandle(dados->novoTaxi);
		CloseHandle(dados->saiuTaxi);
		CloseHandle(dados->movimentoTaxi);
		return;
	}
	ptr_register((TCHAR*)EVENT_RESPOSTA, 4);
	SetEvent(dados->respostaAdmin);
	Sleep(500);
	ResetEvent(dados->respostaAdmin);

	dados->saiuAdmin = CreateEvent(NULL, TRUE, FALSE, EVENT_SAIUA);
	if (dados->saiuAdmin == NULL) {
		_tprintf(TEXT("\n[ERRO] Erro ao criar Evento!\n"));
		CloseHandle(dados->novoTaxi);
		CloseHandle(dados->saiuTaxi);
		CloseHandle(dados->movimentoTaxi);
		CloseHandle(dados->respostaAdmin);
		return;
	}
	ptr_register((TCHAR*)EVENT_SAIUA, 4);
	SetEvent(dados->saiuAdmin);
	Sleep(500);
	ResetEvent(dados->saiuAdmin);

	dados->EspTaxis = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(TAXI), SHM_NAME);
	if (dados->EspTaxis == NULL)
	{
		_tprintf(TEXT("\n[ERRO] Erro ao criar FileMapping!\n"));
		CloseHandle(dados->novoTaxi);
		CloseHandle(dados->saiuTaxi);
		CloseHandle(dados->movimentoTaxi);
		CloseHandle(dados->respostaAdmin);
		CloseHandle(dados->saiuAdmin);
		return;
	}
	ptr_register((TCHAR*)SHM_NAME, 6);

	dados->shared = (TAXI*)MapViewOfFile(dados->EspTaxis, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(TAXI));
	if (dados->shared == NULL)
	{
		_tprintf(TEXT("\n[ERRO] Erro em MapViewOfFile!\n"));
		CloseHandle(dados->EspTaxis);
		CloseHandle(dados->novoTaxi);
		CloseHandle(dados->saiuTaxi);
		CloseHandle(dados->movimentoTaxi);
		CloseHandle(dados->respostaAdmin);
		CloseHandle(dados->saiuAdmin);
		return;
	}
	ptr_register((TCHAR*)SHM_NAME, 7);

	//VAI AO ADMIN VER SE PODE CRIAR
	_tprintf(TEXT("\nAguardando resposta da Central...\n"));
	avisaNovoTaxi(dados);
	if (!dados->taxi->terminar)
		_tprintf(TEXT("\nBem Vindo!\n"));

	ReleaseMutex(hMutex);

	Sleep(1000);

	return;
}

void leMapa(DADOS* dados) {
	dados->EspMapa = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(dados->mapa), SHM_NAME_MAP);
	if (dados->EspMapa == NULL)
	{
		_tprintf(TEXT("\n[ERRO] Erro ao criar FileMapping!\n"));
		return;
	}
	ptr_register((TCHAR*)SHM_NAME, 6);

	dados->sharedMap = (MAPA*)MapViewOfFile(dados->EspMapa, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(dados->mapa));
	if (dados->sharedMap == NULL)
	{
		_tprintf(TEXT("\n[ERRO] Erro em MapViewOfFile!\n"));
		CloseHandle(dados->EspMapa);
		return;
	}
	ptr_register((TCHAR*)SHM_NAME, 7);

	MAPA* aux = NULL;
	CopyMemory(&aux, dados->sharedMap, sizeof(dados->sharedMap));
	if (aux == NULL) {
		_tprintf(TEXT("\n[MAPA] CenTaxi não enviou mapa!\n"));
		dados->mapa = NULL;
		return;
	}
	for (int i = 0; tamanhoMapa == -1; i++)
		if (dados->sharedMap[i].caracter == '\n')
			tamanhoMapa = i;
	dados->mapa = (MAPA*)malloc(sizeof(MAPA) * tamanhoMapa * tamanhoMapa);
	CopyMemory(dados->mapa, &aux, sizeof(dados->mapa));

	for (int i = 0; i < tamanhoMapa * tamanhoMapa; i++) {
		dados->mapa[i].caracter = dados->sharedMap[i].caracter;
		_tprintf(TEXT("%c"), dados->mapa[i].caracter);
	}

	_tprintf(TEXT("\n[MAPA] Mapa lido com sucesso!\n\n"));

	return;
}

DWORD WINAPI ThreadComandos(LPVOID param) {
	TCHAR op[TAM];
	DADOS* dados = ((DADOS*)param);

	do {
		_tprintf(_T("\n>>"));
		_fgetts(op, TAM, stdin);
		op[_tcslen(op) - 1] = '\0';
		WaitForSingleObject(hMutex, INFINITE);
		if (!_tcscmp(op, TEXT("aumentaV"))) {		//AUMENTA 0.5 DE VELOCIDADE
			dados->taxi->velocidade += 0.5;
			_tprintf(_T("\n[COMANDO] Velocidade atual : %f"), dados->taxi->velocidade);
		}
		else if (!_tcscmp(op, TEXT("diminuiV"))) {		//DIMINUI 0.5 DE VELOCIDADE
			if (dados->taxi->velocidade >= 0.5)
				dados->taxi->velocidade -= 0.5;
			_tprintf(_T("\n[COMANDO] Velocidade atual : %f"), dados->taxi->velocidade);
		}
		else if (!_tcscmp(op, TEXT("numQuad"))) {		//DEFINIR NQ
			_tprintf(_T("\n[COMANDO] Valor de NQ (número de quadriculas até ao passageiro) : "));
			_tscanf_s(_T("%d"), &NQ);
			if (NQ < 0)
				NQ = NQ_INICIAL;
		}
		else if (!_tcscmp(op, TEXT("respostaAuto"))) {		//LIGAR/DESLIGAR RESPOSTA AUTOMÁTICA AOS PEDIDOS DE TRANSPORTE
			if (dados->taxi->autoResposta) {
				dados->taxi->autoResposta = 0;
				_tprintf(_T("\n[COMANDO] Desligada resposta automática a pedidos de transporte"));
			}
			else {
				dados->taxi->autoResposta = 1;
				_tprintf(_T("\n[COMANDO] Ligada resposta automática a pedidos de transporte"));
			}
		}
		else if (!_tcscmp(op, TEXT("pass"))) {		//TRANSPORTAR PASSAGEIRO
			if (!dados->taxi->autoResposta)
				dados->taxi->interessado = 1;
			_tprintf(_T("\n[COMANDO] Tenho interesse neste passageiro!"));
		}
		else if (!_tcscmp(op, TEXT("ajuda"))) {		//AJUDA NOS COMANDOS
			_tprintf(_T("\n[COMANDO] Aqui vai uma ajuda..."));
			ajuda();
		}
		if (_tcscmp(op, TEXT("fim")))
			ReleaseMutex(hMutex);
	} while (_tcscmp(op, TEXT("fim")));

	dados->taxi->terminar = 1;

	avisaTaxiSaiu(dados);
	ReleaseMutex(hMutex);

	Sleep(1000);

	ExitThread(0);
}

DWORD WINAPI ThreadMovimentaTaxi(LPVOID param) {	//MANDA TAXI AO ADMIN
	DADOS* dados = ((DADOS*)param);
	int val, valido;

	do {
		valido = 0;
		WaitForSingleObject(hMutex, INFINITE);
		if (dados->taxi->terminar)
			return 0;
		//MOVIMENTA
		//SEM PASSAGEIRO -> TENDÊNCIA PARA SEDESLOCAR PARA A FRENTE
		if (dados->taxi->velocidade != 0 && dados->taxi->disponivel == 1) {
			if (dados->mapa[tamanhoMapa * dados->taxi->Y + dados->taxi->Y + dados->taxi->X + 1].caracter == '_') {
				_tprintf(_T("\n[MOVIMENTO] (%d,%d) -> (%d,%d)"), dados->taxi->X, dados->taxi->Y, dados->taxi->X + 1, dados->taxi->Y);
				dados->taxi->X++;
			}
			else {
				do {
					val = rand() % 3;
					switch (val) {
					case 0: //CIMA
						if (dados->mapa[tamanhoMapa * (dados->taxi->Y - 1) + (dados->taxi->Y - 1) + dados->taxi->X].caracter == '_') {
							_tprintf(_T("\n[MOVIMENTO] (%d,%d) -> (%d,%d)"), dados->taxi->X, dados->taxi->Y, dados->taxi->X, dados->taxi->Y - 1);
							dados->taxi->Y--;
							valido = 1;
						}
						break;
					case 1: //BAIXO
						if (dados->mapa[tamanhoMapa * (dados->taxi->Y + 1) + (dados->taxi->Y + 1) + dados->taxi->X].caracter == '_') {
							_tprintf(_T("\n[MOVIMENTO] (%d,%d) -> (%d,%d)"), dados->taxi->X, dados->taxi->Y, dados->taxi->X, dados->taxi->Y + 1);
							dados->taxi->Y++;
							valido = 1;
						}
						break;
					case 2: //ESQUERDA
						if (dados->mapa[tamanhoMapa * dados->taxi->Y + dados->taxi->Y + dados->taxi->X - 1].caracter == '_') {
							_tprintf(_T("\n[MOVIMENTO] (%d,%d) -> (%d,%d)"), dados->taxi->X, dados->taxi->Y, dados->taxi->X - 1, dados->taxi->Y);
							dados->taxi->X--;
							valido = 1;
						}
						break;
					}
				} while (!valido);
			}
			avisaMovimentoTaxi(dados);
		}

		ReleaseMutex(hMutex);

		Sleep(1000);
	} while (!dados->taxi->terminar);

	ExitThread(0);
}

DWORD WINAPI ThreadSaiuAdmin(LPVOID param) { //NAMED PIPES
	DADOS* dados = ((DADOS*)param);

	WaitForSingleObject(dados->saiuAdmin, INFINITE);

	WaitForSingleObject(hMutex, INFINITE);

	dados->taxi->terminar = 1;

	ReleaseMutex(hMutex);
	_tprintf(_T("\n[AVISO] Administrador encerrou!"));

	Sleep(1000);

	ExitThread(0);
}

DWORD WINAPI ThreadRespostaTransporte(LPVOID param) {	//MANDA TAXI AO ADMIN E RECEBE PASSAGEIRO DO ADMIN -- BUFFER CIRCULAR
	TAXI* taxi = ((TAXI*)param);

	ExitThread(0);
}