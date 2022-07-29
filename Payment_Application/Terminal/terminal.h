#ifndef _TERMINAL_H_
#define _TERMINAL_H_

#include <stdio.h>
#include <time.h>
#include <string.h>
#include "../Platform_Types.h"
#include "../Card/card.h"

typedef struct ST_terminalData_t
{
	uint8 transactionDate[11];
	float transAmount;
	float maxTransAmount;
}ST_terminalData_t;

typedef enum EN_terminalError_t
{
	TERM_OK,
	WRONG_DATE,
	EXPIRED_CARD,
	INVALID_CARD,
	INVALID_AMOUNT,
	EXCEED_MAX_AMOUNT,
	INVALID_MAX_AMOUNT
}EN_terminalError_t;

EN_terminalError_t getTransactionDate(ST_terminalData_t* termData);

EN_terminalError_t isCardExpired(const ST_cardData_t* cardData, const ST_terminalData_t* termData);

EN_terminalError_t isValidCardPAN(ST_cardData_t* cardData);

EN_terminalError_t getTransactionAmount(ST_terminalData_t* termData);

EN_terminalError_t isBelowMaxAmount(ST_terminalData_t* termData);

EN_terminalError_t setMaxAmount(ST_terminalData_t* termData);

#endif //_TERMINAL_H_