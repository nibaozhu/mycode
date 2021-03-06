/*
 * This is free software; see the source for copying conditions.  There is NO
 * warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */
#ifndef HANDLE_H
#define HANDLE_H

#include "transport.h"
#include "client.h"

int handle(std::list<Transport*> *w, std::map<int, Transport*> *m, std::list<Transport*> *c, Transport* t);

#endif

