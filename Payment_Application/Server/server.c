#include "server.h"
#include <stdlib.h>
#include <math.h>


static uint32 PAN_lenght(uint8* scan_str);

static void ftoa(float n, char* str, int afterpoint);
static uint32 intToStr(int x, char str[], int d);
static void reverse(char* str, int len);

static uint8 transLen(uint8* str);

unsigned char current_account_str[40] = { '\0' }; // act as repository all server can reed this array.

/**
 * @brief :function to cheak account is valid , have balance and save the transaction at end
 * @param :transData
 * @return :state of transaction.
 */
EN_transState_t recieveTransactionData(ST_transaction_t* transData)
{
	unsigned char balance_str[20] = { '\0' };

	/************ This function check PAN validation ************/
	EN_serverError_t Server_Ret_Val = isValidAccount(&(transData->cardHolderData));
	if (Server_Ret_Val == DECLINED_STOLEN_CARD)				//PAN doesn't exist.
	{
		//printf("inValidAccount\n");
		Server_Ret_Val = DECLINED_STOLEN_CARD;
	}
	else													//PAN is OK
	{
		//printf("ValidAccount\n");

		/************ This function check amount available ************/
		Server_Ret_Val = isAmountAvailable(&(transData->terminalData));
		if (Server_Ret_Val == LOW_BALANCE)					//Amount doesn't available
		{
			Server_Ret_Val = DECLINED_INSUFFECIENT_FUND;
			//printf("Amount Not Available\n");			//To test
		}
		else												//Amount available
		{
			Server_Ret_Val = APPROVED;						//To test
			//printf("Amount Available\n");;
		}
	}

	/* transState save the terminal state before, and now add the transaction state in the server */
	transData->transState = Server_Ret_Val;

	saveTransaction(transData);
	
	/* Update_accounts not work without the terminal and the server is APROVED */
	if (transData->transState == APPROVED) {
		Update_accounts(transData);							//Insert the new change in database.
	}

	return Server_Ret_Val;
}

/*
 * @brief :This function will take card data and validate these data.
 * @param :It checks if the PAN exists or not in the server's database.
 * @Return :If the PAN doesn't exist will return DECLINED_STOLEN_CARD, else will return OK
 */
EN_serverError_t isValidAccount(ST_cardData_t* cardData)
{
	EN_serverError_t Ret_Val = DECLINED_STOLEN_CARD;

	unsigned char counter = 0;

	FILE* Accounts_DB = (FILE *) fopen("Accounts_DB.txt", "r");
	while (!feof(Accounts_DB)) {		//return 1 whene reach EOF.
		fgets(current_account_str, 40, Accounts_DB);

		while (current_account_str[counter] != '-')
		{
			if (current_account_str[counter] == cardData->primaryAccountNumber[counter])
			{
				Ret_Val = SERVER_OK;
			}
			else
			{
				Ret_Val = DECLINED_STOLEN_CARD;
				break;
			}
			counter++;
		}
		counter = 0;
		if (Ret_Val == SERVER_OK)
		{
			break;
		}
	}
	fclose(Accounts_DB);
	return Ret_Val;
}

/**
 * @brief : This function will take terminal data and validate these data.
 * @param : It checks if the transaction's amount is available or not.
 *          If the transaction amount is greater than the balance in the database will
 * @return : LOW_BALANCE, else will return OK.
 */

EN_serverError_t isAmountAvailable(ST_terminalData_t* termData)
{
	EN_serverError_t Ret_Val = LOW_BALANCE;

	unsigned char balance_str[20] = { '\0' };

	float balance_num = 0.0;
	unsigned char counter = 0;

	strncpy(balance_str, current_account_str + PAN_lenght(current_account_str), 20);		//split balance from scan full one line from file.
	balance_num = (float) atof(balance_str);

	if (balance_num < termData->transAmount)
	{
		Ret_Val = LOW_BALANCE;
	}
	else
	{

		Ret_Val = SERVER_OK;
	}
	return Ret_Val;
}

static uint32 PAN_lenght(uint8* scan_str)		//to return lenght of PAN chars.
{
	unsigned char counter = 0;
	while (scan_str[counter] != '-')
	{
		counter++;
	}
	return (counter + 2);
}


/**
 * @brief It gives a sequence number to a transaction, this number is incremented once a
 *        transaction is processed into the server.
 * @param  ST_transaction_t pointer transData
 * @return (Ret_Val) EN_serverError_t for server Status
 * @retval for server Status OK or have update problem
 */
EN_serverError_t saveTransaction(ST_transaction_t* transData)
{
	EN_serverError_t server_state = SERVER_OK;

	unsigned char Transaction_str[65] = { '\0' };
	FILE* Transactions_DB = fopen("Transactions_DB.txt", "a+");					// intialize Transactions_DB as pointer to file in append and read mode.

	transData->transactionSequenceNumber = 0;
	while (!feof(Transactions_DB)) {		//return 1 whene reach EOF.
		fgets(Transaction_str, 65, Transactions_DB);
		transData->transactionSequenceNumber ++;
	}

	fprintf(Transactions_DB, "%d-PAN %s-at %s-val %0.2f-State %d\n", 
		transData->transactionSequenceNumber,
		transData->cardHolderData.primaryAccountNumber,
		transData->terminalData.transactionDate,
		transData->terminalData.transAmount,
		transData->transState
	);
	
	fclose(Transactions_DB);

	return server_state;
}
/**
 * @brief This function take the balance from current_account_str and modified it in database.
 * @param  ST_transaction_t pointer termData to take the amount
 * @param  (Ret_Val) EN_serverError_t for server Status
 * @retval for server Status OK or have update problem
 */
EN_serverError_t Update_accounts(ST_transaction_t* termData)
{
//	EN_transState_t Ret_Val = ;
	/*********************************** update current balance *************************************/
	unsigned char balance_str[20] = { '\0' };
	float balance_num = 0.0;

	strncpy(balance_str, current_account_str + PAN_lenght(current_account_str), 20);	//split balance to modified
	balance_num = (float) atof(balance_str);													//convert the balance form string to float

	balance_num -= termData->terminalData.transAmount;									//discound the amount from total balance.

	ftoa(balance_num, balance_str, 2);													//convert the new balance form float to string

	strncpy(current_account_str + PAN_lenght(current_account_str), balance_str, 20);	//apply the modified data to account string
	/***********************************************************************************************/
	/**************** update the account file data base in new file as a temp file******************/
	FILE* Accounts_DB = fopen("Accounts_DB.txt", "r");
	FILE* Temp_Accouunts = fopen("Account_temp.txt", "w");

	unsigned char temp_str[40] = { '\0' };
	char wanted_Pan = 0;

	int counter = 0;
	while (!feof(Accounts_DB))
	{//return 1 whene reach EOF.
		fgets(temp_str, 40, Accounts_DB);			//read from Accounts_DB.txt

		while (temp_str[counter] != '-')	// get wanted pan
		{
			if (temp_str[counter] == termData->cardHolderData.primaryAccountNumber[counter])
			{
				wanted_Pan = 1;
			}
			else
			{
				wanted_Pan = 0;
				break;
			}
			counter++;
		}
		counter = 0;

		if (wanted_Pan == 1)
		{
			fprintf(Temp_Accouunts, "%s\n", current_account_str);		//write in temp file.
		}
		else
		{
			fprintf(Temp_Accouunts, "%s", temp_str);		//write in temp file
		}
	}
	fclose(Temp_Accouunts);
	fclose(Accounts_DB);
	/********************** take a copy from the temp file to account database file ******************/
	Accounts_DB = fopen("Accounts_DB.txt", "w");
	Temp_Accouunts = fopen("Account_temp.txt", "r");

	char ch = 0;
	while (1)
	{
		ch = fgetc(Temp_Accouunts);
		if (ch == EOF) break;

		fputc(ch, Accounts_DB);
	}

	fputc(EOF, Accounts_DB);	// to remove the end newline from the file

	fclose(Temp_Accouunts);
	fclose(Accounts_DB);

}

/**
 * @brief :change float number to string.
 * @param :float number n
 * @param :pointer to char to store the result
 * @param :number of digit after the point
 * referance :https://tutorialspoint.dev/language/c/convert-floating-point-number-string.
 */
static void ftoa(float n, char* str, int afterpoint)
{
	int intpart = (int)n;

	float floatpart = n - ((float)intpart);

	int i = intToStr(intpart, str, 0);	// i store number of int digits

	str[i] = '.'; // add dot

	floatpart *= (float)pow(10, afterpoint);	// float part stord num.0000000

	intToStr((int)floatpart, str + i + 1, afterpoint);
}

static uint32 intToStr(int x, char str[], int d)
{
	int i = 0;
	while (x) {
		str[i++] = (x % 10) + '0';
		x = x / 10;
	}

	while (i < d)
		str[i++] = '0';

	reverse(str, i);
	str[i] = '\0';
	return i;
}

static void reverse(char* str, int len)
{
	int i = 0;
	int j = len - 1;
	while (i < j) {
		str[i] ^= str[j];
		str[j] ^= str[i];
		str[i] ^= str[j];
		i++;
		j--;
	}
}

/**
 * @brief : get transaction details for wanted transaction from file act a database for
 *          all transaction, this details save in variable in ST_transaction_t type.
 * @param : transactionSequenceNumber - passing by user to choose the transaction
 * @param : ST_transaction_t* transData to fill the variable by transaction details.
 * @return : error_state 
 */ 
EN_serverError_t getTransaction(uint32 transactionSequenceNumber, ST_transaction_t* transData)
{
	EN_serverError_t Ret_Val = SERVER_OK;
	FILE* Transactions_DB = fopen("Transactions_DB.txt", "r");
	uint8 Transaction_str[65] = { '\0' };
	uint8 amount_str[8] = { '\0' };
	uint8 transState_str[2] = {'\0'};
	uint32 i = 0;
	for (i = 0; i < transactionSequenceNumber; i++)
	{
		fgets(Transaction_str, 65, Transactions_DB);
		if (feof(Transactions_DB))
		{
			Ret_Val = TRANSACTION_NOT_FOUND;
			break;
		}
	}
	if (Ret_Val == SERVER_OK)
	{
		uint32 i = 0;
		transData->transactionSequenceNumber = transactionSequenceNumber;
		for (i = 0; i < 20; i++) {
			transData->cardHolderData.primaryAccountNumber[i] = '\0';
		}
		for (i = 0; i < 11; i++) {
			transData->terminalData.transactionDate[i] = '\0';
		}
		
		uint8 seq_lenght = transLen(Transaction_str);
		uint8 pan_lenght = transLen(Transaction_str + seq_lenght + 5);
		uint8 date_lenght = 10;
		uint8 amount_lenght = transLen(Transaction_str + seq_lenght + 5 + pan_lenght + 4 + date_lenght + 5 );

		strncpy(transData->cardHolderData.primaryAccountNumber, Transaction_str + seq_lenght + 5 , pan_lenght);

		strncpy(transData->terminalData.transactionDate, Transaction_str + seq_lenght + 5 + pan_lenght + 4, 10);

		strncpy(amount_str, Transaction_str + seq_lenght + 5 + pan_lenght + 4 + date_lenght + 5, amount_lenght);	//source : transLen(Transaction_str + 7) + 7 + 4 + 15

		transData->terminalData.transAmount = ((float) atof(amount_str));

		strncpy(transState_str, Transaction_str + seq_lenght + 5 + pan_lenght + 4 + date_lenght + 5 + amount_lenght + 7,1);
		
		/* transState */
		transData->transState = transState_str[0] - 48;
	}
	fclose(Transactions_DB);
	return Ret_Val;
}
static uint8 transLen(uint8* str)
{
	uint8 i = 0;
	while (str[i] != '-' && str[i] != '\0') {
		i++;
	}
	return i;
}