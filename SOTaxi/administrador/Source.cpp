#include <windows.h>
#include <tchar.h>
#include <fcntl.h>
#include <io.h>
#include <stdio.h>
#include <stdlib.h>

#define TAM 200

//CenTaxi
//1 instancia
//le o mapa e gere-o
//sabe dos taxis, posicoes e estado

void ajuda();

int _tmain(int argc, LPTSTR argv[]){
	TCHAR op[TAM];

#ifdef UNICODE
	_setmode(_fileno(stdin), _O_WTEXT);
	_setmode(_fileno(stdout), _O_WTEXT);
#endif

	do {
		_tprintf(_T("\n>>"));
		_fgetts(op, TAM, stdin);
		op[_tcslen(op) - 1] = '\0';
		if (_tcscmp(op, TEXT("expulsar"))) {		//EXPULSAR TAXI

		}
		else if (_tcscmp(op, TEXT("listar"))) {		//LISTAR TAXIS

		}
		else if (_tcscmp(op, TEXT("aceitacaoT"))) {		//PAUSAR/RECOMECAR ACEITAÇÃO DE TAXIS

		}
		else if (_tcscmp(op, TEXT("manifestacoes"))) {		//DEFINIR INTERVALO DE TEMPO DURANTE O QUAL AGUARDA MANIFESTAÇOES DOS TAXIS

		}
		else if (_tcscmp(op, TEXT("ajuda"))) {		//AJUDA NOS COMANDOS
			ajuda();
		}
	} while (_tcscmp(op, TEXT("fim")));

	_tprintf(_T("\nAdministrador vai encerrar!"));
	return 0;
}

void ajuda() {
	_tprintf(_T("\n\n expulsar - EXPULSAR TAXI"));
	_tprintf(_T("\n listar - LISTAR TÁXIS"));
	_tprintf(_T("\n aceitaçãoT - PAUSAR/RECOMECAR ACEITAÇÃO DE TAXIS"));
	_tprintf(_T("\n manifestações - DEFINIR INTERVALO DE TEMPO DURANTE O QUAL AGUARDA MANIFESTAÇOES DOS TAXIS"));
	_tprintf(_T("\n fim - ENCERRAR TODO O SISTEMA"));
	return ;
}