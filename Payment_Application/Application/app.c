#include "app.h"


int main()
{
	uint32 mode = 255;
	
	printf("-------------------- welcame to 22mn application --------------------\n");

	while (1) {
		printf("---------------------------------------------------------------------\n");
		printf("Enter (0) to select -> Exit.\n");
		printf("Enter (1) to select -> Transaction mode.\n");
		printf("Enter (2) to select -> Get old transaction mode.\n");
		printf("Enter (3) to select -> Administrator mode.\n");
		printf("---------------------------------------------------------------------\n");
		printf("Enter the mode you want : ");
		scanf("%d", &mode);
		
		/**
		 *To make the input buffer is emptyand avoid any error when I use any input function next.
		 */
		getchar();		
	
		printf("---------------------------------------------------------------------\n");

		switch (mode) {
		case 1:
			appStart();
			break;
		case 2:
			getTransaction_mode();
			break;
		case 3:
			administrator_mode();
			break;
		case 0:
			printf("---------------------- Goodbye, see you later -----------------------\n");
			return 0;
		}
	}
	
	return 0;
}
/**
 * @brief : function to get transaction Sequence from the user and pass the Sequence 
 *          for getTransaction function and print the result from getTransaction function.
 */
void getTransaction_mode(void)
{
	ST_transaction_t current_trans;
	uint32 transactionSequenceNumber;
	printf("Enter the transaction Sequence Number : ");
	scanf_s("%d", &transactionSequenceNumber);
	fflush(stdin);
	EN_serverError_t Ret_Val = getTransaction(transactionSequenceNumber, &current_trans);

	if (Ret_Val == SERVER_OK) {
		printf("Transaction : %d - PAN %s - at %s - val %0.2f\n", current_trans.transactionSequenceNumber,
			current_trans.cardHolderData.primaryAccountNumber,
			current_trans.terminalData.transactionDate,
			current_trans.terminalData.transAmount);
		if (current_trans.transState == 0) {
			printf("State       : APPROVED\n");
		}
		else
		{
			printf("State       : DECLINED\n");
		}

	}
	else {
		printf("Transaction : not found.\n");
	}
}

void administrator_mode(void) {
	ST_transaction_t current_trans;
	uint32 transactionSequenceNumber;
	printf("administrator_mode :: to get the reason for reject transaction.\n\n");
	printf("Enter the transaction Sequence Number : ");
	scanf_s("%d", &transactionSequenceNumber);
	fflush(stdin);
	EN_serverError_t Ret_Val = getTransaction(transactionSequenceNumber, &current_trans);

	if (Ret_Val == SERVER_OK) {
		printf("Transaction : %d - PAN %s - at %s - val %0.2f\n", 
			current_trans.transactionSequenceNumber,
			current_trans.cardHolderData.primaryAccountNumber,
			current_trans.terminalData.transactionDate,
			current_trans.terminalData.transAmount);

		/*************** transState analysis **********************/
		if (current_trans.transState == 0) {	// NO error

			printf("State       : APPROVED _ Successful transaction\n");
		}
		else
		{										// error
			printf("State       : DECLINED\n");
			printf("Reasone     : ");
			print_transaction_error(current_trans.transState);	
		}
	}
	else {										// non transaction Sequence Number
		printf("Transaction : not found.\n");
	}
}

void print_terminal_error(EN_terminalError_t error_detector) {

	if      (error_detector == WRONG_DATE)         { printf("WRONG_DATE");           }
	else if (error_detector == EXPIRED_CARD)       { printf("EXPIRED_CARD");         }
	else if (error_detector == INVALID_CARD)       { printf("ERROR_IN_LUHN_NUMBER"); }
	else if (error_detector == INVALID_AMOUNT)     { printf("INVALID_AMOUNT");       }
	else if (error_detector == EXCEED_MAX_AMOUNT)  { printf("EXCEED_MAX_AMOUNT");    }
	else if (error_detector == INVALID_MAX_AMOUNT) { printf("INVALID_MAX_AMOUNT");   }

	printf("\n");
}

void print_transaction_error(EN_terminalError_t error_detector) {

	if      (error_detector == DECLINED_INSUFFECIENT_FUND) { printf("BALANCE_NOT_ENOUGH");    }
	else if (error_detector == DECLINED_STOLEN_CARD)       { printf("STOLEN_CARD");           }
	else if (error_detector == INTERNAL_SERVER_ERROR)      { printf("INTERNAL_SERVER_ERROR"); }

	printf("\n");
}

/**
 * @brief : function to get interface the transction pross
 */
void appStart(void)
{
	/* Intialize all state error varible by error state value to not inter any if condition. */
	EN_cardError_t card_state = WRONG_PAN;
	EN_terminalError_t terminal_state = INVALID_AMOUNT;
	EN_serverError_t server_state = SAVING_FAILED;


	card_state = card_interfacing(&current_trans);
	if (card_state == CARD_OK)
	{
		terminal_state = terminal_interfacing((&current_trans));
		printf("---------------------------------------------------------------------\n");
		if (terminal_state != TERM_OK)
		{
			printf("ERRER transaction :: ");
			print_terminal_error(terminal_state);
		}
	}

	/*
	 * don't go to the server if the MAX = 0 or user inter amount = 0.
	 */
	if (terminal_state == TERM_OK)
	{
		server_state = server_interfacing((&current_trans));

		printf("Sequnace : %d - PAN %s - at %s - val %0.2f\n", current_trans.transactionSequenceNumber,
			current_trans.cardHolderData.primaryAccountNumber,
			current_trans.terminalData.transactionDate,
			current_trans.terminalData.transAmount);

		if (current_trans.transState == 0) {
			printf("State    : APPROVED\n");
		}
		else
		{
			printf("State    : DECLINED\n");
		}
	}
	else {
		printf("Invalid operation\n");
	}

	
	printf("\n====>> To anther service.\n");
}

/**
 * @brief : function to get all card data.
 */
EN_cardError_t card_interfacing(ST_transaction_t* current_trans)
{
	EN_cardError_t card_state;

	card_state = getCardHolderName(&(current_trans->cardHolderData));
	if (card_state == WRONG_NAME)		//if card_name is ok go to exp date 
	{
		printf("====>>  WRONG_NAME.\n");
	}
	else {
		//printf("====>>  RIGHT_NAME.\n");		    // TEST
		card_state = getCardExpiryDate(&(current_trans->cardHolderData));
		if (card_state == WRONG_EXP_DATE)		//if exp date os ok go to card pan
		{
			printf("====>>  WRONG_EXP_DATE.\n");
		}
		else {
			// printf("====>>  RIGHT_DATE.\n");		// TEST
			card_state = getCardPAN(&(current_trans->cardHolderData));
			if (card_state == WRONG_PAN)
			{
				printf("====>>  WRONG__PAN.\n");
			}
			else {
			//	printf("====>>  RIGHT_PAN.\n");		// TEST
			}
		}
	}
	return card_state;
}

/**
 * @brief : function to handle the terminal functions.
 */
EN_terminalError_t terminal_interfacing(ST_transaction_t* current_trans)
{
	EN_terminalError_t ret_terminal_state = TERM_OK;

	ret_terminal_state = isValidCardPAN(&(current_trans->cardHolderData));

	if (ret_terminal_state != INVALID_CARD)
	{
	//printf("====>>  VALID_LUHN_NUM.\n");				// for tester

	/************************ if luhn num existed go to getTransactionDate ************************/
		ret_terminal_state = getTransactionDate(&(current_trans->terminalData));
		if (ret_terminal_state != WRONG_DATE)
		{
			/********************************** check isCardExpired ***********************************/
			ret_terminal_state = isCardExpired(&(current_trans->cardHolderData), &(current_trans->terminalData));
			if (ret_terminal_state != EXPIRED_CARD)
			{
				//printf("VALID_CARD\n");				// for test
				/******************* if the card not expired go to check amount ***********************/
				ret_terminal_state = getTransactionAmount(&(current_trans->terminalData));
				if (ret_terminal_state != INVALID_AMOUNT)
				{
					//printf("====>>  VALID_AMOUNT.\n");			//for tester
					/***************************** first set max amount ***************************/

					ret_terminal_state = setMaxAmount(&(current_trans->terminalData));
					if (ret_terminal_state != INVALID_MAX_AMOUNT)
					{
						//printf("VALID_MAX_AMOUNT\n");				// for test
						/************ then check the amount blow the max amount or not ************/
						ret_terminal_state = isBelowMaxAmount(&(current_trans->terminalData));
						if (ret_terminal_state != EXCEED_MAX_AMOUNT)
						{
							//printf("AMOUNT_BELOW_MAX\n");				// for test

						} /* Check amount is blow max or not */

					} /* Check max amount not equal zero */

				} /* Check user amount not equal zero */

			} /* Check card expied date */

		} /* Get transaction date check */

	} /* Luhn number check */
	return ret_terminal_state;
}

/**
 * @brief : function to handle the server functions.
 */
EN_serverError_t server_interfacing(ST_transaction_t* current_trans)
{	

	EN_serverError_t server_state = recieveTransactionData(current_trans);
	return server_state;
}