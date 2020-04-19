#include <windows.h>
#include <tchar.h>
#include <fcntl.h>
#include <io.h>
#include <stdio.h>
#include <stdlib.h>

#define NQ_INICIAL 10
#define TAM 200

//ConTaxi
//1 instancia por taxi
//Movimenta taxi aleatoriamente
//MOSTRA: NOVO PASSAGEIRO, INTERESSE EM TRANSPORTAR ENVIADO, CONFIRMACAO RECEBIDA, RECOLHA DE PASSAGEIRO INICIADA, PASSAGEIRO RECOLHIDO, PASSAGEIRO ENTREGUE

typedef struct {
	TCHAR matricula[6];
	unsigned int X, Y, Xfinal, Yfinal;
	int disponivel;
	TCHAR idPassageiro[TAM];
	float velocidade;
	int autoResposta;
	int interessado;
	int terminar;
} TAXI;

unsigned int NQ=NQ_INICIAL;

void ajuda();
void inicializaTaxi(TAXI *taxi);

int _tmain() {
	TCHAR op[TAM];
	TAXI taxi;

#ifdef UNICODE
	_setmode(_fileno(stdin), _O_WTEXT);
	_setmode(_fileno(stdout), _O_WTEXT);
#endif

	inicializaTaxi(&taxi);

	do {
		_tprintf(_T("\n>>"));
		_fgetts(op, TAM, stdin);
		op[_tcslen(op) - 1] = '\0';
		if (_tcscmp(op, TEXT("aumentaV"))) {		//AUMENTA 0.5 DE VELOCIDADE
			taxi.velocidade += 0.5;
		}
		else if (_tcscmp(op, TEXT("diminuiV"))) {		//DIMINUI 0.5 DE VELOCIDADE
			if(taxi.velocidade>=0.5)
				taxi.velocidade -= 0.5;
		}
		else if (_tcscmp(op, TEXT("numQuad"))) {		//DEFINIR NQ
			_tprintf(_T("\nValor de NQ (número de quadriculas até ao passageiro) : "));
			_tscanf_s(_T("%d"), &NQ);
			if (NQ < 0)
				NQ = NQ_INICIAL;
		}
		else if (_tcscmp(op, TEXT("respostaAuto"))) {		//LIGAR/DESLIGAR RESPOSTA AUTOMÁTICA AOS PEDIDOS DE TRANSPORTE
			if (taxi.autoResposta)
				taxi.autoResposta = 0;
			else
				taxi.autoResposta = 1;
		}
		else if (_tcscmp(op, TEXT("pass"))) {		//TRANSPORTAR PASSAGEIRO
			if (!taxi.autoResposta)
				taxi.interessado = 1;
		}
		else if (_tcscmp(op, TEXT("ajuda"))) {		//AJUDA NOS COMANDOS
			ajuda();
		}
	} while (_tcscmp(op, TEXT("fim")));

	taxi.terminar = 1;

	return 0;
}

void ajuda() {
	_tprintf(_T("\n\n aumentaV - AUMENTA 0.5 DE VELOCIDADE"));
	_tprintf(_T("\n diminuiV - DIMINUI 0.5 DE VELOCIDADE"));
	_tprintf(_T("\n numQuad - DEFINIR NQ"));
	_tprintf(_T("\n respostaAuto - LIGAR/DESLIGAR RESPOSTA AUTOMÁTICA AOS PEDIDOS DE TRANSPORTE"));
	_tprintf(_T("\n pass - TRANSPORTAR PASSAGEIRO"));
	_tprintf(_T("\n fim - ENCERRAR TODO O SISTEMA"));
	return;
}

void inicializaTaxi(TAXI* taxi) {
	int num;

	do {
		num = 0;
		_tprintf(_T("\n Matricula do Táxi: "));		
		_tscanf_s(_T("%s"), taxi->matricula, sizeof(taxi->matricula));
		for (int i = 0; i < 6; i++)
			if (isalpha(taxi->matricula[i]))
				num++;
	} while (num!=2);

	_tprintf(_T("\n Localizacao do Táxi (X Y) : "));
	_tscanf_s(_T("%d"), &taxi->X);
	_tscanf_s(_T("%d"), &taxi->Y);
	taxi->disponivel = 1;
	taxi->velocidade = 1;
	taxi->autoResposta = 1;
	taxi->interessado = 0;
	taxi->terminar = 0;
	taxi->Xfinal = 0;
	taxi->Yfinal = 0;

	//VAI AO ADMIN VER SE PODE CRIAR	
	//VERIFICAR QUE É UNICA

	return;
}