#include "terminal.h"

#define MAX_AMOUNT 1000.0

static void str_date(int mday, int mon, int year, char* string_date);
static void _Exdate_char_to_int(uint32* mon, uint32* year, const uint8* const string_date);
static void _Trdate_char_to_int(uint32* mon, uint32* year, const uint8* const string_date);


/**
 * @brief : this function check luhn number on PAN is TERM_OK or not.
 * @param : ST_cardData_t *cardData to get PAN number
 * @return : TERM_OK if the luhn exist else return INVALID_CARD
 */
EN_terminalError_t isValidCardPAN(ST_cardData_t* cardData)
{
	EN_terminalError_t Ret_Val = INVALID_CARD;
	uint8 i = 0;
	uint8 mul = 0;
	uint8 waight = 0;
	while (cardData->primaryAccountNumber[i] != '\0')
	{
		if (!(i % 2)) { mul = (cardData->primaryAccountNumber[i++] - 48) * 2; }		// 0,2,4,6,...
		else          { mul = (cardData->primaryAccountNumber[i++] - 48) * 1; }		// 1,3,5,7,...

		if (mul > 9) {		/* must check if the multiplcation > 9 if mul = 10 --> get mul 1 if 11 -> 2 ... */
			mul -= 9;
		}
		waight += mul;
	}

	if (!(waight % 10)) { Ret_Val = TERM_OK;      }
	else                { Ret_Val = INVALID_CARD; }

	return Ret_Val;
}

/**
 * @brief : function to get the time from the system.
 * @param : (ST_terminalData_t*) to save the result in it.
 * @return : state_
 */
EN_terminalError_t getTransactionDate(ST_terminalData_t* termData)
{
	uint8 Ret_Val = WRONG_DATE;
	
	time_t ll_time;
	struct tm* ptr_current_time;
	time(&ll_time);
	ptr_current_time = localtime(&ll_time);
	str_date(ptr_current_time->tm_mday, ptr_current_time->tm_mon, ptr_current_time->tm_year, termData->transactionDate);

	//printf("\n-> %s\n", termData->transactionDate);		//test

	Ret_Val = TERM_OK;

	return Ret_Val;
}

/**
 * @brief : function to converse the date to ascii characters.
 * @param : mday has store the day number in the month begin 1:31.
 * @param : mon has store the mounth number in the year begin 0:11.
 * @param : year has store the year begin 1900.
 * @return : void
 */
static void str_date(int mday, int mon, int year, char* string_date) {
	/************ Day conversion to stringe ************/
	if (mday < 10)
	{
		string_date[0] = '0';
	}
	else
	{
		string_date[0] = (mday / 10) % 10 + 48;	//'0' = 48 decimal.
	}
	string_date[1] = mday % 10 + 48;
	/************ End of day conversion ************/

	string_date[2] = '/';		//Stored bar

	/************ month conversion to stringe ************/
	mon++;
	if (mon < 10)
	{
		string_date[3] = '0';
	}
	else
	{
		string_date[3] = (mon / 10) % 10 + 48;	//'0' = 48 decimal.
	}
	string_date[4] = mon % 10 + 48;
	/************ End of month conversion ************/

	string_date[5] = '/';		//Stored bar

	/************ Year conversion to stringe ************/
	year += 1900;
	string_date[6] = year / 1000 % 10 + 48;
	string_date[7] = year / 100 % 10 + 48;
	string_date[8] = year / 10 % 10 + 48;
	string_date[9] = year % 10 + 48;

	/************ End of month conversion ************/
	string_date[10] = '\0';		//Put end of string terminal.

}

/**
 * @brief : function to compare the expired date and current date.
 *
 * @return : state about expired date.
 */
EN_terminalError_t isCardExpired(const ST_cardData_t* cardData, const ST_terminalData_t* termData)
{
	int Ret_Val = WRONG_EXP_DATE;
	uint32 Expiration_Year = 0;
	uint32 Expiration_Mon = 0;
	uint32 Transaction_Year = 0;
	uint32 Transaction_Mon = 0;

	_Exdate_char_to_int(&Expiration_Mon, &Expiration_Year, cardData->cardExpirationDate);

	_Trdate_char_to_int(&Transaction_Mon, &Transaction_Year, termData->transactionDate);

	if (Transaction_Year < Expiration_Year)
	{
		Ret_Val = TERM_OK;
	}
	else if (Transaction_Year == Expiration_Year)
	{
		if (Transaction_Mon <= Expiration_Mon) {
			Ret_Val = TERM_OK;
		}
	}
	return Ret_Val;
}
/**
 * @brief : function to convesion the expire mounth and expire year to integer number.
 */
static void _Exdate_char_to_int(uint32* mon, uint32* year, const uint8* const string_date)
{
	*mon = (string_date[0] - 48) * 10 + (string_date[1] - 48);				// 12/24 -- mm/yy 
	*year = (string_date[3] - 48) * 10 + (string_date[4] - 48);
}

/**
 * @brief : function to convesion the transaction mounth and transaction year to integer number.
 */
static void _Trdate_char_to_int(uint32* mon, uint32* year, const uint8* const string_date)
{
	*mon = (string_date[3] - 48) * 10 + (string_date[4] - 48);				// 12/24 -- mm/yy 
	*year = (string_date[8] - 48) * 10 + (string_date[9] - 48);
}


/**
 * @brief : function to get transaction amount from the user and save in termData variable.
 */
EN_terminalError_t getTransactionAmount(ST_terminalData_t* termData)
{
	int Ret_Val = INVALID_AMOUNT;
	printf("Please enter transaction amount : ");
	scanf_s("%f", &termData->transAmount);
	if (termData->transAmount > 0.0)
	{
		Ret_Val = TERM_OK;
	}
	else
	{
		termData->transAmount = 0.0;
	}
	return Ret_Val;
}


/**
 * @brief : check the amount below the max or not.
 */
EN_terminalError_t isBelowMaxAmount(ST_terminalData_t* termData)
{
	int Ret_Val = EXCEED_MAX_AMOUNT;
	if (termData->transAmount <= termData->maxTransAmount)
	{
		Ret_Val = TERM_OK;
	}
	return Ret_Val;
}

/**
 * @brief : set the max amount equal 1000Egp.
 */
EN_terminalError_t setMaxAmount(ST_terminalData_t* termData)
{
	int Ret_Val = INVALID_MAX_AMOUNT;
	termData->maxTransAmount = MAX_AMOUNT;
	if (termData->maxTransAmount > 0.0)
	{
		Ret_Val = TERM_OK;
	}
	return Ret_Val;
}