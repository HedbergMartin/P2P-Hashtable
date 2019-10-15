/*
* sighant - signal handler interface. Will take a signal and a signal handler
* function and send them with sigaction. SA_RESTART will be only flags applied
* to signal handler function.
*
* Author: Buster Hultgren Wärn <dv17bhn@cs.umu.se>
*
* Final build: 2018-10-09
*/

#ifndef SIGHANT
#define SIGHANT

typedef void Sigfunc(int);

/*
* description: Signal function that handles a signal with sigaction().
* param[in]: signo - The signal.
* param[in]: Sigfunc - Function pointer to a Signal handler function. Will be
* sa_handler for sigaction struct.
*/
void signalHandler (int signo, Sigfunc *func);

#endif //SIGHANT
