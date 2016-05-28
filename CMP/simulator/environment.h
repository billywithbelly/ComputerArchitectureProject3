#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H

#include "library.h"

void loadImage (INST32, INST32*, INST8*, INST8*);
void produceSnapShot (INST32, FILE*, INST32*, int);
void produceReport ();
INST32 instructionFetcher (INST32, INST8*);
void instructionPractice (INST32);
int instructionFragment (INST32, int, int);

#endif