//************************************************************************//
//   -- KEYBOARD MODULE --
//Keyboard routines for using 4x4 keyboards
//
//Keyboard connection:
//  Keyboard columns: P0..P3 is Column0..3
//  Keyboard rows:    P4..P7 is Row0..3
//
//Notes:
//  1) _KBD_CPU_CLOCK_FREQ_
//    Define cpu clock frequency in KHz for correct debounce time delays
//  2) _KBD_IOINIT_ALWAYS_
//    Change to 1 to automatically set pin directions in call to any of the keyboard routines.
//    If defined as 1, no kbd_init function is required.
//This module can be used for any number of keyboards connected to AVR because it
//  receives the keyboard port address as a parameter.
//************************************************************************//

//************************************************************************//
//Include header files
#include <delay.h>
//************************************************************************//

//************************************************************************//
//Definitions
//Define cpu clock frequency in KHz for correct debounce time delays
#define _KBD_CPU_CLOCK_FREQ_ 8000  //KHz

//Change to 1 to automatically set pin directions in call to any of the keyboard routines
#define _KBD_IOINIT_ALWAYS_		0

//Define port addresses
//port = port
//direction port = port - 1
//pin port = port -2

#define KBD_PORT	*kbdport
#define KBD_DDR		*(kbdport -1)
#define KBD_PIN		*(kbdport - 2)

//************************************************************************//

//********************************************************************
void kbd_init(volatile unsigned char *kbdport)
{
	KBD_DDR = 0xF0;  //Rows output, columns input
	KBD_PORT = 0x0F;  //Enable pull-up resistors for columns
}
//********************************************************************

//********************************************************************
int kbd_hit(volatile unsigned char *kbdport)
{
  //Return value:  0  =  no key is available
  //               >0 = a key is available

  #if _KBD_IO_INIT_ALWAYS_ != 0
	  kbd_init(kbdport);
  #endif

  KBD_PORT = 0x0F;  //Ground all rows
  if((KBD_PIN & 0x0F)==0x0F)
  {
    return(0);
  }
  else
  {
    return(1);
  }
}
//********************************************************************

//********************************************************************
void kbd_readkey(volatile unsigned char *kbdport,
					unsigned char *row, unsigned char *col)
{
  unsigned char col_code;
	unsigned char row_index, col_index;

	#if _KBD_IO_INIT_ALWAYS_ != 0
	  kbd_init(kbdport);
  #endif

  while(1)
  {
    //while( (KBD_PIN & 0x0F)!=0x0F );  //Check till all keys released
    //delay(20, _KBD_CPU_CLOCK_FREQ_);
  
    while(!kbd_hit(kbdport));
    delay(20, _KBD_CPU_CLOCK_FREQ_);  //Wait 20 msec debounce time

    if(!kbd_hit(kbdport))
    {
      continue;
    }
    
    //Find key row
    row_index=0;
    do
    {
      KBD_PORT=~(1<<(row_index+4));
      col_code=KBD_PIN & 0x0F;
      row_index++;
    } while(col_code==0x0F && row_index<4);
		row_index--;
    
    if(row_index>=4)
    {
      continue;
    }
    
    col_code=(~col_code) & 0x0F;
    col_index=0;
    while(col_code != 0x00)
    {
      col_code>>=1;
      col_index++;
    }
    col_index--;

		*row = row_index;
		*col = col_index;
		return;
  }  
}
//********************************************************************
