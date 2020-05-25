#include "ConPass.h"

void(*ptr_register)(TCHAR*, int);

int _tmain() {
	HANDLE hThreadComandos, hThreadMovimentaPassageiro, hThreadRespostaTransporte;
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

	if (!WaitNamedPipe(PIPE_NAME, NMPWAIT_WAIT_FOREVER)) {
		_tprintf(TEXT("[ERRO] Ligar ao pipe '%s'! (WaitNamedPipe)\n"), PIPE_NAME);
		exit(-1);
	}
	_tprintf(TEXT("[LEITOR] Ligação ao pipe do escritor... (CreateFile)\n"));
	hPipe = CreateFile(PIPE_NAME, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hPipe == NULL) {
		_tprintf(TEXT("[ERRO] Ligar ao pipe '%s'! (CreateFile)\n"), PIPE_NAME);
		exit(-1);
	}

	hThreadComandos = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ThreadComandos, (LPVOID)&dados, 0, NULL);
	if (hThreadComandos == NULL) {
		_tprintf(TEXT("\n[ERRO] Erro ao lançar Thread!\n"));
		return 0;
	}
	//hThreadMovimentaPassageiro = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ThreadMovimentaPassageiro, (LPVOID)&dados, 0, NULL);
	//if (hThreadMovimentaPassageiro == NULL) {
	//	_tprintf(TEXT("\nErro ao lançar Thread!\n"));
	//	return 0;
	//}
	//hThreadRespostaTransporte = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ThreadRespostaTransporte, (LPVOID)&dados, 0, NULL);
	//if (hThreadRespostaTransporte == NULL) {
	//	_tprintf(TEXT("\nErro ao lançar Thread!\n"));
	//	return 0;
	//}

	HANDLE ghEvents[1];
	ghEvents[0] = hThreadComandos;
	/*ghEvents[1] = hThreadMovimentaPassageiro;
	ghEvents[2] = hThreadRespostaTransporte;*/
	WaitForMultipleObjects(1, ghEvents, TRUE, INFINITE);

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
	FreeLibrary(hLib);

	return 0;
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
	if(dados->passageiros[dados->nPassageiros].tempoEspera!=-1)
		_tprintf(_T("\n[NOVO]  Tempo estimado de espera pelo Taxi '%s' é %d s"), dados->passageiros[dados->nPassageiros].matriculaTaxi, dados->passageiros[dados->nPassageiros].tempoEspera);
	else
		_tprintf(_T("\n[NOVO]  Não houve interesse neste transporte!"));

	return;
}

DWORD WINAPI ThreadComandos(LPVOID param) {
	TCHAR op[TAM], i;
	DADOS* dados = ((DADOS*)param);

	do {
		_tprintf(_T("\n\n"));
		i = _gettch();
		_tprintf(_T("%c"), i);
		op[0] = i;
		_fgetts(&op[1], sizeof(op), stdin);
		op[_tcslen(op) - 1] = '\0';
		if (!_tcscmp(op, TEXT("novo"))) {		//NOVO PASSAGEIRO
			novoPassageiro(dados);
			dados->nPassageiros++;
		}
		_tprintf(_T("\n\n"));
	} while (_tcscmp(op, TEXT("fim")));

	for (int i = 0; i < MAX_PASS; i++)
		dados->passageiros[i].terminar = 1;

	dados->terminar = 1;

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