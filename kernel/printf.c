//
// formatted console output -- printf, panic.
//

#include <stdarg.h>

#include "types.h"
#include "param.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "fs.h"
#include "file.h"
#include "memlayout.h"
#include "riscv.h"
#include "defs.h"
#include "proc.h"
#include <stddef.h>

volatile int panicked = 0;

// lock to avoid interleaving concurrent printf's.
static struct {
  struct spinlock lock;
  int locking;
} pr;

static char digits[] = "0123456789abcdef";

static void
printint(int xx, int base, int sign)
{
  char buf[16];
  int i;
  uint x;

  if(sign && (sign = xx < 0))
    x = -xx;
  else
    x = xx;

  i = 0;
  do {
    buf[i++] = digits[x % base];
  } while((x /= base) != 0);

  if(sign)
    buf[i++] = '-';

  while(--i >= 0)
    consputc(buf[i]);
}

static void
printptr(uint64 x)
{
  int i;
  consputc('0');
  consputc('x');
  for (i = 0; i < (sizeof(uint64) * 2); i++, x <<= 4)
    consputc(digits[x >> (sizeof(uint64) * 8 - 4)]);
}

// Print to the console. only understands %d, %x, %p, %s.
void
printf(char *fmt, ...)
{
  va_list ap;
  int i, c, locking;
  char *s;

  locking = pr.locking;
  if(locking)
    acquire(&pr.lock);

  if (fmt == 0)
    panic("null fmt");

  va_start(ap, fmt);
  for(i = 0; (c = fmt[i] & 0xff) != 0; i++){
    if(c != '%'){
      consputc(c);
      continue;
    }
    c = fmt[++i] & 0xff;
    if(c == 0)
      break;
    switch(c){
    case 'd':
      printint(va_arg(ap, int), 10, 1);
      break;
    case 'x':
      printint(va_arg(ap, int), 16, 1);
      break;
    case 'p':
      printptr(va_arg(ap, uint64));
      break;
    case 's':
      if((s = va_arg(ap, char*)) == 0)
        s = "(null)";
      for(; *s; s++)
        consputc(*s);
      break;
    case '%':
      consputc('%');
      break;
    default:
      // Print unknown % sequence to draw attention.
      consputc('%');
      consputc(c);
      break;
    }
  }

  if(locking)
    release(&pr.lock);
}

void
panic(char *s)
{
  pr.locking = 0;
  printf("panic: ");
  printf(s);
  printf("\n");
  panicked = 1; // freeze uart output from other CPUs
  for(;;)
    ;
}

void
printfinit(void)
{
  initlock(&pr.lock, "pr");
  pr.locking = 1;
}

void backtrace(uint64 * pointer)
{
    uint64 fp_temp = r_fp();
    // same page
    uint64 * bottom =  (uint64 *)PGROUNDDOWN(fp_temp);
    uint64 * up= (uint64 *) PGROUNDUP(fp_temp);
    // convert to pointer
    uint64 * fp = (uint64 *)fp_temp;
    while(1)
    {
        //  get return address 
        uint64 val = *(fp-1);
        printf("in backtrace %p\n", val); 
        // get previous stack pointer
        fp = (uint64 *)*(fp-2);
        if(fp > bottom && fp < up)
        { }
        else
        { break; }
    }


  //      uint64 val = *(pointer-1);
  //      if(val >  || val <  PGROUNDDOWN(val))
  //      { return; }
  //      //if(val < 0x80000000)
  //      printf("in backtrace val2 %p\n", val); 
  //      backtrace((uint64 *)(*(pointer-2)));


   //uint64 val = *(fp-16);
   //uint64 val = *(fp);
   //printf("in backtrace val1 %p\n", val); 
   //val = *(fp-2);
   //printf("in backtrace val3 %p\n", val); 
   //printf("in backtrace fp %p\n", fp_temp); 
}
