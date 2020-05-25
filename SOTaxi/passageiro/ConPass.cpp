#include "ConPass.h"

void(*ptr_register)(TCHAR*, int);

int _tmain() {
	HANDLE hThreadComandos, hThreadMovimentaPassageiro;
	DADOS dados;
	dados.nPassageiros = 0;
	dados.terminar = 0;

	HINSTANCE hLib;

	hLib = LoadLibrary(PATH_DLL);
	if (hLib == NULL)
		return 0;

#ifdef UNICODE
	_setmode(_fileno(stdin), _O_WTEXT);
	_setmode(_fileno(stdout), _O_WTEXT);
#endif

	ptr_register = (void(*)(TCHAR*, int))GetProcAddress(hLib, "dll_register");

	Semaphore = CreateSemaphore(NULL, 1, 1, SEMAPHORE_NAME);
	if (Semaphore == NULL) {
		_tprintf(TEXT("CreateSemaphore failed.\n"));
		return FALSE;
	}
	ptr_register((TCHAR*)SEMAPHORE_NAME, 3);

	_tprintf(TEXT("\nAguardando autorização para entrar...\n"));
	WaitForSingleObject(Semaphore, INFINITE);
	_tprintf(TEXT("\nEntrei!\n"));

	novoPass = CreateEvent(NULL, TRUE, FALSE, EVENT_NOVOP);
	if (novoPass == NULL) {
		_tprintf(TEXT("\n[ERRO] Erro ao criar Evento!\n"));
		return 0;
	}
	ptr_register((TCHAR*)EVENT_NOVOP, 4);

	respostaPass = CreateEvent(NULL, TRUE, FALSE, EVENT_RESPOSTAP);
	if (respostaPass == NULL) {
		_tprintf(TEXT("\n[ERRO] Erro ao criar Evento!\n"));
		CloseHandle(Semaphore);
		CloseHandle(novoPass);
		return 0;
	}
	ptr_register((TCHAR*)EVENT_RESPOSTAP, 4);
	SetEvent(respostaPass);
	ResetEvent(respostaPass);

	respostaMov = CreateEvent(NULL, TRUE, FALSE, EVENT_MOVIMENTOP);
	if (respostaMov == NULL) {
		_tprintf(TEXT("\n[ERRO] Erro ao criar Evento!\n"));
		CloseHandle(Semaphore);
		CloseHandle(novoPass);
		CloseHandle(respostaPass);
		return 0;
	}
	ptr_register((TCHAR*)EVENT_MOVIMENTOP, 4);
	SetEvent(respostaMov);
	ResetEvent(respostaMov);

	if (!WaitNamedPipe(PIPE_NAME, NMPWAIT_WAIT_FOREVER)) {
		_tprintf(TEXT("[ERRO] Ligar ao pipe '%s'! (WaitNamedPipe)\n"), PIPE_NAME);
		return 0;
	}
	_tprintf(TEXT("[LEITOR] Ligação ao pipe do escritor... (CreateFile)\n"));
	hPipe = CreateFile(PIPE_NAME, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hPipe == NULL) {
		_tprintf(TEXT("[ERRO] Ligar ao pipe '%s'! (CreateFile)\n"), PIPE_NAME);
		return 0;
	}

	dados.hMutex = CreateMutex(NULL, FALSE, TEXT("MutexPass"));
	if (dados.hMutex == NULL) {
		_tprintf(TEXT("\n[ERRO] Erro ao criar Mutex!\n"));
		return 0;
	}
	ptr_register((TCHAR*)TEXT("MutexPass"), 1);
	WaitForSingleObject(dados.hMutex, INFINITE);

	hThreadComandos = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ThreadComandos, (LPVOID)&dados, 0, NULL);
	if (hThreadComandos == NULL) {
		_tprintf(TEXT("\n[ERRO] Erro ao lançar Thread!\n"));
		return 0;
	}
	hThreadMovimentaPassageiro = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ThreadMovimentoPassageiro, (LPVOID)&dados, 0, NULL);
	if (hThreadMovimentaPassageiro == NULL) {
		_tprintf(TEXT("\nErro ao lançar Thread!\n"));
		return 0;
	}
	ReleaseMutex(dados.hMutex);

	HANDLE ghEvents[2];
	ghEvents[0] = hThreadComandos;
	ghEvents[1] = hThreadMovimentaPassageiro;
	WaitForMultipleObjects(2, ghEvents, TRUE, INFINITE);

	_tprintf(TEXT("Passageiros vão sair!\n"));
	_tprintf(TEXT("Prima uma tecla...\n"));
	_gettch();

	ReleaseSemaphore(Semaphore, 1, NULL);

	_tprintf(TEXT("[ConPass] Desligar o pipe (DisconnectNamedPipe)\n"));
	if (!DisconnectNamedPipe(hPipe)) {
		_tprintf(TEXT("[ERRO] Desligar o pipe! (DisconnectNamedPipe)"));
		return 0;
	}

	CloseHandle(Semaphore);
	CloseHandle(hPipe);
	CloseHandle(novoPassageiro);
	CloseHandle(respostaPass);
	CloseHandle(respostaMov);
	FreeLibrary(hLib);

	return 0;
}

boolean removePassageiro(DADOS* dados, PASSAGEIRO novo) {
	for (int i = 0; i < dados->nPassageiros; i++) {
		if (!_tcscmp(novo.id, dados->passageiros[i].id)) {
			for (int k = i; k < dados->nPassageiros - 1; k++) {
				dados->passageiros[k] = dados->passageiros[k + 1];
			}
			dados->nPassageiros--;
			_tprintf(TEXT("\n[SAIU PASSAGEIRO] Saiu Passageiro: %s\n\n"), novo.id);
			return TRUE;
		}
	}
	return FALSE;
}

void novoPassageiro(DADOS* dados) {
	DWORD n;
	PASSAGEIRO novo;
	TCHAR aux[TAM];

	_tprintf(_T("\n[NOVO] Id do Passageiro: "));
	_fgetts(aux, TAM_ID, stdin);
	aux[_tcslen(aux) - 1] = '\0';
	_tcscpy_s(novo.id, _countof(novo.id), aux);

	_tprintf(_T("\n[NOVO]  Localizacao do Passageiro (X Y) : "));
	_tscanf_s(_T("%d"), &novo.X);
	_tscanf_s(_T("%d"), &novo.Y);

	_tprintf(_T("\n[NOVO]  Local de destino do Passageiro (X Y) : "));
	_tscanf_s(_T("%d"), &novo.Xfinal);
	_tscanf_s(_T("%d"), &novo.Yfinal);
	novo.movimento = 0;
	novo.terminar = 0;
	novo.id_mapa = TEXT('.');
	novo.tempoEspera = -1;
	for (int i = 0; i < 6; i++)
		novo.matriculaTaxi[i] = ' ';
	novo.matriculaTaxi[6] = '\0';

	//VAI AO ADMIN VER SE PODE CRIAR
	WriteFile(hPipe, (LPVOID)&novo, sizeof(PASSAGEIRO), &n, NULL);

	SetEvent(novoPass);
	Sleep(500);
	ResetEvent(novoPass);
	dados->passageiros[dados->nPassageiros] = novo;

	WaitForSingleObject(respostaPass, INFINITE);

	ReadFile(hPipe, (LPVOID)&dados->passageiros[dados->nPassageiros], sizeof(PASSAGEIRO), &n, NULL);
	if (dados->passageiros[dados->nPassageiros].terminar)
		_tprintf(_T("\n[NOVO] Passageiro terá de aguardar!"));
	else {
		if (dados->passageiros[dados->nPassageiros].tempoEspera != -1)
			_tprintf(_T("\n[NOVO]  Tempo estimado de espera pelo Taxi '%s' é %d s"), dados->passageiros[dados->nPassageiros].matriculaTaxi, dados->passageiros[dados->nPassageiros].tempoEspera);
		else
			_tprintf(_T("\n[NOVO]  Não houve interesse neste transporte!"));
	}

	return;
}

void listarPassageiros(DADOS* dados) {
	if (dados->nPassageiros == 0) {
		_tprintf(_T("\n[LISTAR PASSAGEIRO] Não há Passageiros!"));
		return;
	}
	for (int i = 0; i < dados->nPassageiros; i++) {
		_tprintf(_T("\n[LISTAR PASSAGEIRO] Passageiro %s : "), dados->passageiros[i].id);
		_tprintf(_T("\n (%d, %d) -> (%d, %d)"), dados->passageiros[i].X, dados->passageiros[i].Y, dados->passageiros[i].Xfinal, dados->passageiros[i].Yfinal);
		if (dados->passageiros[i].movimento)
			_tprintf(_T(" - em movimento\n"));
		else
			_tprintf(_T(" - à espera\n"));
	}
	return;
}

DWORD WINAPI ThreadComandos(LPVOID param) {
	TCHAR op[TAM], i;
	DADOS* dados = ((DADOS*)param);

	do {
		_tprintf(_T("\n\n"));
		i = _gettch();
		//WaitForSingleObject(dados->hMutex, INFINITE);
		_tprintf(_T("%c"), i);
		op[0] = i;
		_fgetts(&op[1], sizeof(op), stdin);
		op[_tcslen(op) - 1] = '\0';
		//NOVO PASSAGEIRO
		if (!_tcscmp(op, TEXT("novo"))) {		
			novoPassageiro(dados);
			dados->nPassageiros++;
		}
		//LISTAR PASSAGEIROS
		else if (!_tcscmp(op, TEXT("listar"))) {
			listarPassageiros(dados);
		}
		_tprintf(_T("\n\n"));
		//ReleaseMutex(dados->hMutex);
	} while (_tcscmp(op, TEXT("fim")));

	for (int i = 0; i < MAX_PASS; i++)
		dados->passageiros[i].terminar = 1;

	dados->terminar = 1;

	ExitThread(0);
}

DWORD WINAPI ThreadMovimentoPassageiro(LPVOID param) {	//ADMIN MANDA PASSAGEIRO
	DADOS* dados = ((DADOS*)param);
	DWORD n;
	PASSAGEIRO novo;
	int i, num;

	while (1) {
		num = -1;
		WaitForSingleObject(respostaMov, INFINITE);

		if (dados->terminar)
			return 0;

		WaitForSingleObject(dados->hMutex, INFINITE);

		ReadFile(hPipe, (LPVOID)&novo, sizeof(PASSAGEIRO), &n, NULL);
		for (i = 0; i < dados->nPassageiros && num == -1; i++)
			if (!_tcscmp(novo.id, dados->passageiros[i].id)) {
				dados->passageiros[i] = novo;
				num = i;
			}

		if (dados->passageiros[num].movimento)
			_tprintf(_T("\n[PASS] Passageiro '%s' está ser transportado!"), dados->passageiros[num].id);
		else {
			_tprintf(_T("\n[PASS] Passageiro '%s' chegou ao destino!"), dados->passageiros[num].id);

			//REMOVE PASSAGEIRO
			removePassageiro(dados, dados->passageiros[num]);
		}
		ReleaseMutex(dados->hMutex);
	}

	ExitThread(0);
}