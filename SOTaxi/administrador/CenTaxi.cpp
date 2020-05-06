#include "CenTaxi.h"


int _tmain(int argc, LPTSTR argv[]) {
	HANDLE hThreadComandos, hThreadNovoTaxi, hThreadSaiuTaxi, hThreadMovimento, hThreadNovoPassageiro;
	DADOS dados;
	dados.nTaxis = 0;
	dados.nPassageiros = 0;
	dados.terminar = 0;
	dados.aceitacaoT = 1;
	dados.esperaManifestacoes = TempoManifestacoes;

	if (argc == 3) {
		MaxPass = _wtoi(argv[1]);
		MaxTaxi = _wtoi(argv[2]);
	}
	_tprintf(TEXT("\nMaxPass : %d\nMaxTaxis : %d\n"), MaxPass, MaxTaxi);

#ifdef UNICODE
	_setmode(_fileno(stdin), _O_WTEXT);
	_setmode(_fileno(stdout), _O_WTEXT);
#endif

	Semaphore = CreateSemaphore(NULL, 1, 1, SEMAPHORE_NAME);
	if (Semaphore == NULL) {
		_tprintf(TEXT("CreateSemaphore failed.\n"));
		return 0;
	}
	_tprintf(TEXT("\nAguardando autorização para entrar...\n"));
	WaitForSingleObject(Semaphore, INFINITE);
	_tprintf(TEXT("\nEntrei!\n\n"));

	dados.hMutexDados = CreateMutex(NULL, FALSE, TEXT("MutexDados"));
	if (dados.hMutexDados == NULL) {
		_tprintf(TEXT("\n[ERRO] Erro ao criar Mutex!\n"));
		CloseHandle(Semaphore);
		return 0;
	}
	WaitForSingleObject(dados.hMutexDados, INFINITE);
	ReleaseMutex(dados.hMutexDados);

	dados.novoTaxi = CreateEvent(NULL, TRUE, FALSE, EVENT_NOVOT);
	if (dados.novoTaxi == NULL) {
		_tprintf(TEXT("\n[ERRO] Erro ao criar Evento!\n"));
		CloseHandle(Semaphore);
		return 0;
	}
	SetEvent(dados.novoTaxi);
	ResetEvent(dados.novoTaxi);

	dados.saiuTaxi = CreateEvent(NULL, TRUE, FALSE, EVENT_SAIUT);
	if (dados.saiuTaxi == NULL) {
		_tprintf(TEXT("\n[ERRO] Erro ao criar Evento!\n"));
		CloseHandle(Semaphore);
		CloseHandle(dados.novoTaxi);
		return 0;
	}
	SetEvent(dados.saiuTaxi);
	ResetEvent(dados.saiuTaxi);

	dados.movimentoTaxi = CreateEvent(NULL, TRUE, FALSE, EVENT_MOVIMENTO);
	if (dados.movimentoTaxi == NULL) {
		_tprintf(TEXT("\n[ERRO] Erro ao criar Evento!\n"));
		CloseHandle(Semaphore);
		CloseHandle(dados.novoTaxi);
		CloseHandle(dados.saiuTaxi);
		return 0;
	}
	SetEvent(dados.movimentoTaxi);
	ResetEvent(dados.movimentoTaxi);

	dados.respostaAdmin = CreateEvent(NULL, TRUE, FALSE, EVENT_RESPOSTA);
	if (dados.respostaAdmin == NULL) {
		_tprintf(TEXT("\n[ERRO] Erro ao criar Evento!\n"));
		CloseHandle(Semaphore);
		CloseHandle(dados.novoTaxi);
		CloseHandle(dados.saiuTaxi);
		CloseHandle(dados.movimentoTaxi);
		return 0;
	}

	dados.saiuAdmin = CreateEvent(NULL, TRUE, FALSE, EVENT_SAIUA);
	if (dados.saiuAdmin == NULL) {
		_tprintf(TEXT("\n[ERRO] Erro ao criar Evento!\n"));
		CloseHandle(Semaphore);
		CloseHandle(dados.novoTaxi);
		CloseHandle(dados.saiuTaxi);
		CloseHandle(dados.movimentoTaxi);
		CloseHandle(dados.respostaAdmin);
		return 0;
	}

	dados.EspTaxis = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(TAXI), SHM_TAXI);
	if (dados.EspTaxis == NULL)
	{
		_tprintf(TEXT("\n[ERRO] Erro ao criar FileMapping!\n"));
		CloseHandle(Semaphore);
		CloseHandle(dados.novoTaxi);
		CloseHandle(dados.saiuTaxi);
		CloseHandle(dados.movimentoTaxi);
		CloseHandle(dados.respostaAdmin);
		CloseHandle(dados.saiuAdmin);
		return 0;
	}

	dados.sharedTaxi = (TAXI*)MapViewOfFile(dados.EspTaxis, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(TAXI));
	if (dados.sharedTaxi == NULL)
	{
		_tprintf(TEXT("\n[ERRO] Erro em MapViewOfFile!\n"));
		CloseHandle(Semaphore);
		CloseHandle(dados.EspTaxis);
		CloseHandle(dados.novoTaxi);
		CloseHandle(dados.saiuTaxi);
		CloseHandle(dados.movimentoTaxi);
		CloseHandle(dados.respostaAdmin);
		CloseHandle(dados.saiuAdmin);
		return 0;
	}

	leMapa(&dados);

	hThreadComandos = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ThreadComandos, (LPVOID)&dados, 0, NULL);
	if (hThreadComandos == NULL) {
		_tprintf(TEXT("\n[ERRO] Erro ao lançar Thread!\n"));
		return 0;
	}
	hThreadNovoTaxi = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ThreadNovoTaxi, (LPVOID)&dados, 0, NULL);
	if (hThreadNovoTaxi == NULL) {
		_tprintf(TEXT("\n[ERRO] Erro ao lançar Thread!\n"));
		return 0;
	}
	hThreadSaiuTaxi = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ThreadSaiuTaxi, (LPVOID)&dados, 0, NULL);
	if (hThreadSaiuTaxi == NULL) {
		_tprintf(TEXT("\n[ERRO] Erro ao lançar Thread!\n"));
		return 0;
	}
	hThreadMovimento = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ThreadMovimento, (LPVOID)&dados, 0, NULL);
	if (hThreadMovimento == NULL) {
		_tprintf(TEXT("\n[ERRO] Erro ao lançar Thread!\n"));
		return 0;
	}
	//hThreadNovoPassageiro = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ThreadNovoPassageiro, (LPVOID)&dados, 0, NULL);
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
	WaitForMultipleObjects(4, ghEvents, FALSE, INFINITE);
	TerminateThread(hThreadNovoTaxi, 0);
	TerminateThread(hThreadSaiuTaxi, 0);
	TerminateThread(hThreadMovimento, 0);

	WaitForSingleObject(dados.hMutexDados, INFINITE);

	//NAMED PIPES
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

	ReleaseSemaphore(Semaphore, 1, NULL);

	UnmapViewOfFile(dados.sharedTaxi);
	CloseHandle(Semaphore);
	CloseHandle(dados.EspTaxis);
	CloseHandle(dados.novoTaxi);
	CloseHandle(dados.saiuTaxi);
	CloseHandle(dados.movimentoTaxi);
	CloseHandle(dados.respostaAdmin);
	CloseHandle(dados.hFile);
	CloseHandle(dados.atualizaMap);
	CloseHandle(dados.saiuAdmin);
	CloseHandle(dados.EspMapa);

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

void leMapa(DADOS* dados) {
	dados->hFile = CreateFile(PATH, GENERIC_WRITE | GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (dados->hFile == INVALID_HANDLE_VALUE)
	{
		_tprintf(TEXT("\n[ERRO] Erro ao Abrir Ficheiro!\n"));
		CloseHandle(Semaphore);
		CloseHandle(dados->EspTaxis);
		CloseHandle(dados->novoTaxi);
		CloseHandle(dados->saiuTaxi);
		CloseHandle(dados->movimentoTaxi);
		CloseHandle(dados->respostaAdmin);
		CloseHandle(dados->saiuAdmin);
		return;
	}

	dados->EspMapa = CreateFileMapping(dados->hFile, NULL, PAGE_READWRITE, 0, sizeof(dados->mapa), SHM_MAPA);
	if (dados->EspMapa == NULL)
	{
		_tprintf(TEXT("\n[ERRO] Erro ao criar FileMapping!\n"));
		CloseHandle(Semaphore);
		CloseHandle(dados->EspTaxis);
		CloseHandle(dados->novoTaxi);
		CloseHandle(dados->saiuTaxi);
		CloseHandle(dados->movimentoTaxi);
		CloseHandle(dados->respostaAdmin);
		CloseHandle(dados->saiuAdmin);
		CloseHandle(dados->hFile);
		return;
	}

	dados->sharedMapa = (MAPA*)MapViewOfFile(dados->EspMapa, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(dados->mapa));
	if (dados->sharedMapa == NULL)
	{
		_tprintf(TEXT("\n[ERRO] Erro em MapViewOfFile!\n"));
		CloseHandle(Semaphore);
		CloseHandle(dados->EspTaxis);
		CloseHandle(dados->novoTaxi);
		CloseHandle(dados->saiuTaxi);
		CloseHandle(dados->movimentoTaxi);
		CloseHandle(dados->respostaAdmin);
		CloseHandle(dados->saiuAdmin);
		CloseHandle(dados->hFile);
		CloseHandle(dados->EspMapa);
		return;
	}
	for (int i = 0; tamanhoMapa ==-1; i++)
		if (dados->sharedMapa[i].caracter == '\n')
			tamanhoMapa = i;

	dados->mapa = (MAPA*)malloc(sizeof(MAPA) * tamanhoMapa * tamanhoMapa);

	for (int i = 0; i < tamanhoMapa * tamanhoMapa; i++) {
		dados->mapa[i].caracter = dados->sharedMapa[i].caracter;
		_tprintf(TEXT("%c"), dados->mapa[i].caracter);
	}

	Sleep(1000);
	return;
}

DWORD WINAPI ThreadComandos(LPVOID param) {
	TCHAR op[TAM];
	DADOS* dados = ((DADOS*)param);

	do {
		_tprintf(_T("\n\n>>"));
		_fgetts(op, TAM, stdin);
		op[_tcslen(op) - 1] = '\0';
		WaitForSingleObject(dados->hMutexDados, INFINITE);
		if (_tcscmp(op, TEXT("expulsar"))) {		//EXPULSAR TAXI

		}
		else if (_tcscmp(op, TEXT("listar"))) {		//LISTAR TAXIS
			listarTaxis(dados);
		}
		else if (_tcscmp(op, TEXT("aceitacaoT"))) {		//PAUSAR/RECOMECAR ACEITAÇÃO DE TAXIS
			if (dados->aceitacaoT) {
				dados->aceitacaoT = 0;
				_tprintf(_T("\n[COMANDO] Pausar aceitação de Taxis"));
			}
			else {
				dados->aceitacaoT = 1;
				_tprintf(_T("\n[COMANDO] Recomeçar aceitação de Taxis"));
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
		WaitForSingleObject(dados->novoTaxi, INFINITE);

		if (dados->terminar)
			return 0;
		WaitForSingleObject(dados->hMutexDados, INFINITE);

		CopyMemory(&novo, dados->sharedTaxi, sizeof(TAXI));
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
		WaitForSingleObject(dados->saiuTaxi, INFINITE);

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

	dados->atualizaMap = CreateEvent(NULL, TRUE, FALSE, EVENT_ATUALIZAMAP);
	if (dados->atualizaMap == NULL) {
		_tprintf(TEXT("\n[ERRO] Erro ao criar Evento!\n"));
		return 0;
	}

	while (1) {
		WaitForSingleObject(dados->movimentoTaxi, INFINITE);

		if (dados->terminar)
			return 0;
		WaitForSingleObject(dados->hMutexDados, INFINITE);

		CopyMemory(&novo, dados->sharedTaxi, sizeof(TAXI));
		for (int i = 0; i < dados->nTaxis; i++)
			if (!_tcscmp(novo.matricula, dados->taxis[i].matricula)) {
				dados->taxis[i] = novo;
				_tprintf(_T("\n[MOVIMENTO] Taxi %s -> (%d,%d)"), novo.matricula, novo.X, novo.Y);
				char buf;
				buf = dados->taxis[i].id_mapa + '0';
				eliminaIdMapa(dados, buf);
				dados->mapa[tamanhoMapa * novo.X + novo.Y].caracter = buf;
			}

		CopyMemory(dados->sharedMapa, dados->mapa, sizeof(dados->mapa));
		_tprintf(TEXT("\n[MAPA] Mapa atualizado com sucesso!\n"));

		ReleaseMutex(dados->hMutexDados);

		SetEvent(dados->atualizaMap);
		Sleep(500);
		ResetEvent(dados->atualizaMap);

		Sleep(1000);
	}

	ExitThread(0);
}

DWORD WINAPI ThreadNovoPassageiro(LPVOID param) {		//VERIFICA SE HA NOVOS PASSAGEIROS
	DADOS* dados = ((DADOS*)param);

	ExitThread(0);
}

boolean adicionaTaxi(DADOS* dados, TAXI novo) {
	if (!dados->aceitacaoT)
		return FALSE;
	if (dados->nTaxis >= MaxTaxi)
		return FALSE;
	for (int i = 0; i < dados->nTaxis; i++)
		if (_tcscmp(novo.matricula, dados->taxis[i].matricula))
			return FALSE;

	dados->taxis[dados->nTaxis] = novo;
	dados->taxis[dados->nTaxis].id_mapa = id_mapa_taxi;
	dados->nTaxis++;
	id_mapa_taxi++;
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
	if (dados->nPassageiros >= MaxPass)
		return FALSE;

	dados->passageiros[dados->nPassageiros] = novo;
	dados->passageiros[dados->nPassageiros].id_mapa = id_mapa_pass;
	id_mapa_pass++;
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

void eliminaIdMapa(DADOS* dados, char id) {
	int x = 0, y = 0;
	for (int i = 0; i < tamanhoMapa * tamanhoMapa; i++) {
		if (dados->mapa[tamanhoMapa * y + x].caracter == id)
			dados->mapa[tamanhoMapa * y + x].caracter = '.';
		if (dados->mapa[tamanhoMapa * y + x].caracter == '\n') {
			x = 0;
			y++;
		}
		else {
			x++;
		}
	}
}