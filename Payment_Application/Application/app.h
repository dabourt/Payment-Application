#ifndef _APP_H_
#define _APP_H_

#include "../Server/server.h"

void appStart(void);

void getTransaction_mode(void);

void administrator_mode(void);

EN_cardError_t card_interfacing(ST_transaction_t* current_trans);

EN_terminalError_t terminal_interfacing(ST_transaction_t* current_trans);

EN_serverError_t server_interfacing(ST_transaction_t* current_trans);

void print_terminal_error(EN_terminalError_t error_detector);

void print_transaction_error(EN_terminalError_t error_detector);


/* current_trans act a temporary storge, all function use it to pass variable and returned throw it */
ST_transaction_t current_trans;


#endif //_APP_H_