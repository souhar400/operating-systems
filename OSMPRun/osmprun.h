//
// Created by studi on 19.04.22.
//

#ifndef OSMP_OSMPRUN_H
#define OSMP_OSMPRUN_H

#include "lib/OSMP.h"

#define MAXARGS 100

void initChild(struct shared_memory *mem, int rank);
void initMemory(struct shared_memory *mem);

#endif //OSMP_OSMPRUN_H
