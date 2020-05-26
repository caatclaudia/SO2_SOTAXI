#include "Header.h"

void comunicacaoParaCentral(DADOS* dados) {
	CopyMemory(dados->shared, dados->taxi, sizeof(TAXI));
	return;
}

void avisaNovoTaxi(DADOS* dados) {
	comunicacaoParaCentral(dados);

	SetEvent(dados->novoTaxi);
	Sleep(500);
	ResetEvent(dados->novoTaxi);

	/*if (WaitForSingleObject(dados->respostaAdmin, 5000) == WAIT_TIMEOUT)
		dados->taxi->terminar = 1;
	else*/
		WaitForSingleObject(dados->respostaAdmin, INFINITE);
		CopyMemory(dados->taxi, dados->shared, sizeof(TAXI));

	return;
}

void recebeInfo(DADOS* dados) {
	TAXI* novo;
	novo = (TAXI*)malloc(sizeof(TAXI));

	CopyMemory(novo, dados->shared, sizeof(TAXI));
	if (!_tcscmp(novo->matricula, dados->taxi->matricula)) {
		dados->taxi = novo;
		if (dados->taxi->disponivel)
			_tprintf(_T("\n[PASS] Taxi entregou o Passageiro!"));
		else
			_tprintf(_T("\n[PASS] Taxi recolheu o Passageiro!"));
	}

	return;
}

void avisaTaxiSaiu(DADOS* dados) {
	SetEvent(dados->saiuTaxi);
	ResetEvent(dados->saiuTaxi);

	comunicacaoParaCentral(dados);

	SetEvent(dados->saiuTaxi);
	Sleep(500);
	ResetEvent(dados->saiuTaxi);

	return;
}

void avisaMovimentoTaxi(DADOS* dados) {
	comunicacaoParaCentral(dados);

	SetEvent(dados->movimentoTaxi);
	Sleep(500);
	ResetEvent(dados->movimentoTaxi);
	return;
}
