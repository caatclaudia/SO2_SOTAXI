#include <windows.h>
#include <tchar.h>
#include <fcntl.h>
#include <io.h>
#include <stdio.h>
#include <stdlib.h>

#define TAM 200
#define MAX_PASS 20

//ConPass
//1 instancia
//Passageiros tem um id(assumir que já é unico)
//Indica à central a existência de novo passageiro que está em X, Y e pertende ir para X, Y
//Recebe da central : Táxi atribuido ao passageiro X; Táxi recolheu passageiro X; Táxi entregou o passageiro X

typedef struct {
	TCHAR id[TAM];
	unsigned int X, Y, Xfinal, Yfinal;
	int movimento;
	int terminar;
} PASSAGEIRO;

void novoPassageiro(PASSAGEIRO pass[], int atualPass);

int _tmain() {
	TCHAR op[TAM];
	PASSAGEIRO pass[MAX_PASS];
	int atualPass = 0;

#ifdef UNICODE
	_setmode(_fileno(stdin), _O_WTEXT);
	_setmode(_fileno(stdout), _O_WTEXT);
#endif

	do {
		_tprintf(_T("\n>>"));
		_fgetts(op, TAM, stdin);
		op[_tcslen(op) - 1] = '\0';
		if (_tcscmp(op, TEXT("novo"))) {		//NOVO PASSAGEIRO
			novoPassageiro(&pass[0], atualPass);
			atualPass++;
		}
	} while (_tcscmp(op, TEXT("fim")));

	for (int i = 0; i < MAX_PASS; i++)
		pass[i].terminar = 1;

	return 0;
}

void novoPassageiro(PASSAGEIRO pass[], int atualPass) {
	
	_tprintf(_T("\n Id do Passageiro: "));
		_fgetts(pass[atualPass].id, TAM, stdin);
		pass[atualPass].id[_tcslen(pass[atualPass].id) - 1] = '\0';

	_tprintf(_T("\n Localizacao do Passageiro (X Y) : "));
	_tscanf_s(_T("%d"), &pass[atualPass].X);
	_tscanf_s(_T("%d"), &pass[atualPass].Y);

	_tprintf(_T("\n Local de destino do Passageiro (X Y) : "));
	_tscanf_s(_T("%d"), &pass[atualPass].Xfinal);
	_tscanf_s(_T("%d"), &pass[atualPass].Yfinal);
	pass[atualPass].movimento = 0;
	pass[atualPass].terminar = 0;

	//VAI AO ADMIN VER SE PODE CRIAR

	return;
}