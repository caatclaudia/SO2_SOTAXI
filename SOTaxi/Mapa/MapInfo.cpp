#include "MapInfo.h"

void(*ptr_register)(TCHAR*, int);

int _tmain(int argc, TCHAR argv[]) {
	HANDLE hThreadSair, hThreadAtualizaMapa;
	DADOS dados;
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

	hMutex = CreateMutex(NULL, FALSE, NOME_MUTEXMAPA);
	if (hMutex == NULL) {
		_tprintf(TEXT("\n[ERRO] Erro ao criar Mutex!\n"));
		return -1;
	}
	ptr_register((TCHAR*)NOME_MUTEXMAPA, 1);
	WaitForSingleObject(hMutex, INFINITE);
	ReleaseMutex(hMutex);

	EspMapa = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(dados.mapa), SHM_NAME);
	if (EspMapa == NULL)
	{
		_tprintf(TEXT("\n[ERRO] Erro ao criar FileMapping!\n"));
		return -1;
	}
	ptr_register((TCHAR*)SHM_NAME, 6);

	shared = (MAPA*)MapViewOfFile(EspMapa, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(dados.mapa));
	if (shared == NULL)
	{
		_tprintf(TEXT("\n[ERRO] Erro em MapViewOfFile!\n"));
		CloseHandle(EspMapa);
		return -1;
	}
	ptr_register((TCHAR*)SHM_NAME, 7);

	recebeMapa(&dados);

	hThreadAtualizaMapa = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ThreadAtualizaMapa, (LPVOID)&dados, 0, NULL);
	if (hThreadAtualizaMapa == NULL) {
		_tprintf(TEXT("\n[ERRO] Erro ao lançar Thread!\n"));
		return 0;
	}

	hThreadSair = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ThreadSair, (LPVOID)&dados, 0, NULL);
	if (hThreadSair == NULL) {
		_tprintf(TEXT("\n[ERRO] Erro ao lançar Thread!\n"));
		return 0;
	}

	HANDLE ghEvents[2];
	ghEvents[0] = hThreadAtualizaMapa;
	ghEvents[1] = hThreadSair;
	WaitForMultipleObjects(2, ghEvents, TRUE, INFINITE);

	TerminateThread(hThreadAtualizaMapa, 0);


	_tprintf(TEXT("\nPrima uma tecla...\n"));
	_gettch();

	CloseHandle(EspMapa);
	CloseHandle(atualizaMap);
	FreeLibrary(hLib);

	return 0;
}

void recebeMapa(DADOS* dados) {
	MAPA* aux = NULL;
	CopyMemory(&aux, shared, sizeof(shared));
	for (int i = 0; tamanhoMapa == -1; i++)
		if (shared[i].caracter == '\n')
			tamanhoMapa = i;
	dados->mapa = (MAPA*)malloc(sizeof(MAPA) * tamanhoMapa * tamanhoMapa);
	CopyMemory(dados->mapa, &aux, sizeof(dados->mapa));
	mostraMapa(dados);
	_tprintf(TEXT("\n[MAPA] Mapa lido com sucesso!\n"));
}

void mostraMapa(DADOS* dados) {
	for (int i = 0; i < tamanhoMapa * tamanhoMapa; i++) {
		dados->mapa[i].caracter = shared[i].caracter;
		_tprintf(TEXT("%c"), dados->mapa[i].caracter);
	}

	return;
}

DWORD WINAPI ThreadAtualizaMapa(LPVOID param) {
	DADOS* dados = ((DADOS*)param);

	atualizaMap = CreateEvent(NULL, TRUE, FALSE, EVENT_ATUALIZAMAP);
	if (atualizaMap == NULL) {
		_tprintf(TEXT("\n[ERRO] Erro ao criar Evento!\n"));
		return 0;
	}
	SetEvent(atualizaMap);
	Sleep(500);
	ResetEvent(atualizaMap);

	while (1) {
		WaitForSingleObject(atualizaMap, INFINITE);

		if (dados->terminar)
			return 0;

		system("cls");
		_tprintf(TEXT("\n[ATUALIZAÇÃO] Atualizei o Mapa!\n"));

		CopyMemory(dados->mapa, shared, sizeof(dados->mapa));
		mostraMapa(dados);
		Sleep(3000);
	}
	Sleep(500);

	ExitThread(0);
}

DWORD WINAPI ThreadSair(LPVOID param) {
	TCHAR op[TAM];
	DADOS* dados = ((DADOS*)param);

	do {
		_fgetts(op, TAM, stdin);
		op[_tcslen(op) - 1] = '\0';
		WaitForSingleObject(hMutex, INFINITE);
	} while (_tcscmp(op, TEXT("fim")));

	dados->terminar = 1;
	ReleaseMutex(hMutex);

	Sleep(1000);

	ExitThread(0);
}