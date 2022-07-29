#ifndef _SERVER_H_
#define _SERVER_H_

#include "../Platform_Types.h"
#include "../Terminal/terminal.h"

typedef enum EN_transState_t
{
	APPROVED,
	DECLINED_INSUFFECIENT_FUND,
	DECLINED_STOLEN_CARD,
	INTERNAL_SERVER_ERROR
}EN_transState_t;

typedef enum EN_serverError_t
{
	SERVER_OK,
	SAVING_FAILED,
	TRANSACTION_NOT_FOUND,
	ACCOUNT_NOT_FOUND,
	LOW_BALANCE
}EN_serverError_t;

typedef struct ST_transaction_t
{
	ST_cardData_t cardHolderData;
	ST_terminalData_t terminalData;
	EN_transState_t transState;
	uint32 transactionSequenceNumber;
}ST_transaction_t;

typedef struct ST_accountsDB_t
{
	float balance;
	uint8 primaryAccountNumber[20];
}ST_accountsDB_t;


EN_transState_t recieveTransactionData(ST_transaction_t* transData);

EN_serverError_t isValidAccount(ST_cardData_t* cardData);

EN_serverError_t isAmountAvailable(ST_terminalData_t* termData);

EN_serverError_t saveTransaction(ST_transaction_t* transData);

EN_serverError_t getTransaction(uint32 transactionSequenceNumber, ST_transaction_t* transData);

/**
 * @brief This function take the balance from current_account_str and modified it in database.
 * @param  ST_transaction_t pointer termData to take the amount
 * @param  (Ret_Val) EN_serverError_t for server Status
 * @retval for server Status OK or have update problem
 */
EN_serverError_t Update_accounts(ST_transaction_t* termData);

#endif //_SERVER_H_
