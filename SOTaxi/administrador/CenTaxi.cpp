#include "CenTaxi.h"

int _tmain(int argc, LPTSTR argv[]) {
	HANDLE hThreadComandos, hThreadNovoTaxi, hThreadSaiuTaxi, hThreadMovimento, hThreadNovoPassageiro, hThreadTempoTransporte;
	DADOS dados;
	dados.info = (INFO*)malloc(sizeof(INFO));
	dados.info->nTaxis = 0;
	dados.info->nPassageiros = 0;
	dados.terminar = 0;
	dados.aceitacaoT = 1;
	dados.esperaManifestacoes = TempoManifestacoes;

	srand((unsigned)time(NULL));

	HINSTANCE hLib;

	hLib = LoadLibrary(PATH_DLL);
	if (hLib == NULL)
		return 0;

#ifdef UNICODE
	_setmode(_fileno(stdin), _O_WTEXT);
	_setmode(_fileno(stdout), _O_WTEXT);
#endif

	ptr_register = (void(*)(TCHAR*, int))GetProcAddress(hLib, "dll_register");
	ptr_log = (void(*)(TCHAR*))GetProcAddress(hLib, "dll_log");

	Semaphore = CreateSemaphore(NULL, 1, 1, SEMAPHORE_NAME);
	if (Semaphore == NULL) {
		_tprintf(TEXT("CreateSemaphore failed.\n"));
		return 0;
	}
	ptr_register((TCHAR*)SEMAPHORE_NAME, 3);

	_tprintf(TEXT("\nAguardando autorização para entrar...\n"));
	WaitForSingleObject(Semaphore, INFINITE);
	_tprintf(TEXT("\nEntrei!\n\n"));

	inicializaBuffer();

	if (argc == 3) {
		MaxPass = _wtoi(argv[1]);
		MaxTaxi = _wtoi(argv[2]);
	}
	_tprintf(TEXT("\nMaxPass : %d\nMaxTaxis : %d\n"), MaxPass, MaxTaxi);

	dados.hMutexDados = CreateMutex(NULL, FALSE, NOME_MUTEX_DADOS);
	if (dados.hMutexDados == NULL) {
		_tprintf(TEXT("\n[ERRO] Erro ao criar Mutex!\n"));
		CloseHandle(Semaphore);
		return 0;
	}
	ptr_register((TCHAR*)NOME_MUTEX_DADOS, 1);
	WaitForSingleObject(dados.hMutexDados, INFINITE);
	ReleaseMutex(dados.hMutexDados);

	dados.novoTaxi = CreateEvent(NULL, TRUE, FALSE, EVENT_NOVOT);
	if (dados.novoTaxi == NULL) {
		_tprintf(TEXT("\n[ERRO] Erro ao criar Evento!\n"));
		CloseHandle(Semaphore);
		return 0;
	}
	ptr_register((TCHAR*)EVENT_NOVOT, 4);
	SetEvent(dados.novoTaxi);
	ResetEvent(dados.novoTaxi);

	dados.saiuTaxi = CreateEvent(NULL, TRUE, FALSE, EVENT_SAIUT);
	if (dados.saiuTaxi == NULL) {
		_tprintf(TEXT("\n[ERRO] Erro ao criar Evento!\n"));
		CloseHandle(Semaphore);
		CloseHandle(dados.novoTaxi);
		return 0;
	}
	ptr_register((TCHAR*)EVENT_SAIUT, 4);
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
	ptr_register((TCHAR*)EVENT_MOVIMENTO, 4);
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
	ptr_register((TCHAR*)EVENT_RESPOSTA, 4);

	dados.infoAdmin = CreateEvent(NULL, TRUE, FALSE, EVENT_INFOA);
	if (dados.infoAdmin == NULL) {
		_tprintf(TEXT("\n[ERRO] Erro ao criar Evento!\n"));
		CloseHandle(Semaphore);
		CloseHandle(dados.novoTaxi);
		CloseHandle(dados.saiuTaxi);
		CloseHandle(dados.movimentoTaxi);
		CloseHandle(dados.respostaAdmin);
		return 0;
	}
	ptr_register((TCHAR*)EVENT_INFOA, 4);

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
	ptr_register((TCHAR*)EVENT_SAIUA, 4);

	dados.EspTaxis = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(TAXI), SHM_TAXI);
	if (dados.EspTaxis == NULL)
	{
		_tprintf(TEXT("\n[ERRO] Erro ao criar FileMapping!\n"));
		CloseHandle(Semaphore);
		CloseHandle(dados.novoTaxi);
		CloseHandle(dados.saiuTaxi);
		CloseHandle(dados.movimentoTaxi);
		CloseHandle(dados.respostaAdmin);
		CloseHandle(dados.infoAdmin);
		return 0;
	}
	ptr_register((TCHAR*)SHM_TAXI, 6);

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
		CloseHandle(dados.infoAdmin);
		return 0;
	}
	ptr_register((TCHAR*)SHM_TAXI, 7);

	dados.novoPassageiro = CreateEvent(NULL, TRUE, FALSE, EVENT_NOVOP);
	if (dados.novoPassageiro == NULL) {
		_tprintf(TEXT("\n[ERRO] Erro ao criar Evento!\n"));
		CloseHandle(Semaphore);
		CloseHandle(dados.EspTaxis);
		CloseHandle(dados.novoTaxi);
		CloseHandle(dados.saiuTaxi);
		CloseHandle(dados.movimentoTaxi);
		CloseHandle(dados.respostaAdmin);
		UnmapViewOfFile(dados.sharedTaxi);
		return 0;
	}
	ptr_register((TCHAR*)EVENT_NOVOP, 4);
	SetEvent(dados.novoPassageiro);
	ResetEvent(dados.novoPassageiro);

	dados.respostaPass = CreateEvent(NULL, TRUE, FALSE, EVENT_RESPOSTAP);
	if (dados.respostaPass == NULL) {
		_tprintf(TEXT("\n[ERRO] Erro ao criar Evento!\n"));
		CloseHandle(Semaphore);
		CloseHandle(dados.EspTaxis);
		CloseHandle(dados.novoTaxi);
		CloseHandle(dados.saiuTaxi);
		CloseHandle(dados.movimentoTaxi);
		CloseHandle(dados.respostaAdmin);
		UnmapViewOfFile(dados.sharedTaxi);
		CloseHandle(dados.novoPassageiro);
		return 0;
	}
	ptr_register((TCHAR*)EVENT_RESPOSTAP, 4);

	dados.respostaMov = CreateEvent(NULL, TRUE, FALSE, EVENT_MOVIMENTOP);
	if (dados.respostaMov == NULL) {
		_tprintf(TEXT("\n[ERRO] Erro ao criar Evento!\n"));
		CloseHandle(Semaphore);
		CloseHandle(dados.EspTaxis);
		CloseHandle(dados.novoTaxi);
		CloseHandle(dados.saiuTaxi);
		CloseHandle(dados.movimentoTaxi);
		CloseHandle(dados.respostaAdmin);
		UnmapViewOfFile(dados.sharedTaxi);
		CloseHandle(dados.novoPassageiro);
		CloseHandle(dados.respostaPass);
		return 0;
	}
	ptr_register((TCHAR*)EVENT_MOVIMENTOP, 4);

	EspInfo = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(INFO), SHM_INFO);
	if (EspInfo == NULL)
	{
		_tprintf(TEXT("\n[ERRO] Erro ao criar FileMapping!\n"));
		return 0;
	}
	ptr_register((TCHAR*)SHM_INFO, 6);

	sharedInfo = (INFO*)MapViewOfFile(EspInfo, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(INFO));
	if (sharedInfo == NULL)
	{
		_tprintf(TEXT("\n[ERRO] Erro em MapViewOfFile!\n"));
		return 0;
	}
	ptr_register((TCHAR*)SHM_INFO, 7);

	hTimer = CreateWaitableTimer(NULL, TRUE, NULL);
	if (hTimer == NULL)
	{
		_tprintf(TEXT("CreateWaitableTimer failed (%d)\n"), GetLastError());
		return 0;
	}
	ptr_register((TCHAR*)TEXT("hTimer"), 5);

	transporte = CreateEvent(NULL, TRUE, FALSE, EVENT_TRANSPORTE);
	if (transporte == NULL) {
		_tprintf(TEXT("\n[ERRO] Erro ao criar Evento!\n"));
		return 0;
	}
	ptr_register((TCHAR*)EVENT_TRANSPORTE, 4);

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
	hThreadNovoPassageiro = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ThreadNovoPassageiro, (LPVOID)&dados, 0, NULL);
	if (hThreadNovoPassageiro == NULL) {
		_tprintf(TEXT("\nErro ao lançar Thread!\n"));
		return 0;
	}
	hThreadTempoTransporte = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ThreadTempoTransporte, (LPVOID)&dados, 0, NULL);
	if (hThreadTempoTransporte == NULL) {
		_tprintf(TEXT("\n[ERRO] Erro ao lançar Thread!\n"));
		return 0;
	}

	HANDLE ghEvents[6];
	ghEvents[0] = hThreadComandos;
	ghEvents[1] = hThreadNovoTaxi;
	ghEvents[2] = hThreadSaiuTaxi;
	ghEvents[3] = hThreadMovimento;
	ghEvents[4] = hThreadTempoTransporte;
	ghEvents[5] = hThreadNovoPassageiro;
	WaitForMultipleObjects(6, ghEvents, FALSE, INFINITE);

	//NAMED PIPES
	DWORD n;
	for (int i = 0; i < dados.info->nTaxis; i++) {
		dados.info->taxis[i].terminar = 1;
		WriteFile(pipeT[dados.info->taxis[i].id_mapa], (LPVOID)&dados.info->taxis[i], sizeof(TAXI), &n, NULL);
		ptr_log((TCHAR*)TEXT("CenTaxi envia Taxi por Named Pipe!"));
		SetEvent(dados.saiuAdmin);
		Sleep(2000);
		ResetEvent(dados.saiuAdmin);
	}
	for (int i = 0; i < dados.info->nPassageiros; i++)
		dados.info->passageiros[i].terminar = 1;
	if (dados.info->nPassageiros > 0) {
		WriteFile(hPipe, (LPVOID)&dados.info->passageiros[0], sizeof(PASSAGEIRO), &n, NULL);
		ptr_log((TCHAR*)TEXT("CenTaxi envia Passageiro por Named Pipe!"));
	}

	SetEvent(dados.respostaMov);
	Sleep(500);
	ResetEvent(dados.respostaMov);

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
	CloseHandle(dados.infoAdmin);
	CloseHandle(dados.EspMapa);
	CloseHandle(transporte);
	CloseHandle(dados.novoPassageiro);
	CloseHandle(dados.respostaPass);
	CloseHandle(dados.respostaMov);
	CloseHandle(dados.saiuAdmin);
	FreeLibrary(hLib);

	return 0;
}

void inicializaBuffer() {
	hMemoria = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(BUFFER), BUFFER_CIRCULAR);
	ptr_register((TCHAR*)BUFFER_CIRCULAR, 6);

	BufferMemoria = (BUFFER*)MapViewOfFile(hMemoria, FILE_MAP_WRITE, 0, 0, sizeof(BUFFER));
	ptr_register((TCHAR*)BUFFER_CIRCULAR, 7);

	sem_mutex = CreateSemaphore(NULL, 1, 1, SEMAPHORE_MUTEX);
	if (sem_mutex == NULL) {
		_tprintf(TEXT("\n[ERRO] Erro ao criar Sem�foro!\n"));
		return;
	}
	ptr_register((TCHAR*)SEMAPHORE_MUTEX, 3);

	sem_itens = CreateSemaphore(NULL, 0, MAX_PASS, SEMAPHORE_ITENS);
	if (sem_itens == NULL) {
		_tprintf(TEXT("\n[ERRO] Erro ao criar Sem�foro!\n"));
		return;
	}
	ptr_register((TCHAR*)SEMAPHORE_ITENS, 3);

	sem_vazios = CreateSemaphore(NULL, MAX_PASS, MAX_PASS, SEMAPHORE_VAZIOS);
	if (sem_vazios == NULL) {
		_tprintf(TEXT("\n[ERRO] Erro ao criar Sem�foro!\n"));
		return;
	}
	ptr_register((TCHAR*)SEMAPHORE_VAZIOS, 3);

	return;
}

void ajuda() {
	_tprintf(_T("\n\n novoP - ADICIONA PASSAGEIRO"));
	_tprintf(_T("\n mapa - VISUALIZA MAPA"));
	_tprintf(_T("\n expulsar - EXPULSAR TAXI"));
	_tprintf(_T("\n listarT - LISTAR TÁXIS"));
	_tprintf(_T("\n listarP - LISTAR PASSAGEIROS"));
	_tprintf(_T("\n aceitacaoT - PAUSAR/RECOMECAR ACEITAÇÃO DE TAXIS"));
	_tprintf(_T("\n manifestacoes - DEFINIR INTERVALO DE TEMPO DURANTE O QUAL AGUARDA MANIFESTAÇOES DOS TAXIS"));
	_tprintf(_T("\n fim - ENCERRAR TODO O SISTEMA"));
	return;
}

void listarTaxis(DADOS* dados) {
	if (dados->info->nTaxis == 0) {
		_tprintf(_T("\n[LISTAR TAXIS] Não há Taxis!"));
		return;
	}
	for (int i = 0; i < dados->info->nTaxis; i++) {
		_tprintf(_T("\n[LISTAR TAXIS] Taxi %s : "), dados->info->taxis[i].matricula);
		_tprintf(_T(" (%d, %d) "), dados->info->taxis[i].X, dados->info->taxis[i].Y);
		if (dados->info->taxis[i].disponivel)
			_tprintf(_T("sem passageiro!\n"));
		else
			_tprintf(_T("com passageiro!\n"));
	}
	return;
}

void listarPassageiros(DADOS* dados) {
	if (dados->info->nPassageiros == 0) {
		_tprintf(_T("\n[LISTAR PASSAGEIRO] Não há Passageiros!"));
		return;
	}
	for (int i = 0; i < dados->info->nPassageiros; i++) {
		_tprintf(_T("\n[LISTAR PASSAGEIRO] Passageiro %s : "), dados->info->passageiros[i].id);
		_tprintf(_T("\n (%d, %d) -> (%d, %d)"), dados->info->passageiros[i].X, dados->info->passageiros[i].Y, dados->info->passageiros[i].Xfinal, dados->info->passageiros[i].Yfinal);
		if (dados->info->passageiros[i].movimento)
			_tprintf(_T(" - em movimento\n"));
		else
			_tprintf(_T(" - à espera\n"));
	}
	return;
}

void verMapa(DADOS* dados) {
	_tprintf(TEXT("\n"));
	for (int i = 0; i < tamanhoMapa * tamanhoMapa; i++) {
		_tprintf(TEXT("%c"), dados->mapa[i].caracter);
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
		CloseHandle(dados->infoAdmin);
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
		CloseHandle(dados->infoAdmin);
		CloseHandle(dados->hFile);
		return;
	}
	ptr_register((TCHAR*)SHM_MAPA, 6);

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
		CloseHandle(dados->infoAdmin);
		CloseHandle(dados->hFile);
		CloseHandle(dados->EspMapa);
		return;
	}
	ptr_register((TCHAR*)SHM_MAPA, 7);

	for (int i = 0; tamanhoMapa == -1; i++)
		if (dados->sharedMapa[i].caracter == '\n')
			tamanhoMapa = i;

	dados->mapa = (MAPA*)malloc(sizeof(MAPA) * tamanhoMapa * tamanhoMapa);

	for (int i = 0; i < tamanhoMapa * tamanhoMapa; i++) {
		dados->mapa[i].caracter = dados->sharedMapa[i].caracter;
	}
	verMapa(dados);

	Sleep(1000);
	return;
}

void novoP(DADOS* dados) {
	PASSAGEIRO novo;
	TCHAR aux[TAM] = TEXT("\n");

	_tprintf(_T("\n[NOVO] Id do Passageiro: "));
	_fgetts(novo.id, TAM_ID, stdin);
	novo.id[_tcslen(novo.id) - 1] = '\0';

	_tprintf(_T("\n[NOVO]  Localizacao do Passageiro (X Y) : "));
	_tscanf_s(_T("%d"), &novo.X);
	_tscanf_s(_T("%d"), &novo.Y);

	_tprintf(_T("\n[NOVO]  Local de destino do Passageiro (X Y) : "));
	_tscanf_s(_T("%d"), &novo.Xfinal);
	_tscanf_s(_T("%d"), &novo.Yfinal);

	_stprintf_s(aux, TAM, TEXT("Passageiro %s em (%d,%d)!"), novo.id, novo.X, novo.Y);
	ptr_log(aux);
	ptr_log((TCHAR*)aux);

	novo.movimento = 0;
	novo.terminar = 0;
	novo.id_mapa = TEXT('.');
	novo.tempoEspera = -1;
	for (int i = 0; i < 6; i++)
		novo.matriculaTaxi[i] = ' ';
	novo.matriculaTaxi[6] = '\0';

	if (adicionaPassageiro(dados, novo))
		transportePassageiro(dados, (dados->info->nPassageiros-1));

	return;
}

void transportePassageiro(DADOS* dados, int indice) {
	TCHAR aux[TAM] = TEXT("\n");

	WaitForSingleObject(sem_vazios, INFINITE);
	WaitForSingleObject(sem_mutex, INFINITE);
	BufferMemoria->Passageiros[BufferMemoria->NextIn] = dados->info->passageiros[indice];
	BufferMemoria->NextIn = (BufferMemoria->NextIn + 1) % MAX_PASS;
	ptr_log((TCHAR*)TEXT("CenTaxi coloca passageiro em Buffer Circular!"));
	ReleaseSemaphore(sem_mutex, 1, NULL);
	ReleaseSemaphore(sem_itens, dados->info->nTaxis, NULL);

	TAXI novoT;
	acabouTempo = 0;
	int interessados = 0;
	Sleep(500);
	while (!acabouTempo) {
		SetEvent(transporte);
		Sleep(1000);
		ResetEvent(transporte);

		//RECEBE TODOS OS INTERESSES
		CopyMemory(&novoT, dados->sharedTaxi, sizeof(TAXI));
		ptr_log((TCHAR*)TEXT("CenTaxi recebe Taxi do ConTaxi por memória partilhada!"));
		if (novoT.interessado) {
			for (int i = 0; i < dados->info->nTaxis; i++)
				if (!_tcscmp(novoT.matricula, dados->info->taxis[i].matricula)) {
					dados->info->taxis[i] = novoT;
					interessados++;
				}
		}
	}
	DWORD n;

	if (interessados > 0) {
		int valor;
		do {
			valor = rand() % dados->info->nTaxis;
		} while (!dados->info->taxis[valor].interessado);
		_stprintf_s(aux, TAM, TEXT("Taxi %s vai transportar %s!"), dados->info->taxis[valor].matricula, dados->info->passageiros[indice].id);
		ptr_log(aux);
		ptr_log((TCHAR*)aux);
		dados->info->taxis[valor].Xfinal = dados->info->passageiros[indice].X;
		dados->info->taxis[valor].Yfinal = dados->info->passageiros[indice].Y;
		dados->info->taxis[valor].disponivel = 0;
		for (int i = 0; i < 6; i++)
			dados->info->passageiros[indice].matriculaTaxi[i] = dados->info->taxis[valor].matricula[i];

		//CALCULA TEMPO ESTIMADO DE ESPERA
		dados->info->passageiros[indice].tempoEspera = (int)(calculaDistancia(dados->info->taxis[valor].X, dados->info->taxis[valor].Y, dados->info->passageiros[indice].X, dados->info->passageiros[indice].Y) / dados->info->taxis[valor].velocidade);
		_tprintf(_T("\n[PASS]  O tempo estimado de espera para este passageiro é %d s"), dados->info->passageiros[indice].tempoEspera);

		WriteFile(pipeT[dados->info->taxis[valor].id_mapa], (LPVOID)&dados->info->taxis[valor], sizeof(TAXI), &n, NULL);
		ptr_log((TCHAR*)TEXT("CenTaxi envia Taxi por Named Pipe!"));
		SetEvent(dados->respostaAdmin);
		Sleep(500);
		ResetEvent(dados->respostaAdmin);

	}
	else
		_tprintf(_T("\n[PASS]  Não houve interesse de nenhum Taxi o transporte de '%s'!"), dados->info->passageiros[indice].id);
	transporteAceite(dados);

	return;
}

boolean adicionaTaxi(DADOS* dados, TAXI novo) {
	TCHAR aux[TAM];
	if (!dados->aceitacaoT)
		return FALSE;
	if (dados->info->nTaxis >= MaxTaxi)
		return FALSE;
	for (int i = 0; i < dados->info->nTaxis; i++)
		if (!_tcscmp(novo.matricula, dados->info->taxis[i].matricula))
			return FALSE;

	dados->info->taxis[dados->info->nTaxis] = novo;
	dados->info->taxis[dados->info->nTaxis].id_mapa = numPipes;
	dados->info->nTaxis++;
	_tprintf(TEXT("[NOVO TAXI] Novo Taxi: %s\n"), novo.matricula);
	_stprintf_s(aux, TAM, TEXT("Taxi %s entrou!"), novo.matricula);
	ptr_log(aux);
	ptr_log((TCHAR*)aux);

	char buf;
	buf = dados->info->taxis[dados->info->nTaxis-1].id_mapa + '0';
	dados->mapa[tamanhoMapa * novo.Y + novo.Y + novo.X].caracter = buf;

	for (int i = 0; i < tamanhoMapa * tamanhoMapa; i++)
		dados->sharedMapa[i].caracter = dados->mapa[i].caracter;
	CopyMemory(dados->sharedMapa, dados->mapa, sizeof(dados->mapa));
	ptr_log((TCHAR*)TEXT("CenTaxi envia Mapa para MapInfo por memória partilhada!"));
	CopyMemory(sharedInfo, dados->info, sizeof(INFO));
	ptr_log((TCHAR*)TEXT("CenTaxi envia Info para MapInfo por memória partilhada!"));
	_tprintf(TEXT("\n[MAPA] Mapa atualizado com sucesso!\n"));

	SetEvent(dados->atualizaMap);
	Sleep(500);
	ResetEvent(dados->atualizaMap);
	return TRUE;
}

boolean removeTaxi(DADOS* dados, TAXI novo) {
	TCHAR aux[TAM];
	for (int i = 0; i < dados->info->nTaxis; i++) {
		if (!_tcscmp(novo.matricula, dados->info->taxis[i].matricula)) {
			for (int k = i; k < dados->info->nTaxis - 1; k++) {
				dados->info->taxis[k] = dados->info->taxis[k + 1];
			}
			dados->info->nTaxis--;
			_tprintf(TEXT("[SAIU TAXI] Saiu Taxi: %s\n"), novo.matricula);
			_stprintf_s(aux, TAM, TEXT("Taxi %s saiu!"), novo.matricula);
			ptr_log(aux);
			ptr_log((TCHAR*)aux);

			char buf;
			buf = dados->info->taxis[i].id_mapa + '0';
			eliminaIdMapa(dados, buf);
			for (int i = 0; i < tamanhoMapa * tamanhoMapa; i++)
				dados->sharedMapa[i].caracter = dados->mapa[i].caracter;
			CopyMemory(dados->sharedMapa, dados->mapa, sizeof(dados->mapa));
			ptr_log((TCHAR*)TEXT("CenTaxi envia Mapa para MapInfo por memória partilhada!"));
			CopyMemory(sharedInfo, dados->info, sizeof(INFO));
			ptr_log((TCHAR*)TEXT("CenTaxi envia Info para MapInfo por memória partilhada!"));
			_tprintf(TEXT("\n[MAPA] Mapa atualizado com sucesso!\n"));

			SetEvent(dados->atualizaMap);
			Sleep(500);
			ResetEvent(dados->atualizaMap);
			return TRUE;
		}
	}
	return FALSE;
}

boolean adicionaPassageiro(DADOS* dados, PASSAGEIRO novo) {
	TCHAR aux[TAM];
	if (dados->info->nPassageiros >= MaxPass)
		return FALSE;

	dados->info->passageiros[dados->info->nPassageiros] = novo;
	dados->info->passageiros[dados->info->nPassageiros].id_mapa = id_mapa_pass;
	id_mapa_pass++;
	dados->info->nPassageiros++;
	_tprintf(TEXT("[NOVO PASSAGEIRO] Novo Passageiro: %s\n"), novo.id);
	_stprintf_s(aux, TAM, TEXT("Passageiro %s entrou no sistema!"), novo.id);
	ptr_log(aux);
	ptr_log((TCHAR*)aux);
	deslocaPassageiroParaPorta(dados);

	dados->mapa[tamanhoMapa * dados->info->passageiros[dados->info->nPassageiros-1].Y + dados->info->passageiros[dados->info->nPassageiros-1].Y + dados->info->passageiros[dados->info->nPassageiros-1].X].caracter = (char)dados->info->passageiros[dados->info->nPassageiros - 1].id_mapa;
	for (int i = 0; i < tamanhoMapa * tamanhoMapa; i++)
		dados->sharedMapa[i].caracter = dados->mapa[i].caracter;
	CopyMemory(dados->sharedMapa, dados->mapa, sizeof(dados->mapa));
	ptr_log((TCHAR*)TEXT("CenTaxi envia Mapa para MapInfo por memória partilhada!"));
	CopyMemory(sharedInfo, dados->info, sizeof(INFO));
	ptr_log((TCHAR*)TEXT("CenTaxi envia Info para MapInfo por memória partilhada!"));
	_tprintf(TEXT("\n[MAPA] Mapa atualizado com sucesso!\n"));

	SetEvent(dados->atualizaMap);
	Sleep(500);
	ResetEvent(dados->atualizaMap);
	return TRUE;
}

boolean removePassageiro(DADOS* dados, PASSAGEIRO novo) {
	TCHAR aux[TAM];
	for (int i = 0; i < dados->info->nPassageiros; i++) {
		if (!_tcscmp(novo.id, dados->info->passageiros[i].id)) {
			for (int k = i; k < dados->info->nPassageiros - 1; k++) {
				dados->info->passageiros[k] = dados->info->passageiros[k + 1];
			}
			dados->info->nPassageiros--;
			_tprintf(TEXT("[SAIU PASSAGEIRO] Saiu Passageiro: %s\n"), novo.id);
			_stprintf_s(aux, TAM, TEXT("Passageiro %s saiu!"), novo.id);
			ptr_log(aux);
			ptr_log((TCHAR*)aux);

			eliminaIdMapa(dados, novo.id_mapa);
			for (int i = 0; i < tamanhoMapa * tamanhoMapa; i++)
				dados->sharedMapa[i].caracter = dados->mapa[i].caracter;
			CopyMemory(dados->sharedMapa, dados->mapa, sizeof(dados->mapa));
			ptr_log((TCHAR*)TEXT("CenTaxi envia Mapa para MapInfo por memória partilhada!"));
			CopyMemory(sharedInfo, dados->info, sizeof(INFO));
			ptr_log((TCHAR*)TEXT("CenTaxi envia Info para MapInfo por memória partilhada!"));
			_tprintf(TEXT("\n[MAPA] Mapa atualizado com sucesso!\n"));

			SetEvent(dados->atualizaMap);
			Sleep(500);
			ResetEvent(dados->atualizaMap);
			return TRUE;
		}
	}
	return FALSE;
}

void eliminaIdMapa(DADOS* dados, char id) {
	int x = 0, y = 0;
	for (int i = 0; i < tamanhoMapa * tamanhoMapa; i++) {
		if (dados->mapa[i].caracter == id)
			dados->mapa[i].caracter = '_';
	}
}

void expulsarTaxi(DADOS* dados, TCHAR* matr) {
	int i;

	for (i = 0; i < dados->info->nTaxis && _tcscmp(matr, dados->info->taxis[i].matricula); i++);
	if (!_tcscmp(matr, dados->info->taxis[i].matricula)) {
		dados->info->taxis[i].terminar = 1;
		enviaTaxi(dados, &dados->info->taxis[i]);
		Sleep(1000);
		removeTaxi(dados, dados->info->taxis[i]);
	}
	return;
}

void transporteAceite(DADOS* dados) {
	BufferMemoria->NextOut = (BufferMemoria->NextOut + 1) % MAX_PASS;
	ptr_log((TCHAR*)TEXT("CenTaxi retira passageiro de Buffer Circular!"));
	ReleaseSemaphore(sem_vazios, 1, NULL);

	return;
}

void enviaTaxi(DADOS* dados, TAXI* taxi) {
	CopyMemory(dados->sharedTaxi, taxi, sizeof(TAXI));
	ptr_log((TCHAR*)TEXT("CenTaxi envia Taxi para ConTaxi por memória partilhada!"));
	SetEvent(dados->infoAdmin);
	Sleep(500);
	ResetEvent(dados->infoAdmin);

	return;
}

void deslocaPassageiroParaPorta(DADOS* dados) {
	TCHAR aux[TAM] = TEXT("\n");
	int VALIDO = 0;

	if (dados->mapa[tamanhoMapa * dados->info->passageiros[dados->info->nPassageiros - 1].Y + dados->info->passageiros[dados->info->nPassageiros - 1].Y + dados->info->passageiros[dados->info->nPassageiros - 1].X].caracter != '_') {
		for (int i = 1; VALIDO != 1; i++) {
			if (dados->mapa[tamanhoMapa * dados->info->passageiros[dados->info->nPassageiros - 1].Y + dados->info->passageiros[dados->info->nPassageiros - 1].Y + dados->info->passageiros[dados->info->nPassageiros - 1].X + i].caracter == '_') {
				dados->info->passageiros[dados->info->nPassageiros - 1].X += i;
				VALIDO = 1;
			}
			else if (dados->mapa[tamanhoMapa * (dados->info->passageiros[dados->info->nPassageiros - 1].Y - i) + (dados->info->passageiros[dados->info->nPassageiros - 1].Y - i) + dados->info->passageiros[dados->info->nPassageiros - 1].X].caracter == '_') {
				dados->info->passageiros[dados->info->nPassageiros - 1].Y -= i;
				VALIDO = 1;
			}
			else if (dados->mapa[tamanhoMapa * (dados->info->passageiros[dados->info->nPassageiros - 1].Y + i) + (dados->info->passageiros[dados->info->nPassageiros - 1].Y + i) + dados->info->passageiros[dados->info->nPassageiros - 1].X].caracter == '_') {
				dados->info->passageiros[dados->info->nPassageiros - 1].Y += i;
				VALIDO = 1;
			}
			else if (dados->mapa[tamanhoMapa * dados->info->passageiros[dados->info->nPassageiros - 1].Y + dados->info->passageiros[dados->info->nPassageiros - 1].Y + dados->info->passageiros[dados->info->nPassageiros - 1].X - i].caracter == '_') {
				dados->info->passageiros[dados->info->nPassageiros - 1].X -= i;
				VALIDO = 1;
			}
		}
		_stprintf_s(aux, TAM, TEXT("Passageiro %s deslocado para (%d,%d)!"), dados->info->passageiros[dados->info->nPassageiros - 1].id, dados->info->passageiros[dados->info->nPassageiros - 1].X, dados->info->passageiros[dados->info->nPassageiros - 1].Y);
		ptr_log(aux);
		ptr_log((TCHAR*)aux);
		_tprintf(TEXT("Passageiro %s deslocado para (%d,%d)!"), dados->info->passageiros[dados->info->nPassageiros - 1].id, dados->info->passageiros[dados->info->nPassageiros - 1].X, dados->info->passageiros[dados->info->nPassageiros - 1].Y);
	}
	if (dados->mapa[tamanhoMapa * dados->info->passageiros[dados->info->nPassageiros - 1].Yfinal + dados->info->passageiros[dados->info->nPassageiros - 1].Yfinal + dados->info->passageiros[dados->info->nPassageiros - 1].Xfinal].caracter != '_') {
		for (int i = 1; VALIDO != 1; i++) {
			if (dados->mapa[tamanhoMapa * dados->info->passageiros[dados->info->nPassageiros - 1].Yfinal + dados->info->passageiros[dados->info->nPassageiros - 1].Yfinal + dados->info->passageiros[dados->info->nPassageiros - 1].Xfinal + i].caracter == '_') {
				dados->info->passageiros[dados->info->nPassageiros - 1].Xfinal += i;
				VALIDO = 1;
			}
			else if (dados->mapa[tamanhoMapa * (dados->info->passageiros[dados->info->nPassageiros - 1].Yfinal - i) + (dados->info->passageiros[dados->info->nPassageiros - 1].Yfinal - i) + dados->info->passageiros[dados->info->nPassageiros - 1].Xfinal].caracter == '_') {
				dados->info->passageiros[dados->info->nPassageiros - 1].Yfinal -= i;
				VALIDO = 1;
			}
			else if (dados->mapa[tamanhoMapa * (dados->info->passageiros[dados->info->nPassageiros - 1].Yfinal + i) + (dados->info->passageiros[dados->info->nPassageiros - 1].Yfinal + i) + dados->info->passageiros[dados->info->nPassageiros - 1].Xfinal].caracter == '_') {
				dados->info->passageiros[dados->info->nPassageiros - 1].Yfinal += i;
				VALIDO = 1;
			}
			else if (dados->mapa[tamanhoMapa * dados->info->passageiros[dados->info->nPassageiros - 1].Yfinal + dados->info->passageiros[dados->info->nPassageiros - 1].Yfinal + dados->info->passageiros[dados->info->nPassageiros - 1].Xfinal - i].caracter == '_') {
				dados->info->passageiros[dados->info->nPassageiros - 1].Xfinal -= i;
				VALIDO = 1;
			}
		}
	}
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

DWORD WINAPI ThreadTempoTransporte(LPVOID param) {
	DADOS* dados = ((DADOS*)param);
	LARGE_INTEGER liDueTime;
	liDueTime.QuadPart = -10000000LL * dados->esperaManifestacoes;

	while (1) {
		WaitForSingleObject(transporte, INFINITE);

		if (dados->terminar)
			break;

		if (!SetWaitableTimer(hTimer, &liDueTime, 0, NULL, NULL, 0))
		{
			_tprintf(_T("SetWaitableTimer failed (%d)\n"), GetLastError());
			return 0;
		}

		WaitForSingleObject(hTimer, INFINITE);
		acabouTempo = 1;

	}
	ExitThread(0);
}

DWORD WINAPI ThreadComandos(LPVOID param) {
	TCHAR op[TAM], matr[7], i;
	DADOS* dados = ((DADOS*)param);

	do {
		_tprintf(_T("\n\n"));
		i = _gettch();
		WaitForSingleObject(dados->hMutexDados, INFINITE);
		_tprintf(_T("%c"), i);
		op[0] = i;
		_fgetts(&op[1], sizeof(op), stdin);
		op[_tcslen(op) - 1] = '\0';
		//NOVO PASSAGEIRO
		if (!_tcscmp(op, TEXT("novoP"))) {
			novoP(dados);
		}
		//VER MAPA
		else if (!_tcscmp(op, TEXT("mapa"))) {
			verMapa(dados);
		}
		//EXPULSAR TAXI
		else if (!_tcscmp(op, TEXT("expulsar"))) {
			_tprintf(_T("\n Matricula do Táxi: "));
			_fgetts(matr, sizeof(matr), stdin);
			matr[_tcslen(matr) - 1] = '\0';
			expulsarTaxi(dados, matr);
		}
		//LISTAR TAXIS
		else if (!_tcscmp(op, TEXT("listarT"))) {
			listarTaxis(dados);
		}
		//LISTAR PASSAGEIROS
		else if (!_tcscmp(op, TEXT("listarP"))) {
			listarPassageiros(dados);
		}
		//PAUSAR/RECOMECAR ACEITAÇÃO DE TAXIS
		else if (!_tcscmp(op, TEXT("aceitacaoT"))) {
			if (dados->aceitacaoT) {
				dados->aceitacaoT = 0;
				_tprintf(_T("\n[COMANDO] Pausar aceitação de Taxis"));
			}
			else {
				dados->aceitacaoT = 1;
				_tprintf(_T("\n[COMANDO] Recomeçar aceitação de Taxis"));
			}
		}
		//DEFINIR INTERVALO DE TEMPO DURANTE O QUAL AGUARDA MANIFESTAÇOES DOS TAXIS
		else if (!_tcscmp(op, TEXT("manifestacoes"))) {
			_tprintf(_T("\n[COMANDO] Intervalo de tempo durante o qual aguarda manifestações (em segundos): "));
			_tscanf_s(_T("%d"), &dados->esperaManifestacoes);
			if (dados->esperaManifestacoes <= 0)
				dados->esperaManifestacoes = TempoManifestacoes;
		}
		//AJUDA NOS COMANDOS
		else if (!_tcscmp(op, TEXT("ajuda"))) {
			ajuda();
		}
		if (_tcscmp(op, TEXT("fim")))
			ReleaseMutex(dados->hMutexDados);
		_tprintf(_T("\n\n"));
	} while (_tcscmp(op, TEXT("fim")));

	dados->terminar = 1;

	ReleaseMutex(dados->hMutexDados);

	ExitThread(0);
}

//VERIFICA SE HA NOVOS TAXIS
DWORD WINAPI ThreadNovoTaxi(LPVOID param) {
	DADOS* dados = ((DADOS*)param);
	TAXI novo;
	TCHAR aux[TAM] = TEXT("\n"), aux1[TAM] = TEXT("\n");

	while (1) {
		WaitForSingleObject(dados->novoTaxi, INFINITE);

		if (dados->terminar)
			return 0;
		WaitForSingleObject(dados->hMutexDados, INFINITE);

		CopyMemory(&novo, dados->sharedTaxi, sizeof(TAXI));
		ptr_log((TCHAR*)TEXT("CenTaxi recebe Taxi do ConTaxi por memória partilhada!"));
		if (adicionaTaxi(dados, novo)) {
			_tprintf(TEXT("Novo Taxi: %s\n"), dados->info->taxis[dados->info->nTaxis - 1].matricula);
			CopyMemory(dados->sharedTaxi, &dados->info->taxis[dados->info->nTaxis - 1], sizeof(TAXI));
			_stprintf_s(aux, TAM, TEXT("Taxi %s entrou!\n"), novo.matricula);
			ptr_log(aux);
			_stprintf_s(aux1, TAM, TEXT("Taxi %s em (%d,%d) vazio!"), novo.matricula, novo.X, novo.Y);
			ptr_log(aux1);
		}
		else {
			novo.terminar = 1;
			CopyMemory(dados->sharedTaxi, &novo, sizeof(TAXI));
		}
		ptr_log((TCHAR*)TEXT("CenTaxi envia Taxi para ConTaxi por memória partilhada!"));
		SetEvent(dados->respostaAdmin);
		Sleep(500);
		ResetEvent(dados->respostaAdmin);

		if (!novo.terminar) {
			TCHAR PIPE[200];
			_stprintf_s(PIPE, sizeof(TEXT("\\\\.\\pipe\\taxi%d")), TEXT("\\\\.\\pipe\\taxi%d"), dados->info->taxis[dados->info->nTaxis - 1].id_mapa);
			pipeT[numPipes] = CreateNamedPipe(PIPE, PIPE_ACCESS_OUTBOUND, PIPE_WAIT | PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE, 1, sizeof(TAXI), sizeof(TAXI), 1000, NULL);
			if (pipeT[numPipes] == INVALID_HANDLE_VALUE) {
				_tprintf(TEXT("[ERRO] Criar Named Pipe! %d (CreateNamedPipe)"), GetLastError());
				return 0;
			}
			ptr_register((TCHAR*)PIPE, 8);
			if (!ConnectNamedPipe(pipeT[numPipes], NULL)) {
				_tprintf(TEXT("[ERRO] Ligação ao Táxi! %d (ConnectNamedPipe\n"), GetLastError());
				return 0;
			}
			numPipes++; 
		}

		ReleaseMutex(dados->hMutexDados);

		Sleep(1000);
	}

	ExitThread(0);
}

//VERIFICA SE SAIRAM TAXIS
DWORD WINAPI ThreadSaiuTaxi(LPVOID param) {
	DADOS* dados = ((DADOS*)param);
	TAXI novo;
	TCHAR aux[TAM] = TEXT("\n");

	while (1) {
		WaitForSingleObject(dados->saiuTaxi, INFINITE);

		if (dados->terminar)
			return 0;
		WaitForSingleObject(dados->hMutexDados, INFINITE);

		CopyMemory(&novo, dados->sharedTaxi, sizeof(TAXI));
		_stprintf_s(aux, TAM, TEXT("Taxi %s saiu!"), novo.matricula);
		ptr_log(aux);
		ptr_log((TCHAR*)TEXT("CenTaxi recebe Taxi do ConTaxi por memória partilhada!"));
		removeTaxi(dados, novo);

		ReleaseMutex(dados->hMutexDados);

		Sleep(1000);
	}

	ExitThread(0);
}

DWORD WINAPI ThreadMovimento(LPVOID param) {
	DADOS* dados = ((DADOS*)param);
	TAXI novo;
	int VALIDO;
	DWORD n;
	int num = 10;

	dados->atualizaMap = CreateEvent(NULL, TRUE, FALSE, EVENT_ATUALIZAMAP);
	if (dados->atualizaMap == NULL) {
		_tprintf(TEXT("\n[ERRO] Erro ao criar Evento!\n"));
		return 0;
	}

	while (1) {
		num--;
		TCHAR aux[TAM] = TEXT("\n");
		WaitForSingleObject(dados->movimentoTaxi, INFINITE);

		if (dados->terminar)
			return 0;
		WaitForSingleObject(dados->hMutexDados, INFINITE);

		CopyMemory(&novo, dados->sharedTaxi, sizeof(TAXI));
		ptr_log((TCHAR*)TEXT("CenTaxi recebe Taxi do ConTaxi por memória partilhada!"));
		for (int i = 0; i < dados->info->nTaxis; i++) {
			if (!_tcscmp(novo.matricula, dados->info->taxis[i].matricula)) {
				dados->info->taxis[i] = novo;
				_tprintf(_T("\n[MOVIMENTO] Taxi %s -> (%d,%d)"), novo.matricula, novo.X, novo.Y);
				char buf;
				buf = dados->info->taxis[i].id_mapa + '0';
				eliminaIdMapa(dados, buf);
				dados->mapa[tamanhoMapa * novo.Y + novo.Y + novo.X].caracter = buf;
				//SE TIVER PASSAGEIRO VERIFICA SE CHEGOU AO DESTINO
				if (!dados->info->taxis[i].disponivel) {
					VALIDO = 0;
					for (int j = 0; j < dados->info->nPassageiros && VALIDO == 0; j++) {
						//SE TAXI CHEGOU AO SITIO DE RECOLHA DE PASSAGEIRO
						if (!_tcscmp(dados->info->passageiros[j].matriculaTaxi, dados->info->taxis[i].matricula) && dados->info->passageiros[j].X == dados->info->taxis[i].X && dados->info->passageiros[j].Y == dados->info->taxis[i].Y) {
							_tprintf(_T("\n[MOVIMENTO] Taxi '%s' apanhou o Passageiro '%s'\n"), dados->info->taxis[i].matricula, dados->info->passageiros[j].id);
							_stprintf_s(aux, TAM, TEXT("Passageiro %s a ser transportado!"), dados->info->passageiros[j].id);
							ptr_log(aux);
							ptr_log((TCHAR*)aux);
							dados->info->taxis[i].Xfinal = dados->info->passageiros[j].Xfinal;
							dados->info->taxis[i].Yfinal = dados->info->passageiros[j].Yfinal;
							//AVISA O TÁXI
							enviaTaxi(dados, &dados->info->taxis[i]);
							Sleep(1000);
							VALIDO = 1;

							dados->info->passageiros[j].movimento = 1;
							dados->info->passageiros[j].tempoEspera = -1;

							WriteFile(hPipe, (LPVOID)&dados->info->passageiros[j], sizeof(PASSAGEIRO), &n, NULL);
							SetEvent(dados->respostaMov);
							Sleep(500);
							ResetEvent(dados->respostaMov);
						}
						//SE TAXI CHEGOU AO SITIO DE DESTINO DO PASSAGEIRO
						if (!_tcscmp(dados->info->passageiros[j].matriculaTaxi, dados->info->taxis[i].matricula) && dados->info->passageiros[j].Xfinal == dados->info->taxis[i].X && dados->info->passageiros[j].Yfinal == dados->info->taxis[i].Y) {
							_tprintf(_T("\n[MOVIMENTO] Taxi '%s' deixou Passageiro '%s'\n"), dados->info->taxis[i].matricula, dados->info->passageiros[j].id);
							_stprintf_s(aux, TAM, TEXT("Passageiro %s entregue!"), dados->info->passageiros[j].id);
							ptr_log(aux);
							ptr_log((TCHAR*)aux);
							dados->info->taxis[i].disponivel = 1;
							dados->info->taxis[i].Xfinal = 0;
							dados->info->taxis[i].Yfinal = 0;
							//AVISA O TÁXI
							enviaTaxi(dados, &dados->info->taxis[i]);
							Sleep(1000);
							VALIDO = 1;

							dados->info->passageiros[j].movimento = 0;
							dados->info->passageiros[j].tempoEspera = -1;
							for (int i = 0; i < 6; i++)
								dados->info->passageiros[j].matriculaTaxi[i] = ' ';
							dados->info->passageiros[j].matriculaTaxi[6] = '\0';

							WriteFile(hPipe, (LPVOID)&dados->info->passageiros[j], sizeof(PASSAGEIRO), &n, NULL);
							SetEvent(dados->respostaMov);
							Sleep(500);
							ResetEvent(dados->respostaMov);

							removePassageiro(dados, dados->info->passageiros[j]);
						}
					}
				}
				for (int i = 0; i < tamanhoMapa * tamanhoMapa; i++)
					dados->sharedMapa[i].caracter = dados->mapa[i].caracter;
				CopyMemory(dados->sharedMapa, dados->mapa, sizeof(dados->mapa));
				ptr_log((TCHAR*)TEXT("CenTaxi envia Mapa para MapInfo por memória partilhada!"));
				CopyMemory(sharedInfo, dados->info, sizeof(INFO));
				ptr_log((TCHAR*)TEXT("CenTaxi envia Info para MapInfo por memória partilhada!"));
				_tprintf(TEXT("\n[MAPA] Mapa atualizado com sucesso!\n"));
			}
		}

		ReleaseMutex(dados->hMutexDados);

		SetEvent(dados->atualizaMap);
		Sleep(500);
		ResetEvent(dados->atualizaMap);

		if (num == 0) {
			for (int i = 0; i < dados->info->nPassageiros; i++) {
				if (dados->info->passageiros[i].tempoEspera == -1) {
					transportePassageiro(dados, i);

					WriteFile(hPipe, (LPVOID)&dados->info->passageiros[dados->info->nPassageiros - 1], sizeof(PASSAGEIRO), &n, NULL);
					ptr_log((TCHAR*)TEXT("CenTaxi envia Passageiro por Named Pipe!"));
					SetEvent(dados->respostaPass);
					Sleep(500);
					ResetEvent(dados->respostaPass);
				}
			}
			num = 10;
		}

		Sleep(500);
	}

	ExitThread(0);
}

//VERIFICA SE HA NOVOS PASSAGEIROS
DWORD WINAPI ThreadNovoPassageiro(LPVOID param) {
	DADOS* dados = ((DADOS*)param);
	TCHAR aux[TAM] = TEXT("\n"), aux1[TAM] = TEXT("\n");
	PASSAGEIRO novo;
	DWORD n;

	hPipe = CreateNamedPipe(PIPE_NAME, PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED, PIPE_WAIT | PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE, 1, sizeof(PASSAGEIRO), sizeof(PASSAGEIRO), 1000, NULL);
	if (hPipe == INVALID_HANDLE_VALUE) {
		_tprintf(TEXT("[ERRO] Criar Named Pipe! %d (CreateNamedPipe)"), GetLastError());
		return 0;
	}
	ptr_register((TCHAR*)PIPE_NAME, 8);
	if (!ConnectNamedPipe(hPipe, NULL)) {
		_tprintf(TEXT("[ERRO] Ligação ao leitor! %d (ConnectNamedPipe\n"), GetLastError());
		return 0;
	}

	while (1) {
		WaitForSingleObject(dados->novoPassageiro, INFINITE);

		if (dados->terminar)
			return 0;
		WaitForSingleObject(dados->hMutexDados, INFINITE);

		ReadFile(hPipe, (LPVOID)&novo, sizeof(PASSAGEIRO), &n, NULL);
		ptr_log((TCHAR*)TEXT("CenTaxi recebe Passageiro por Named Pipe!"));
		if (!adicionaPassageiro(dados, novo)) {
			novo.terminar = 1;
			WriteFile(hPipe, (LPVOID)&novo, sizeof(PASSAGEIRO), &n, NULL);
			ptr_log((TCHAR*)TEXT("CenTaxi envia Passageiro por Named Pipe!"));
			SetEvent(dados->respostaPass);
			Sleep(500);
			ResetEvent(dados->respostaPass);
		}
		else {
			transportePassageiro(dados, (dados->info->nPassageiros - 1));

			WriteFile(hPipe, (LPVOID)&dados->info->passageiros[dados->info->nPassageiros - 1], sizeof(PASSAGEIRO), &n, NULL);
			ptr_log((TCHAR*)TEXT("CenTaxi envia Passageiro por Named Pipe!"));
			SetEvent(dados->respostaPass);
			Sleep(500);
			ResetEvent(dados->respostaPass);
		}

		ReleaseMutex(dados->hMutexDados);

		Sleep(1000);
	}

	ExitThread(0);
}