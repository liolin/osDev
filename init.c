/*
  init.c
*/

#include "includes/stdint.h"
#include "includes/console.h"
#include "includes/intr.h"

/**
  \brief startpoint of the os

  Print "Hello World on the Screen"
*/

void init(void)
{
  kputs("START - Init GDT");
  initGDT();
  kputs("\t\t\t\tOK\n");

  kputs("START - Init IDT");
  initIntr();
  kputs("\t\t\t\tOK\n");

  kputs("START - Enable interupts");
  asm volatile("sti"); //Hardware interupts aktivieren
  kputs("\t\tOK\n");
}
