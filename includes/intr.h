#ifndef GDT_H
#define GDT_H

#include "stdint.h"
#include "console.h"

#define GDT_ENTRIES 5
#define IDT_ENTRIES 256

typedef struct cpu_state cpuState_t;
struct cpu_state
{
  uint32_t  eax;
  uint32_t  ebx;
  uint32_t  ecx;
  uint32_t  edx;
  uint32_t  esi;
  uint32_t  edi;
  uint32_t  ebp;

  uint32_t  intr;
  uint32_t  error;

  uint32_t  eip;
  uint32_t  cs;
  uint32_t  eflags;
  uint32_t  esp;
  uint32_t  ss;
};


void initGDT();
void initIntr();

void int_handler();

#endif /* GDT_H */
