#include "card.h"


/**
 * @brief : function to get card holder name from the user and save in cardData variable.
 */
EN_cardError_t getCardHolderName(ST_cardData_t* cardData)
{
	EN_cardError_t Ret_Val = WRONG_NAME;
	uint8 mask[30] = { '\0' };
	uint8 i = 0;

	printf("Please enter card holder's name : ");
	gets(mask);
	if (strlen(mask) >= 20 && strlen(mask) <= 24)
	{
		for (i = 0; i <= strlen(mask); i++)
		{
			cardData->cardHolderName[i] = mask[i];
		}
		Ret_Val = CARD_OK;
	}

	return Ret_Val;
}

/**
 * @brief : function to get card expiry date from the user and save in cardData variable.
 */
EN_cardError_t getCardExpiryDate(ST_cardData_t* cardData)
{
	uint8 Ret_Val = WRONG_EXP_DATE;
	uint8 mask[30] = { '\0' };
	uint8 i = 0;

	printf("Please enter card expiry date : ");
	gets(mask);
	if (strlen(mask) == 5)
	{
		for (i = 0; i <= 5; i++)
		{
			cardData->cardExpirationDate[i] = mask[i];
		}
		Ret_Val = CARD_OK;
	}

	return Ret_Val;
}

/**
 * @brief : function to get get card PAN from the user and save in cardData variable.
 */
EN_cardError_t getCardPAN(ST_cardData_t* cardData)
{
	uint8 Ret_Val = WRONG_PAN;
	uint8 mask[30] = { '\0' };
	uint8 i = 0;

	printf("Please enter card's Primary Account Number : ");
	gets(mask);
	if (strlen(mask) <= 19 && strlen(mask) >= 16)
	{
		for (i = 0; i <= strlen(mask); i++)
		{
			cardData->primaryAccountNumber[i] = mask[i];
		}
		Ret_Val = CARD_OK;
	}
	
	return Ret_Val;
}