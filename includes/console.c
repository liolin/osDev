
#include "console.h"

static int x = 0;
static int y = 0;
static int kprintf_res = 0;     // return value of kprintf show printf on C
static uint16_t color = 0x0A;
static uint16_t* video = (uint16_t*)VADRESS;

void kclear(void)
{
  int i = 0;

  for(i = 0; i < 25 * 80; i++)
  {
    video[i] = (uint16_t)0x00;
  }
  x = y = 0;
}

void kputc(const char c)
{
  int i = 0;
  if(c == '\n' || x > 79)
  {
    y++;
    x = 0;
  }

  if(c == '\n')
    return;

  // Screen Scrolling
  if(y > 24)
  {
    for(i = 0; i < 24 * 80; i++)
    {
      video[i] = video[i + 80];
    }
    y--;
  }

  if(c == '\t')
  {
    for(i = 0; i < 4;i++)
    {
      video[y * 80 + x] = (((uint16_t)color) << 8) | (uint16_t) ' ';
      x++;
      kprintf_res++;
    }
  }
  else
  {
    video[y * 80 + x] = (((uint16_t)color) << 8) | (uint16_t)c;
    x++;
    kprintf_res++;
  }
}

void kputs(const char *pStr)
{
  while(*pStr)
    kputc(*pStr++);
}

int kprintf(const char* fmt, ...)
{
  va_list ap;
  kprintf_res = 0;
  unsigned long n = 0;

  va_start(ap, fmt);

  while(*fmt)
  {
    if(*fmt == '%')
    {
      switch(*++fmt)
      {
        case 's':
          kputs(va_arg(ap, char*));
          break;

        case 'd':
          n = va_arg(ap, unsigned long int);
          kputn(n, 10);
          break;

        case 'o':
          n = va_arg(ap, unsigned long int);
          kputn(n, 8);
          break;

        case 'x':
          n = va_arg(ap, unsigned long int);
          kputn(n, 16);
          break;

        case '%':
          kputc('%');
          break;

        case '\0':
          goto out;
          break;
        default:
          kputc('%');
          kputc(*fmt);
          break;
      }
    }
    else
      kputc(*fmt);

    fmt++;
  }

out:
  va_end(ap);
  return kprintf_res;
}

void kputn(unsigned long nr, unsigned long sys)
{
  char *digits = "0123456789abcdefghijklmnopqrstuvwxyz";
  char buffer[80];
  char *p;

  if(sys > 36)
    return;

  p = buffer + 80; // set p on the last of buffer
  *p = '\0';

  do
  {
    *--p = (digits[nr % sys]);
    nr /= sys;
  }while(nr);

  kputs(p);
}
