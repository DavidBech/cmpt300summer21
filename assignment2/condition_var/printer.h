#ifndef _PRINTER_H
#define _PRINTER_H

#include <pthread.h>

void Printer_init();
void Printer_signalNextChar();
void Printer_shutdown();

#endif