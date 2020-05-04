#include <Windows.h>
#include <tchar.h>
#include <math.h>
#include <stdio.h>
#include <fcntl.h>
#include <conio.h>
#include <io.h>

#define TAM 50
#define WAITTIMEOUT 1000
#define SHM_NAME TEXT("EspacoMapa")
#define EVENT_ATUALIZAMAP TEXT("AtualizaMapa")
#define NOME_MUTEXMAPA TEXT("MutexMapa")

typedef struct {
	char caracter;
} MAPA;

typedef struct {
	MAPA mapa[TAM][TAM];
	int terminar;
} DADOS;

#define PATH TEXT("..\\mapa.txt")

HANDLE EspMapa;	//FileMapping
char* shared;
HANDLE atualizaMap;
HANDLE hMutex;

void inicializaVariaveis();
void recebeMapa(DADOS* dados);
void mostraMapa(DADOS* dados);
DWORD WINAPI ThreadPrincipal(LPVOID param);
DWORD WINAPI ThreadAtualizaMapa(LPVOID param);
DWORD WINAPI ThreadSair(LPVOID param);

int _tmain(int argc, TCHAR argv[]) {
	HANDLE hThreadSair, hThreadPrincipal, hThreadAtualizaMapa;
	DADOS dados;
	TCHAR nome[100] = PATH;
	int x = 0, y = 0;
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

	EspMapa = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(char) * TAM * TAM, SHM_NAME);
	if (EspMapa == NULL)
	{
		_tprintf(TEXT("\n[ERRO] Erro ao criar FileMapping!\n"));
		return -1;
	}

	shared = (char*)MapViewOfFile(EspMapa, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(char) * TAM * TAM);
	if (shared == NULL)
	{
		_tprintf(TEXT("\n[ERRO] Erro em MapViewOfFile!\n"));
		CloseHandle(EspMapa);
		return -1;
	}

	recebeMapa(&dados);

	hThreadPrincipal = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ThreadPrincipal, (LPVOID)&dados, 0, NULL);
	if (hThreadPrincipal == NULL) {
		_tprintf(TEXT("\n[ERRO] Erro ao lançar Thread!\n"));
		return 0;
	}

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

	HANDLE ghEvents[3];
	ghEvents[0] = hThreadPrincipal;
	ghEvents[1] = hThreadAtualizaMapa;
	ghEvents[2] = hThreadSair;
	WaitForMultipleObjects(3, ghEvents, TRUE, INFINITE);
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
	char* aux = (char*)malloc(sizeof(char) * 50 * 52);

	CopyMemory(aux, shared, sizeof(char) * 50 * 52);
	int x = 0, y = 0;
	for (int i = 0; i < TAM * TAM; i++) {
		dados->mapa[y][x].caracter = aux[i];
		if (aux[i] == '\n') {
			x = 0;
			y++;
		}
		else {
			x++;
		}
	}
	_tprintf(TEXT("\n[MAPA] Mapa lido com sucesso!\n"));
	for (int x = 0; x < TAM - 1; x++) {
		for (int y = 0; y < TAM; y++)
			_tprintf(TEXT("%c"), dados->mapa[x][y].caracter);
		_tprintf(TEXT("\n"));
	}
}

void mostraMapa(DADOS* dados) {
	for (int x = 0; x < TAM - 2; x++) {
		for (int y = 0; y < TAM; y++)
			_tprintf(TEXT("%c"), dados->mapa[x][y].caracter);
		_tprintf(TEXT("\n"));
	}

	return;
}

DWORD WINAPI ThreadPrincipal(LPVOID param) {
	DADOS* dados = ((DADOS*)param);

	while (!dados->terminar) {
		system("cls");
		mostraMapa(dados);

		Sleep(3000);
	}

	Sleep(500);

	ExitThread(0);
}

DWORD WINAPI ThreadAtualizaMapa(LPVOID param) {
	DADOS* dados = ((DADOS*)param);
	char* aux = (char*)malloc(sizeof(char) * 50 * 52);

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
		_tprintf(TEXT("\n[ATUALIZAÇÃO] Atualizei o Mapa!\n"));

		CopyMemory(aux, shared, sizeof(char) * TAM * TAM);
		int x = -1, y = -1;
		for (int i = 0; i < TAM * TAM; i++) {
			if (aux[i] == '\n') {
				x = 0;
				y++;
			}
			else {
				x++;
			}
			if (aux[i] != '\n') {
				dados->mapa[y][x].caracter = aux[i];
			}
		}
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