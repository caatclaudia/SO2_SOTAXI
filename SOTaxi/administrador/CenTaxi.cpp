#include "CenTaxi.h"

//ptr_log -> Quando Taxi começa a transportar

HKEY chave;
DWORD queAconteceu;
DWORD tamanho;

int _tmain(int argc, LPTSTR argv[]) {
	HANDLE hThreadComandos, hThreadNovoTaxi, hThreadSaiuTaxi, hThreadMovimento, hThreadNovoPassageiro, hThreadTempoTransporte;
	DADOS dados;
	dados.nTaxis = 0;
	dados.nPassageiros = 0;
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

	inicializaVariaveis();
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

	dados.infoAdmin = CreateEvent(NULL, TRUE, FALSE, EVENT_SAIUA);
	if (dados.infoAdmin == NULL) {
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
	//hThreadNovoPassageiro = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ThreadNovoPassageiro, (LPVOID)&dados, 0, NULL);
	//if (hThreadNovoPassageiro == NULL) {
	//	_tprintf(TEXT("\nErro ao lançar Thread!\n"));
	//	return 0;
	//}
	hThreadTempoTransporte = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ThreadTempoTransporte, (LPVOID)&dados, 0, NULL);
	if (hThreadTempoTransporte == NULL) {
		_tprintf(TEXT("\n[ERRO] Erro ao lançar Thread!\n"));
		return 0;
	}

	HANDLE ghEvents[5];
	ghEvents[0] = hThreadComandos;
	ghEvents[1] = hThreadNovoTaxi;
	ghEvents[2] = hThreadSaiuTaxi;
	ghEvents[3] = hThreadMovimento;
	ghEvents[4] = hThreadTempoTransporte;
	//ghEvents[2] = hThreadNovoPassageiro;
	WaitForMultipleObjects(5, ghEvents, FALSE, INFINITE);
	TerminateThread(hThreadNovoTaxi, 0);
	TerminateThread(hThreadSaiuTaxi, 0);
	TerminateThread(hThreadMovimento, 0);
	TerminateThread(hThreadTempoTransporte, 0);

	WaitForSingleObject(dados.hMutexDados, INFINITE);

	//NAMED PIPES
	for (int i = 0; i < dados.nTaxis; i++)
		dados.taxis[i].terminar = 1;
	for (int i = 0; i < dados.nPassageiros; i++)
		dados.passageiros[i].terminar = 1;

	SetEvent(dados.infoAdmin);
	Sleep(500);
	ResetEvent(dados.infoAdmin);

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
	CloseHandle(dados.infoAdmin);
	CloseHandle(dados.EspMapa);
	CloseHandle(transporte);
	FreeLibrary(hLib);
	RegCloseKey(chave);

	return 0;
}

void inicializaBuffer() {
	hMemoria = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(BUFFER), BUFFER_CIRCULAR);
	ptr_register((TCHAR*)BUFFER_CIRCULAR, 6);

	BufferMemoria = (BUFFER*)MapViewOfFile(hMemoria, FILE_MAP_WRITE, 0, 0, sizeof(BUFFER));
	ptr_register((TCHAR*)BUFFER_CIRCULAR, 7);

	sem_mutex = CreateSemaphore(NULL, 1, 1, SEMAPHORE_MUTEX);
	if (sem_mutex == NULL) {
		_tprintf(TEXT("\n[ERRO] Erro ao criar Semáforo!\n"));
		return;
	}
	ptr_register((TCHAR*)SEMAPHORE_MUTEX, 3);

	sem_itens = CreateSemaphore(NULL, 0, MAX_PASS, SEMAPHORE_ITENS);
	if (sem_itens == NULL) {
		_tprintf(TEXT("\n[ERRO] Erro ao criar Semáforo!\n"));
		return;
	}
	ptr_register((TCHAR*)SEMAPHORE_ITENS, 3);

	sem_vazios = CreateSemaphore(NULL, MAX_PASS, MAX_PASS, SEMAPHORE_VAZIOS);
	if (sem_vazios == NULL) {
		_tprintf(TEXT("\n[ERRO] Erro ao criar Semáforo!\n"));
		return;
	}
	ptr_register((TCHAR*)SEMAPHORE_VAZIOS, 3);

	return;
}

void inicializaVariaveis() {
	DWORD opcao;
	TCHAR str[TAM], op;

	//Criar/abrir uma chave em HKEY_CURRENT_USER\Software\MinhaAplicacao
	if (RegCreateKeyEx(HKEY_CURRENT_USER, TEXT("Software\\Mapa"), 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &chave, &queAconteceu) != ERROR_SUCCESS) {
		_tprintf(TEXT("[ERRO] Erro ao criar/abrir chave (%d)\n"), GetLastError());
		return;
	}
	else {
		//Se a chave foi criada, inicializar os valores
		if (queAconteceu == REG_CREATED_NEW_KEY) {
			_tprintf(TEXT("[DETALHES] Chave: HKEY_CURRENT_USER\\Software\\Mapa\n"));
			//Criar valor "TaxiOcupado"
			RegSetValueEx(chave, TEXT("TaxiOcupado"), 0, REG_SZ, (LPBYTE)TEXT("Vermelho"), _tcslen(TEXT("Vermelho")) * sizeof(TCHAR));
			//Criar valor "TaxiLivre"
			RegSetValueEx(chave, TEXT("TaxiLivre"), 0, REG_SZ, (LPBYTE)TEXT("Verde"), _tcslen(TEXT("Verde")) * sizeof(TCHAR));
			//Criar valor "TaxiBuscarPassageiro"
			RegSetValueEx(chave, TEXT("TaxiBuscarPassageiro"), 0, REG_SZ, (LPBYTE)TEXT("Laranja"), _tcslen(TEXT("Laranja")) * sizeof(TCHAR));
			_tprintf(TEXT("[DETALHES] Valores TaxiOcupado, TaxiLivre e TaxiBuscarPassageiro guardados\n"));
		}
		//Se a chave foi aberta, ler os valores lá guardados
		else if (queAconteceu == REG_OPENED_EXISTING_KEY) {
			_tprintf(TEXT("Chave: HKEY_CURRENT_USER\\Software\\Mapa\n"));
			tamanho = TAM;
			RegQueryValueEx(chave, TEXT("TaxiOcupado"), NULL, NULL, (LPBYTE)str, &tamanho);
			str[tamanho / sizeof(TCHAR)] = '\0';
			_tprintf(TEXT("[DETALHES] TaxiOcupado: %s\n"), str);
			RegQueryValueEx(chave, TEXT("TaxiLivre"), NULL, NULL, (LPBYTE)str, &tamanho);
			str[tamanho / sizeof(TCHAR)] = '\0';
			_tprintf(TEXT("[DETALHES] TaxiLivre: %s\n"), str);
			RegQueryValueEx(chave, TEXT("TaxiBuscarPassageiro"), NULL, NULL, (LPBYTE)str, &tamanho);
			str[tamanho / sizeof(TCHAR)] = '\0';
			_tprintf(TEXT("[DETALHES] TaxiBuscarPassageiro: %s\n"), str);
		}
		do {
			_tprintf(TEXT("\n[DETALHES] Deseja alterar algum destes valores (s/n)? "));
			_tscanf_s(_T("%c"), &op, sizeof(op));
		} while (op != 's' && op != 'n');
		if (op == 's') {
			do {
				_tprintf(TEXT("\n[DETALHES] 1- Valor TaxiOcupado"));
				_tprintf(TEXT("\n[DETALHES] 2- Valor TaxiLivre"));
				_tprintf(TEXT("\n[DETALHES] 3- Valor TaxiBuscarPassageiro"));
				_tprintf(TEXT("\n[DETALHES] 4- Voltar"));
				_tprintf(TEXT("\n[DETALHES] Opção: "));
				_tscanf_s(_T("%d"), &opcao);
				if (opcao == 1) {
					_tprintf(TEXT("\n[DETALHES] Valor TaxiOcupado: "));
					_tscanf_s(_T("%s"), str, sizeof(str));
					RegSetValueEx(chave, TEXT("TaxiOcupado"), 0, REG_SZ, (LPBYTE)str, _tcslen(str) * sizeof(TCHAR));
				}
				else if (opcao == 2) {
					_tprintf(TEXT("\n[DETALHES] Valor TaxiLivre: "));
					_tscanf_s(_T("%s"), str, sizeof(str));
					RegSetValueEx(chave, TEXT("TaxiLivre"), 0, REG_SZ, (LPBYTE)str, _tcslen(str) * sizeof(TCHAR));
				}
				else if (opcao == 3) {
					_tprintf(TEXT("\n[DETALHES] Valor TaxiBuscarPassageiro: "));
					_tscanf_s(_T("%s"), str, sizeof(str));
					RegSetValueEx(chave, TEXT("TaxiBuscarPassageiro"), 0, REG_SZ, (LPBYTE)str, _tcslen(str) * sizeof(TCHAR));
				}
			} while (opcao != 4);
		}
	}
	return;
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
	if (dados->nTaxis == 0) {
		_tprintf(_T("\n[LISTAR TAXIS] Não há Taxis!"));
		return;
	}
	for (int i = 0; i < dados->nTaxis; i++) {
		_tprintf(_T("\n[LISTAR TAXIS] Taxi %s : "), dados->taxis[i].matricula);
		_tprintf(_T(" (%d, %d) "), dados->taxis[i].X, dados->taxis[i].Y);
		if (dados->taxis[i].disponivel)
			_tprintf(_T("sem passageiro!\n"));
		else
			_tprintf(_T("com passageiro!\n"));
	}
	return;
}

void listarPassageiros(DADOS* dados) {
	if (dados->nPassageiros == 0) {
		_tprintf(_T("\n[LISTAR PASSAGEIRO] Não há Passageiros!"));
		return;
	}
	for (int i = 0; i < dados->nPassageiros; i++) {
		_tprintf(_T("\n[LISTAR PASSAGEIRO] Passageiro %d : "), i);
		_tprintf(_T("\n (%d, %d) -> (%d, %d)\n"), dados->passageiros[i].X, dados->passageiros[i].Y, dados->passageiros[i].Xfinal, dados->passageiros[i].Yfinal);
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

boolean adicionaTaxi(DADOS* dados, TAXI novo) {
	TCHAR aux[TAM];
	if (!dados->aceitacaoT)
		return FALSE;
	if (dados->nTaxis >= MaxTaxi)
		return FALSE;
	for (int i = 0; i < dados->nTaxis; i++)
		if (!_tcscmp(novo.matricula, dados->taxis[i].matricula))
			return FALSE;

	dados->taxis[dados->nTaxis] = novo;
	dados->taxis[dados->nTaxis].id_mapa = id_mapa_taxi;
	dados->nTaxis++;
	id_mapa_taxi++;
	_tprintf(TEXT("[NOVO TAXI] Novo Taxi: %s\n"), novo.matricula);
	_stprintf_s(aux, TAM, TEXT("Taxi %s entrou!"), novo.matricula);
	ptr_log(aux);
	ptr_log((TCHAR*)aux);
	return TRUE;
}

boolean removeTaxi(DADOS* dados, TAXI novo) {
	TCHAR aux[TAM];
	for (int i = 0; i < dados->nTaxis; i++) {
		if (!_tcscmp(novo.matricula, dados->taxis[i].matricula)) {
			for (int k = i; k < dados->nTaxis - 1; k++) {
				dados->taxis[k] = dados->taxis[k + 1];
			}
			dados->nTaxis--;
			_tprintf(TEXT("[SAIU TAXI] Saiu Taxi: %s\n"), novo.matricula);
			_stprintf_s(aux, TAM, TEXT("Taxi %s saiu!"), novo.matricula);
			ptr_log(aux);
			ptr_log((TCHAR*)aux);
			return TRUE;
		}
	}
	return FALSE;
}

boolean adicionaPassageiro(DADOS* dados, PASSAGEIRO novo) {
	TCHAR aux[TAM];
	if (dados->nPassageiros >= MaxPass)
		return FALSE;

	dados->passageiros[dados->nPassageiros] = novo;
	dados->passageiros[dados->nPassageiros].id_mapa = id_mapa_pass;
	id_mapa_pass++;
	dados->nPassageiros++;
	_tprintf(TEXT("[NOVO PASSAGEIRO] Novo Passageiro: %s\n"), novo.id);
	_stprintf_s(aux, TAM, TEXT("Passageiro %s entrou!"), novo.id);
	ptr_log(aux);
	ptr_log((TCHAR*)aux);
	return TRUE;
}

boolean removePassageiro(DADOS* dados, PASSAGEIRO novo) {
	TCHAR aux[TAM];
	for (int i = 0; i < dados->nPassageiros; i++) {
		if (!_tcscmp(novo.id, dados->passageiros[i].id)) {
			for (int k = i; k < dados->nPassageiros - 1; k++) {
				dados->passageiros[k] = dados->passageiros[k + 1];
			}
			dados->nPassageiros--;
			_tprintf(TEXT("[SAIU PASSAGEIRO] Saiu Passageiro: %s\n"), novo.id);
			_stprintf_s(aux, TAM, TEXT("Passageiro %s saiu!"), novo.id);
			ptr_log(aux);
			ptr_log((TCHAR*)aux);
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

	for (i = 0; i < dados->nTaxis && _tcscmp(matr, dados->taxis[i].matricula); i++);
	if (!_tcscmp(matr, dados->taxis[i].matricula)) {
		dados->taxis[i].terminar = 1;
		enviaTaxi(dados, &dados->taxis[i]);
		Sleep(1000);
		removeTaxi(dados, dados->taxis[i]);
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

void newPassageiro(DADOS* dados) {
	PASSAGEIRO novo;
	TCHAR aux[TAM] = TEXT("\n");

	_tprintf(_T("\n[NOVO] Id do Passageiro: "));
	_fgetts(novo.id, TAM, stdin);
	novo.id[_tcslen(novo.id) - 1] = '\0';

	_tprintf(_T("\n[NOVO]  Localizacao do Passageiro (X Y) : "));
	_tscanf_s(_T("%d"), &novo.X);
	_tscanf_s(_T("%d"), &novo.Y);

	_tprintf(_T("\n[NOVO]  Local de destino do Passageiro (X Y) : "));
	_tscanf_s(_T("%d"), &novo.Xfinal);
	_tscanf_s(_T("%d"), &novo.Yfinal);
	novo.movimento = 0;
	novo.terminar = 0;
	novo.id_mapa = TEXT('.');
	adicionaPassageiro(dados, novo);

	_stprintf_s(aux, TAM, TEXT("Passageiro %s em (%d,%d)!"), novo.id, novo.X, novo.Y);
	ptr_log(aux);
	ptr_log((TCHAR*)aux);

	WaitForSingleObject(sem_vazios, INFINITE);
	WaitForSingleObject(sem_mutex, INFINITE);
	BufferMemoria->Passageiros[BufferMemoria->NextIn] = novo;
	BufferMemoria->NextIn = (BufferMemoria->NextIn + 1) % MAX_PASS;
	ptr_log((TCHAR*)TEXT("CenTaxi coloca passageiro em Buffer Circular!"));
	ReleaseSemaphore(sem_mutex, 1, NULL);
	ReleaseSemaphore(sem_itens, 1, NULL);

	TAXI novoT;
	acabouTempo = 0;
	int interessados = 0;
	Sleep(500);
	while (!acabouTempo) {
		SetEvent(transporte);
		Sleep(500);
		ResetEvent(transporte);

		//RECEBE TODOS OS INTERESSES
		CopyMemory(&novoT, dados->sharedTaxi, sizeof(TAXI));
		ptr_log((TCHAR*)TEXT("CenTaxi recebe Taxi do ConTaxi por memória partilhada!"));
		if (novoT.interessado) {
			for (int i = 0; i < dados->nTaxis; i++)
				if (!_tcscmp(novoT.matricula, dados->taxis[i].matricula)) {
					dados->taxis[i] = novoT;
					interessados++;
				}
		}
	}

	if (interessados > 0) {
		int valor;
		do {
			valor = rand() % dados->nTaxis;
		} while (!dados->taxis[valor].interessado);
		_stprintf_s(aux, TAM, TEXT("Taxi %s vai transportar %s!"), dados->taxis[valor].matricula, novo.id);
		ptr_log(aux);
		ptr_log((TCHAR*)aux);
		dados->taxis[valor].Xfinal = novo.X;
		dados->taxis[valor].Yfinal = novo.Y;
		dados->taxis[valor].disponivel = 0;

		CopyMemory(dados->sharedTaxi, &dados->taxis[valor], sizeof(TAXI));
		ptr_log((TCHAR*)TEXT("CenTaxi envia Taxi para ConTaxi por memória partilhada!"));
		SetEvent(dados->respostaAdmin);
		Sleep(500);
		ResetEvent(dados->respostaAdmin);

		//DEPOIS DE ACEITAR TRANSPORTE
		transporteAceite(dados);
	}

	return;
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
	TCHAR op[TAM], matr[7];
	DADOS* dados = ((DADOS*)param);

	do {
		_tprintf(_T("\n\n>> "));
		_fgetts(op, sizeof(op), stdin);
		op[_tcslen(op) - 1] = '\0';
		WaitForSingleObject(dados->hMutexDados, INFINITE);
		//NOVO PASSAGEIRO
		if (!_tcscmp(op, TEXT("novoP"))) {
			newPassageiro(dados);
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
		else if (!_tcscmp(op, TEXT("listar"))) {
			listarTaxis(dados);
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
			_tprintf(TEXT("Novo Taxi: %s\n"), dados->taxis[dados->nTaxis - 1].matricula);
			CopyMemory(dados->sharedTaxi, &dados->taxis[dados->nTaxis - 1], sizeof(TAXI));
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

	dados->atualizaMap = CreateEvent(NULL, TRUE, FALSE, EVENT_ATUALIZAMAP);
	if (dados->atualizaMap == NULL) {
		_tprintf(TEXT("\n[ERRO] Erro ao criar Evento!\n"));
		return 0;
	}

	while (1) {
		TCHAR aux[TAM] = TEXT("\n");
		WaitForSingleObject(dados->movimentoTaxi, INFINITE);

		if (dados->terminar)
			return 0;
		WaitForSingleObject(dados->hMutexDados, INFINITE);

		CopyMemory(&novo, dados->sharedTaxi, sizeof(TAXI));
		ptr_log((TCHAR*)TEXT("CenTaxi recebe Taxi do ConTaxi por memória partilhada!"));
		for (int i = 0; i < dados->nTaxis; i++)
			if (!_tcscmp(novo.matricula, dados->taxis[i].matricula)) {
				dados->taxis[i] = novo;
				_tprintf(_T("\n[MOVIMENTO] Taxi %s -> (%d,%d)"), novo.matricula, novo.X, novo.Y);
				char buf;
				buf = dados->taxis[i].id_mapa + '0';
				eliminaIdMapa(dados, buf);
				dados->mapa[tamanhoMapa * novo.Y + novo.Y + novo.X].caracter = buf;
				//SE TIVER PASSAGEIRO VERIFICA SE CHEGOU AO DESTINO
				if (!dados->taxis[i].disponivel) {
					VALIDO = 0;
					for (int j = 0; j < dados->nPassageiros && VALIDO == 0; j++) {
						//SE TAXI CHEGOU AO SITIO DE RECOLHA DE PASSAGEIRO
						if (dados->passageiros[j].X == dados->taxis[i].Xfinal && dados->passageiros[j].X == dados->taxis[i].X && dados->passageiros[j].Y == dados->taxis[i].Yfinal && dados->passageiros[j].Y == dados->taxis[i].Y) {
							_tprintf(_T("\n[MOVIMENTO] Taxi '%s' apanhou o Passageiro '%s'\n"), dados->taxis[i].matricula, dados->passageiros[j].id);
							_stprintf_s(aux, TAM, TEXT("Passageiro %s a ser transportado!"), dados->passageiros[j].id);
							ptr_log(aux);
							ptr_log((TCHAR*)aux);
							dados->taxis[i].Xfinal = dados->passageiros[j].Xfinal;
							dados->taxis[i].Yfinal = dados->passageiros[j].Yfinal;
							//AVISA O TÁXI
							enviaTaxi(dados, &dados->taxis[i]);
							Sleep(1000);
							VALIDO = 1;
						}
						//SE TAXI CHEGOU AO SITIO DE DESTINO DO PASSAGEIRO
						if (dados->passageiros[j].Xfinal == dados->taxis[i].Xfinal && dados->passageiros[j].Xfinal == dados->taxis[i].X && dados->passageiros[j].Yfinal == dados->taxis[i].Yfinal && dados->passageiros[j].Yfinal == dados->taxis[i].Y) {
							_tprintf(_T("\n[MOVIMENTO] Taxi '%s' deixou Passageiro '%s'\n"), dados->taxis[i].matricula, dados->passageiros[j].id);
							_stprintf_s(aux, TAM, TEXT("Passageiro %s entregue!"), dados->passageiros[j].id);
							ptr_log(aux);
							ptr_log((TCHAR*)aux);
							removePassageiro(dados, dados->passageiros[j]);
							dados->taxis[i].disponivel = 1;
							dados->taxis[i].Xfinal = 0;
							dados->taxis[i].Yfinal = 0;
							//AVISA O TÁXI
							enviaTaxi(dados, &dados->taxis[i]);
							Sleep(1000);
							VALIDO = 1;
						}
					}
				}
			}
		CopyMemory(dados->sharedMapa, dados->mapa, sizeof(dados->mapa));
		ptr_log((TCHAR*)TEXT("CenTaxi envia Mapa para MapInfo por memória partilhada!"));
		_tprintf(TEXT("\n[MAPA] Mapa atualizado com sucesso!\n"));

		ReleaseMutex(dados->hMutexDados);

		SetEvent(dados->atualizaMap);
		Sleep(500);
		ResetEvent(dados->atualizaMap);

		Sleep(500);
	}

	ExitThread(0);
}

//VERIFICA SE HA NOVOS PASSAGEIROS
DWORD WINAPI ThreadNovoPassageiro(LPVOID param) {
	DADOS* dados = ((DADOS*)param);
	TCHAR aux[TAM] = TEXT("\n"), aux1[TAM] = TEXT("\n");
	PASSAGEIRO novo;

	/*_stprintf_s(aux, TAM, TEXT("Passageiro %s entrou!"), novo.id);
	ptr_log(aux);
	ptr_log((TCHAR*)aux);
	_stprintf_s(aux1, TAM, TEXT("Passageiro %s em (%d,%d)!"), novo.id, novo.X, novo.Y);
	ptr_log(aux1);
	ptr_log((TCHAR*)aux1);*/

	WaitForSingleObject(sem_vazios, INFINITE);
	WaitForSingleObject(sem_mutex, INFINITE);
	//BufferMemoria->Passageiros[BufferMemoria->NextIn] = novo;
	BufferMemoria->NextIn = (BufferMemoria->NextIn + 1) % MAX_PASS;
	ReleaseSemaphore(sem_mutex, 1, NULL);
	ReleaseSemaphore(sem_itens, 1, NULL);


	//DEPOIS DE ACEITAR TRANSPORTE
	transporteAceite(dados);

	ExitThread(0);
}