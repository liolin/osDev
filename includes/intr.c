#include "intr.h"

extern void intr_stub_0(void);
extern void intr_stub_1(void);
extern void intr_stub_2(void);
extern void intr_stub_3(void);
extern void intr_stub_4(void);
extern void intr_stub_5(void);
extern void intr_stub_6(void);
extern void intr_stub_7(void);
extern void intr_stub_8(void);
extern void intr_stub_9(void);
extern void intr_stub_10(void);
extern void intr_stub_11(void);
extern void intr_stub_12(void);
extern void intr_stub_13(void);
extern void intr_stub_14(void);
extern void intr_stub_15(void);
extern void intr_stub_16(void);
extern void intr_stub_17(void);
extern void intr_stub_18(void);

extern void intr_stub_32(void);
extern void intr_stub_33(void);

extern void intr_stub_48(void);



// siehe http://www.lowlevel.eu/wiki/Global_Descriptor_Table

// Segment Type
#define GDT_FLAG_DATASEG  0x02    // Data Segment
#define GDT_FLAG_CODESEG  0x0a    // Code Segment
#define GDT_FLAG_TSS      0x09    // Gates and TSS Segment

// Access Byte
#define GDT_FLAG_SEGMENT  0x10    // Segment flag look above
#define GDT_FLAG_RING0    0x00
#define GDT_FLAG_RING3    0x60
#define GDT_FLAG_PRESENT  0x80

// Flags
#define GDT_FLAG_4K_GRAN  0x800
#define GDT_FLAG_32_BIT   0x400

static uint64_t gdt[GDT_ENTRIES];
static uint64_t idt[IDT_ENTRIES];

/**
  \param pos  position in the table
  \param base start addresse from the segment
  \param limit  size from the segment (in 4kb or bytes look flags)

  Set a GDT-Entry
*/

static void setGDTentry(int pos, uint32_t base, uint32_t limit, int32_t flags)
{
  // LL ist ein suffix und steht für Long Long
  // Erstellt so mit ein Long Long
  gdt[pos] = limit & 0xffffLL;
  gdt[pos] |= (base & 0xffffffLL) << 16;
  gdt[pos] |= (flags & 0xffLL) << 40;
  gdt[pos] |= ((limit >> 16) & 0xfLL) << 48;    // wird zu 0
  gdt[pos] |= ((flags >> 8) & 0xffLL) << 52;    // wird zu 0
  gdt[pos] |= ((base >> 24) & 0xffLL) << 56;    // wird zu 0
}


/**
  Load the new GDT-Table
  And reload the Segmentregister
*/

static void loadGDT()
{
  struct
  {
      uint16_t limit;
      void *pointer;
  } __attribute__((packed)) gdtp = {
      .limit = GDT_ENTRIES * 8 - 1,
      .pointer = gdt,
  };

  asm volatile("lgdt %0" : : "m" (gdtp));

  asm volatile(
    "mov $0x10, %ax;"
    "mov %ax, %ds;"
    "mov %ax, %es;"
    "mov %ax, %ss;"
    "ljmp $0x8, $.1;"
    ".1:"
  );
}


/**
  Init the GDT-Table
*/

void initGDT()
{
  // 0xfffff 4kb = 0xfffff * 4 * 1024 = 4GB
  setGDTentry(0, 0, 0, 0);   // Null Descriptor
  setGDTentry(1, 0, 0xfffff, GDT_FLAG_SEGMENT | GDT_FLAG_32_BIT |
     GDT_FLAG_CODESEG | GDT_FLAG_4K_GRAN | GDT_FLAG_PRESENT);
  setGDTentry(2, 0, 0xfffff, GDT_FLAG_SEGMENT | GDT_FLAG_32_BIT |
    GDT_FLAG_DATASEG | GDT_FLAG_4K_GRAN | GDT_FLAG_PRESENT);
  setGDTentry(3, 0, 0xfffff, GDT_FLAG_SEGMENT | GDT_FLAG_32_BIT |
    GDT_FLAG_CODESEG | GDT_FLAG_4K_GRAN | GDT_FLAG_PRESENT | GDT_FLAG_RING3);
  setGDTentry(4, 0, 0xfffff, GDT_FLAG_SEGMENT | GDT_FLAG_32_BIT |
    GDT_FLAG_DATASEG | GDT_FLAG_4K_GRAN | GDT_FLAG_PRESENT | GDT_FLAG_RING3);

  loadGDT();
}

static inline void outb(uint16_t port, uint8_t data)
{
  asm volatile("outb %0, %1" : : "a" (data), "Nd" (port));
}

void initPIC()
{
  // Master-PIC initialisieren
  outb(0x20, 0x11); // Initialisierungsbefehl fuer den PIC
  outb(0x21, 0x20); // Interruptnummer fuer IRQ 0
  outb(0x21, 0x04); // An IRQ 2 haengt der Slave
  outb(0x21, 0x01); // ICW 4

  // Slave-PIC initialisieren
  outb(0xa0, 0x11); // Initialisierungsbefehl fuer den PIC
  outb(0xa1, 0x28); // Interruptnummer fuer IRQ 8
  outb(0xa1, 0x02); // An IRQ 2 haengt der Slave
  outb(0xa1, 0x01); // ICW 4

  // Alle IRQs aktivieren (demaskieren)
  outb(0x21, 0x0);
  outb(0xa1, 0x0);
}

#define IDT_FLAG_INTERRUPT_GATE 0xe
#define IDT_FLAG_PRESENT        0x80
#define IDT_FLAG_RING0          0x00
#define IDT_FLAG_RING3          0x60

void setIDTentry(int pos, void (*fn)(), uint32_t selector, int32_t flags)
{
  unsigned long int handler = (unsigned long int)fn;

  idt[pos] = (handler & 0xffffLL);
  idt[pos] |= (selector & 0xffffLL) << 16;
  idt[pos] |= (flags & 0xffLL) << 40;
  idt[pos] |= ((handler >> 16) & 0xffffLL) << 48;
}

void loadIDT()
{
  struct {
    unsigned short int limit;
    void *pointer;
  } __attribute__((packed)) idtp = {
    .limit = IDT_ENTRIES * 8 -1,
    .pointer = idt,
  };

  asm volatile("lidt %0" : : "m" (idtp));
}

void initIntr()
{
  initPIC();

  // Exception 0 - 31
  setIDTentry(0, intr_stub_0, 0x8, IDT_FLAG_INTERRUPT_GATE | IDT_FLAG_RING0 | IDT_FLAG_PRESENT);
  setIDTentry(1, intr_stub_1, 0x8, IDT_FLAG_INTERRUPT_GATE | IDT_FLAG_RING0 | IDT_FLAG_PRESENT);
  setIDTentry(2, intr_stub_2, 0x8, IDT_FLAG_INTERRUPT_GATE | IDT_FLAG_RING0 | IDT_FLAG_PRESENT);
  setIDTentry(3, intr_stub_3, 0x8, IDT_FLAG_INTERRUPT_GATE | IDT_FLAG_RING0 | IDT_FLAG_PRESENT);
  setIDTentry(4, intr_stub_4, 0x8, IDT_FLAG_INTERRUPT_GATE | IDT_FLAG_RING0 | IDT_FLAG_PRESENT);
  setIDTentry(5, intr_stub_5, 0x8, IDT_FLAG_INTERRUPT_GATE | IDT_FLAG_RING0 | IDT_FLAG_PRESENT);
  setIDTentry(6, intr_stub_6, 0x8, IDT_FLAG_INTERRUPT_GATE | IDT_FLAG_RING0 | IDT_FLAG_PRESENT);
  setIDTentry(7, intr_stub_8, 0x8, IDT_FLAG_INTERRUPT_GATE | IDT_FLAG_RING0 | IDT_FLAG_PRESENT);
  setIDTentry(8, intr_stub_9, 0x8, IDT_FLAG_INTERRUPT_GATE | IDT_FLAG_RING0 | IDT_FLAG_PRESENT);
  setIDTentry(19, intr_stub_10, 0x8, IDT_FLAG_INTERRUPT_GATE | IDT_FLAG_RING0 | IDT_FLAG_PRESENT);
  setIDTentry(11, intr_stub_11, 0x8, IDT_FLAG_INTERRUPT_GATE | IDT_FLAG_RING0 | IDT_FLAG_PRESENT);
  setIDTentry(12, intr_stub_12, 0x8, IDT_FLAG_INTERRUPT_GATE | IDT_FLAG_RING0 | IDT_FLAG_PRESENT);
  setIDTentry(13, intr_stub_13, 0x8, IDT_FLAG_INTERRUPT_GATE | IDT_FLAG_RING0 | IDT_FLAG_PRESENT);
  setIDTentry(14, intr_stub_14, 0x8, IDT_FLAG_INTERRUPT_GATE | IDT_FLAG_RING0 | IDT_FLAG_PRESENT);
  setIDTentry(15, intr_stub_15, 0x8, IDT_FLAG_INTERRUPT_GATE | IDT_FLAG_RING0 | IDT_FLAG_PRESENT);
  setIDTentry(16, intr_stub_16, 0x8, IDT_FLAG_INTERRUPT_GATE | IDT_FLAG_RING0 | IDT_FLAG_PRESENT);
  setIDTentry(17, intr_stub_17, 0x8, IDT_FLAG_INTERRUPT_GATE | IDT_FLAG_RING0 | IDT_FLAG_PRESENT);
  setIDTentry(18, intr_stub_18, 0x8, IDT_FLAG_INTERRUPT_GATE | IDT_FLAG_RING0 | IDT_FLAG_PRESENT);

  // IRQ 32 - 47
  setIDTentry(32, intr_stub_32, 0x8, IDT_FLAG_INTERRUPT_GATE | IDT_FLAG_RING0 | IDT_FLAG_PRESENT);
  setIDTentry(33, intr_stub_33, 0x8, IDT_FLAG_INTERRUPT_GATE | IDT_FLAG_RING0 | IDT_FLAG_PRESENT);

  // Syscalls 48 - 63
  setIDTentry(48, intr_stub_48, 0x8, IDT_FLAG_INTERRUPT_GATE | IDT_FLAG_RING3 | IDT_FLAG_PRESENT);

  loadIDT();
}

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


void int_handler(struct cpu_state *cpu)
{
  // 0x0 - 0x1f Exceptions
  if(cpu->intr <= 0x1f)
  {
    kprintf("Exception %d, Kernel wird angehalten!\n", cpu->intr);

    kprintf("Register Selbst gesichert\n");
    kprintf("%%eax: %d\n", cpu->eax);
    kprintf("%%ebx: %d\n", cpu->ebx);
    kprintf("%%ecx: %d\n", cpu->ecx);
    kprintf("%%edx: %d\n", cpu->edx);
    kprintf("%%esi: %d\n", cpu->esi);
    kprintf("%%edi: %d\n", cpu->edi);
    kprintf("%%ebp: %d\n", cpu->ebp);

    kprintf("Interrupt: %d\n", cpu->intr);
    kprintf("Error: %d\n", cpu->error);

    kprintf("\nRegister CPU gesichert");
    kprintf("%%eip: %d\n", cpu->eip);
    kprintf("%%cs: %d\n", cpu->cs);
    kprintf("%%eflags: %d\n", cpu->eflags);
    kprintf("%%esp: %d\n", cpu->esp);
    kprintf("%%ss: %d\n", cpu->ss);

    while(1)
    {
      asm volatile("cli; hlt");
    }
  }
  // 0x20 - 0x2f hardarewaree
  else if(cpu->intr <= 0x2f || cpu->intr >= 0x20)
  {
    kprintf("Hardware-Interrupt %d", cpu->intr);

    if(cpu->intr >= 0x28)
      outb(0xa0, 0x20);
    outb(0x20, 0x20);
  }
  else
  {
    kprintf("Software-Interrupt %d", cpu->intr);
  }
}
