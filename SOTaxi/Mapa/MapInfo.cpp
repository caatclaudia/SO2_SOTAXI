#include "MapInfo.h"

int _tmain(int argc, TCHAR argv[]) {
	HANDLE hThreadSair, hThreadAtualizaMapa;
	DADOS dados;
	dados.terminar = 0;

#ifdef UNICODE
	_setmode(_fileno(stdin), _O_WTEXT);
	_setmode(_fileno(stdout), _O_WTEXT);
#endif

	inicializaVariaveis();

	hMutex = CreateMutex(NULL, FALSE, NOME_MUTEXMAPA);
	if (hMutex == NULL) {
		_tprintf(TEXT("\n[ERRO] Erro ao criar Mutex!\n"));
		return -1;
	}
	WaitForSingleObject(hMutex, INFINITE);
	ReleaseMutex(hMutex);

	EspMapa = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(dados.mapa), SHM_NAME);
	if (EspMapa == NULL)
	{
		_tprintf(TEXT("\n[ERRO] Erro ao criar FileMapping!\n"));
		return -1;
	}

	shared = (MAPA*)MapViewOfFile(EspMapa, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(dados.mapa));
	if (shared == NULL)
	{
		_tprintf(TEXT("\n[ERRO] Erro em MapViewOfFile!\n"));
		CloseHandle(EspMapa);
		return -1;
	}

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
	return 0;
}

void inicializaVariaveis() {
	HKEY chave;
	DWORD queAconteceu, tamanho, opcao;
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
			_tscanf_s(_T("%c"), &op);
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

		RegCloseKey(chave);
	}

	return;
}

void recebeMapa(DADOS* dados) {
	MAPA* aux=NULL;
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