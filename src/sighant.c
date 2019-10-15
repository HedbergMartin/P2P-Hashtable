/*
* sighant - signal handler interface. Will take a signal and a signal handler
* function and send them with sigaction. SA_RESTART will be only flags applied
* to signal handler function.
*
* Author: Buster Hultgren Wärn <dv17bhn@cs.umu.se>
*
* Final build: 2018-10-09
*/

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include "headers/sighant.h"

/*
* description: Signal function that handles a signal with sigaction().
* param[in]: signo - The signal.
* param[in]: Sigfunc - Function pointer to a Signal handler function. Will be
* sa_handler for sigaction struct.
*/
void signalHandler (int signo, Sigfunc *sigFunc) {

	struct sigaction act;
	act.sa_handler = sigFunc;
// #ifdef SA_RESTART
// 	act.sa_flags |= SA_RESTART;
// #endif

	if (sigemptyset(&act.sa_mask) < 0) {

		perror("sigemptyset()");
		exit(1);
	}
	if (sigaction(signo, &act, NULL) < 0) {

		perror("sicacton()");
		exit(1);
	}
}