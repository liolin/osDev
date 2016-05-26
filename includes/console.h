#ifndef CONSOLE_H
#define CONSOLE_H

#include "stdarg.h"
#include "stdint.h"

#define VADRESS 0xB8000

void kclear(void);
void kputc(char c);
void kputs(const char* s);
int kprintf(const char* fmt, ...);
void kputn(unsigned long nr, unsigned long sys);

#endif /* CONSOLE_H */
