//**************************************************************************
/*{{
Project: AVRCalculator
Description:
  A scientific calculator for ATmega32 with enhanced features:
    - Formula editor
    - Mathematical functions
Compiler:
  AVRgcc


		By:
		    M. Mehdipour (mo.mehdipour@gmail.com)
}}*/
//**************************************************************************

//AVRCalculator.c : source file for the AVRCalculator project

//CPU clock frequency in KHz
#define CLOCK_FREQ		8000	//KHz
#define LCD_PORT			PORTB
#define KBD1_PORT			PORTD
#define KBD2_PORT			PORTA

//Include header files
#include "AVRCalculator.h"
#include "parser.h"
#include "types.h"
#include <lcd.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <delay.h>
#include <pgmspace.h>
#include "lcdmore.h"
#include "keybrd.h"
#include "AVRCalculatorTimer.h"

//************************************************************************//
//************************************************************************//
//Constant definitions

////////////////////////////////////////////////////////////////////////////
//All used buttons
#define								BUTTON_UNDEFINED				0xFF
//Numbers
#define								NUMBER_0								'0'
#define								NUMBER_1								'1'
#define								NUMBER_2								'2'
#define								NUMBER_3								'3'
#define								NUMBER_4								'4'
#define								NUMBER_5								'5'
#define								NUMBER_6								'6'
#define								NUMBER_7								'7'
#define								NUMBER_8								'8'
#define								NUMBER_9								'9'
//Main operators
#define								OPERATOR_PLUS						'+'
#define								OPERATOR_MINUS					'-'
#define								OPERATOR_MUL						'*'
#define								OPERATOR_DIV						'/'
//Other operators
#define								OPERATOR_POWER					'^'
//Function buttons
  //Triangular functions
#define								FUNCTION_SIN						0
#define								FUNCTION_COS						1
#define								FUNCTION_TAN						2
#define								FUNCTION_ARCSIN					3
#define								FUNCTION_ARCCOS					4
#define								FUNCTION_ARCTAN					5
#define								FUNCTION_SINH						6
#define								FUNCTION_COSH						7
#define								FUNCTION_TANH						8
#define								FUNCTION_ARCSINH				9
#define								FUNCTION_ARCCOSH				10
#define								FUNCTION_ARCTANH				11
  //Other functions
#define								FUNCTION_LN							12
#define								FUNCTION_LOG						13
#define								FUNCTION_EXP						14
#define								FUNCTION_RAN						15
#define								FUNCTION_SQRT						16
//Formula control buttons
#define								FORMULA_LEFT						17
#define								FORMULA_RIGHT						18
#define								FORMULA_HOME						19
#define								FORMULA_END							20
#define								FORMULA_DEL							21
#define								FORMULA_INS							22
//Constants
#define								CONSTANT_PI							23
//Variables
#define								VARIABLE_ANS						24
//Other buttons
#define								BUTTON_ON								25
#define								BUTTON_OFF							26
#define								BUTTON_SHIFT						27
#define								BUTTON_E								28
#define								BUTTON_PERIOD						'.'
#define								BUTTON_DRG							29
#define								BUTTON_HYP							30
#define								BUTTON_LPAREN						'('
#define								BUTTON_RPAREN						')'
#define								BUTTON_EQUAL						'='


////////////////////////////////////////////////////////////////////////////
//All formula diplay char codes
//Mark end of each table with 0xFF
//Thease tables only contain the text that will be displayed for for functions and
//	characters on the LCD.
//Note: Thease characters are also used for generating parsable formula for expression
//	parser when a function or character is not defined in the parsable_strs_table.
FLASH char formula_char_table_1len[][2] = {
	{NUMBER_0, '0'},
	{NUMBER_1, '1'},
	{NUMBER_2, '2'},
	{NUMBER_3, '3'},
	{NUMBER_4, '4'},
	{NUMBER_5, '5'},
	{NUMBER_6, '6'},
	{NUMBER_7, '7'},
	{NUMBER_8, '8'},
	{NUMBER_9, '9'},
	{OPERATOR_PLUS, '+'},
	{OPERATOR_MINUS, '-'},
	{OPERATOR_MUL, 0x78},
	{OPERATOR_DIV, 0xFD},
	{OPERATOR_POWER, '^'},
	{BUTTON_LPAREN, '('},
	{BUTTON_RPAREN, ')'},
	{BUTTON_PERIOD, '.'},
	{BUTTON_E, 'E'},
	{CONSTANT_PI, 0xB6},
	{0xFF,'?'}  //End of tble
};

FLASH char formula_char_table_2len[][3] = {
	{FUNCTION_SQRT, 0xE8, '('},
	{0xFF, '?','?'}  //End of tble
};

FLASH char formula_char_table_3len[][4] = {
	{FUNCTION_LN, 'L','n','('},
	{VARIABLE_ANS, 'A','n','s'},
	{0xFF, '?','?','?'}  //End of tble
};

FLASH char formula_char_table_4len[][5] = {
	{FUNCTION_EXP, 'e','x','p','('},
	{FUNCTION_SIN, 's','i','n','('},
	{FUNCTION_COS, 'c','o','s','('},
	{FUNCTION_TAN, 't','a','n','('},
	{FUNCTION_LOG, 'l','o','g','('},
	{FUNCTION_RAN, 'R','a','n','#'},
	{0xFF, '?','?','?','?'}  //End of tble
};

FLASH char formula_char_table_5len[][6] = {
	{FUNCTION_SINH, 's','i','n','h','('},
	{FUNCTION_COSH, 'c','o','s','h','('},
	{FUNCTION_TANH, 't','a','n','h','('},
	{FUNCTION_ARCSIN, 's','i','n', 5, '('},
	{FUNCTION_ARCCOS, 'c','o','s', 5, '('},
	{FUNCTION_ARCTAN, 't','a','n', 5, '('},
	{0xFF, '?','?','?','?','?'}  //End of tble
};

FLASH char formula_char_table_6len[][7] = {
	{FUNCTION_ARCSINH, 's','i','n','h', 5, '('},
	{FUNCTION_ARCCOSH, 'c','o','s','h', 5, '('},
	{FUNCTION_ARCTANH, 't','a','n','h', 5, '('},
	{0xFF, '?','?','?','?','?', '?'}  //End of tble
};

FLASH char formula_char_table_7len[][8] = {
	{0xFF, '?','?','?','?','?','?','?'}  //End of tble
};

FLASH char formula_char_table_8len[][9] = {
	{0xFF, '?','?','?','?','?', '?', '?', '?'}  //End of tble
};

////////////////////////////////////////////////////////////////////////////
//Calculator sub modes
#define					SM_FORMULA						0
#define					SM_NEWFORMULA					1
#define					SM_DRGSELECT					2
#define					SM_ERROR							3

//Time limit for auto power off feature (according to timer1 compare match A (SIG_OUTPUT_COMPARE1A))
#define 				AUTO_POWER_OFF_BOUND					1500  //~5 Seconds

//************************************************************************//
//************************************************************************//
//Type definiyions


//Calculator global status
typedef struct
{
	BOOLEAN shift:1;
	BOOLEAN alpha:1;
	BOOLEAN buttonreleased:1;
	BOOLEAN poweron:1;
	unsigned char anglebase:2;
	unsigned char insertmode:1;
	unsigned char hyp:1;
	unsigned char submode;
	unsigned int autopoweroff_counter;
} CALC_STATUS;

//LCD status
typedef struct {
	UCHAR row, col;
	BOOLEAN showtext:1;
  BOOLEAN showcursor:1;
	BOOLEAN charblinking:1;
	BOOLEAN cursorblinking:1;
	
	unsigned char temp_blinkstep;
} LCD_STATUS;



//************************************************************************//
//************************************************************************//
//Global FLASH contant variables
//Welcome message
FLASH char welcome_msg[] = "   welcome to     AVRCalculator";

////////////////////////////////////////////////////////////////////////////
//Calculator keyboard button map for normal and shift modes (used by button_read function).
FLASH char kbd_table_normal[4][8] = {
	{NUMBER_7, NUMBER_8, NUMBER_9, OPERATOR_MUL, BUTTON_SHIFT, FORMULA_LEFT, FORMULA_RIGHT, BUTTON_ON},
	{NUMBER_4, NUMBER_5, NUMBER_6, OPERATOR_MINUS, OPERATOR_DIV, BUTTON_LPAREN, BUTTON_RPAREN, FORMULA_DEL},
	{NUMBER_1, NUMBER_2, NUMBER_3, OPERATOR_PLUS, FUNCTION_SQRT, BUTTON_HYP, FUNCTION_LOG, FUNCTION_LN},
	{NUMBER_0, BUTTON_PERIOD, BUTTON_E, BUTTON_EQUAL, OPERATOR_POWER, FUNCTION_SIN, FUNCTION_COS, FUNCTION_TAN}
};

FLASH char kbd_table_shift[4][8] = {
	{BUTTON_UNDEFINED, BUTTON_UNDEFINED, BUTTON_UNDEFINED, BUTTON_UNDEFINED, BUTTON_SHIFT, FORMULA_HOME, FORMULA_END, BUTTON_OFF},
	{BUTTON_UNDEFINED, BUTTON_UNDEFINED, BUTTON_UNDEFINED, BUTTON_UNDEFINED, BUTTON_UNDEFINED, BUTTON_UNDEFINED, BUTTON_UNDEFINED, FORMULA_INS},
	{BUTTON_UNDEFINED, BUTTON_UNDEFINED, BUTTON_UNDEFINED, BUTTON_UNDEFINED, BUTTON_UNDEFINED, BUTTON_DRG, BUTTON_UNDEFINED, FUNCTION_EXP},
	{BUTTON_UNDEFINED, FUNCTION_RAN, CONSTANT_PI, VARIABLE_ANS, BUTTON_UNDEFINED, FUNCTION_ARCSIN, FUNCTION_ARCCOS, FUNCTION_ARCTAN}
};

////////////////////////////////////////////////////////////////////////////
//Custom LCD characaters
FLASH LCC LCDCHAR_LEFTARROWSHIFT[]		=	{0x18, 0x10, 0x18, 0x0A, 0x1C, 0x0F, 0x04, 0x02};
FLASH LCC LCDCHAR_LEFTARROWNOSHIFT[]	=	{0x00, 0x00, 0x00, 0x02, 0x04, 0x0F, 0x04, 0x02};
FLASH LCC LCDCHAR_SHIFT[] 						=	{0x18, 0x10, 0x18, 0x08, 0x18, 0x00, 0x00, 0x00};

FLASH LCC LCDCHAR_RIGHTARROW[]     		=	{0x00, 0x00, 0x00, 0x08, 0x04, 0x1E, 0x04, 0x08};

FLASH LCC LCDCHAR_DSMALL[]         		=	{0x0C, 0x0A, 0x0A, 0x0A, 0x0C, 0x00, 0x00, 0x00};
FLASH LCC LCDCHAR_RSMALL[]         		=	{0x0C, 0x0A, 0x0C, 0x0A, 0x0A, 0x00, 0x00, 0x00};
FLASH LCC LCDCHAR_GSMALL[]         		=	{0x06, 0x08, 0x0B, 0x09, 0x06, 0x00, 0x00, 0x00};

FLASH LCC LCDCHAR_HYP[]								= {0x08, 0x08, 0x0E, 0x0A, 0x0A, 0x00, 0x00, 0x00};

FLASH LCC LCDCHAR_INSERT[]						= {0x1B, 0x11, 0x11, 0x00, 0x00, 0x11, 0x11, 0x1B};

FLASH LCC LCDCHAR_INVERSE[]						= {0x01, 0x01, 0x1D, 0x01, 0x01, 0x00, 0x00, 0x00};



//************************************************************************//
//************************************************************************//
//Global SRAM variables

CALC_STATUS calc_status = {OFF, OFF, False, RADIANS, OFF, OFF};

LCD_STATUS lcd_status = {0,0,True,True,False};

char lcd_line0[16], lcd_line1[16];

//Input formula variables and constants
#define 	FORMULA_MAX_LEN				50
#define		FORMULA_BLINK_BOUND		45
char formula[FORMULA_MAX_LEN];

//Input formula display status
typedef struct
{
	int len;
	int displeftpos, disprightpos, cursorpos;
} FORMULA_STATUS;

FORMULA_STATUS formula_status;

//************************************************************************//
//************************************************************************//
//Functions

//************************************************************************//
//FOR DEBUGGING ONLY
/*void debug_writeint(int i)
{
	char temp[30];
	itoa(i, temp, 10);
	lcd_puts(temp);
}*/

//************************************************************************//
//Return True if a keystroke is available
BOOLEAN kbd_hitall(void)
{
  return(kbd_hit(&KBD1_PORT) || kbd_hit(&KBD2_PORT));
}

//************************************************************************//
//Reads a key from both keyboards (waits for keystroke if necessary)
//Returns row and column of the button pressed.
void kbd_readkeyall(unsigned char *row, unsigned char *col)
{
	unsigned char kbd_num=0;
	//Only one key can be pressed at any time
	do {
		while(kbd_hit(&KBD1_PORT) && kbd_hit(&KBD2_PORT))
		{
			//Check auto power off
			if(calc_status.autopoweroff_counter >= AUTO_POWER_OFF_BOUND)
			{
				return;
			}
		}

		if(kbd_hit(&KBD1_PORT))
		{
			kbd_readkey(&KBD1_PORT, row, col);
			kbd_num = 1;
		}
		else if(kbd_hit(&KBD2_PORT))
		{
			kbd_readkey(&KBD2_PORT, row, col);
			kbd_num = 2;
		}
	} while(kbd_num==0);
	//0 <= *row <= 3, 0 <= *col <= 3
	*row = (~*row) & 0x03;
	*col = (~*col) & 0x03;
	//Convert row and col to all keyboard
	if(kbd_num==2)
		*col += 4;
}

//************************************************************************//
//NOT USED IN THIS VERSION
//This function can be implemented to read combination of buttons pressed simultaneously.
/*BOOLEAN button_multipressed(void)
{
	return(kbd_hitall());
}*/

//************************************************************************//
//NOT USED IN THIS VERSION
//This function can be implemented to read combination of buttons pressed simultaneously.
/*void button_multiread(void)
{
}*/

//************************************************************************//
//Returns True if a button is pressed and available.
BOOLEAN button_pressed(void)
{
	return((kbd_hit(&KBD1_PORT) && !kbd_hit(&KBD2_PORT)) ||
			   (kbd_hit(&KBD2_PORT) && !kbd_hit(&KBD1_PORT)));
}

//************************************************************************//
//Reads a button from the keyboard. This function returns the button code depending on the
//  shift status of the calculator.
unsigned char button_read(void)
{
	unsigned char row, col;
	char button;
	
	while(!calc_status.buttonreleased);
	kbd_readkeyall(&row, &col);
	calc_status.buttonreleased = False;

	if(calc_status.autopoweroff_counter >= AUTO_POWER_OFF_BOUND)
	{
		return(BUTTON_OFF);
	}

	if(calc_status.shift)
	{
		memcpy_P(&button, &kbd_table_shift[row][col], 1);
	}
	else
	{
		memcpy_P(&button, &kbd_table_normal[row][col], 1);
	}
	calc_status.autopoweroff_counter = 0;
	return button;

/*	unsigned char debug_btns[]={FUNCTION_SIN, FUNCTION_SIN, FUNCTION_SIN, FUNCTION_SIN, FUNCTION_COS, NUMBER_3, NUMBER_6, BUTTON_RPAREN,  BUTTON_RPAREN,  BUTTON_RPAREN,  BUTTON_RPAREN,  BUTTON_RPAREN, BUTTON_EQUAL, FORMULA_LEFT, OPERATOR_PLUS, NUMBER_3, BUTTON_EQUAL, FORMULA_LEFT, OPERATOR_MINUS, NUMBER_6, NUMBER_5, BUTTON_EQUAL, FUNCTION_SIN, FUNCTION_SIN, FUNCTION_COS, BUTTON_EQUAL, FORMULA_LEFT, NUMBER_5, NUMBER_7, BUTTON_RPAREN, BUTTON_RPAREN, BUTTON_EQUAL, FORMULA_LEFT, BUTTON_RPAREN, OPERATOR_PLUS, NUMBER_2};
	static int debug_charindex=0;
	char debugchar;
	
	debugchar=debug_btns[debug_charindex++];
	return(debugchar);*/
}

//************************************************************************//
//Applies lcd_status settings to the LCD.
void lcd_applystatus(void)
{
	
  char cmd_code = 0x00;
	
	lcd_gotoxy(lcd_status.col, lcd_status.row);

	if(lcd_status.showtext) 
		cmd_code|=LCD_DISP_DISP_ON;
	else
		cmd_code |= LCD_DISP_DISP_OFF;
	if(lcd_status.showcursor)
	  cmd_code |= LCD_DISP_CURSOR_ON;
	else
		cmd_code |= LCD_DISP_CURSOR_OFF;
	if(lcd_status.charblinking)
		cmd_code |= LCD_DISP_BLINK_ON;
	else
		cmd_code |= LCD_DISP_BLINK_OFF;

	lcd_command(cmd_code);
}

//************************************************************************//
//Interrupt Service Routine for Timer1 Compare Match A:
//  -- Show cursor blinking
//  -- Check keyboards to see if all buttons are released.
ISR(SIG_OUTPUT_COMPARE1A)
{
	static BOOLEAN show_insert_box = False;

	calc_status.autopoweroff_counter++;

	if(lcd_status.cursorblinking)
	{
		lcd_status.temp_blinkstep++;
		if(lcd_status.temp_blinkstep>=3)  //3 * 200ms = ~600ms
		{
			if(calc_status.insertmode && !lcd_status.charblinking)
			{
				lcd_status.showcursor = False;
				show_insert_box = !show_insert_box;
				if(show_insert_box)
					lcd_putchar(4);
				else
					lcd_putchar(lcd_line0[lcd_status.col]);
			}
			else
			{
				lcd_status.showcursor = !lcd_status.showcursor;
			}
			lcd_status.temp_blinkstep = 0;
		}
		if(lcd_status.charblinking)
			lcd_status.showcursor=False;
		lcd_applystatus();
	}

	if(!calc_status.buttonreleased)
	{
		calc_status.buttonreleased = !button_pressed();
	}
	
}

//************************************************************************//
//Seraches for the string representing corresponding to the button code
//Returns False if the specified button code does not exist in the tables (not used in this version)
//Other return values:
//  displen: length of the representing string
//  char_address: address of the representing string in the flash memory
BOOLEAN find_formula_char(unsigned char charcode, unsigned char *displen,
		FLASH char **char_address)
{
	unsigned char code;
	unsigned char row, table_num=1;
	FLASH char *char_table;
	
	while(table_num<=8)
	{
		switch(table_num)
		{
		case 1:
			char_table = (FLASH char *) &formula_char_table_1len;
			break;
		case 2:
			char_table = (FLASH char *) &formula_char_table_2len;
			break;
		case 3:
			char_table = (FLASH char *) &formula_char_table_3len;
			break;
		case 4:
			char_table = (FLASH char *) &formula_char_table_4len;
			break;
		case 5:
			char_table = (FLASH char *) &formula_char_table_5len;
			break;
		case 6:
			char_table = (FLASH char *) &formula_char_table_6len;
			break;
		case 7:
			char_table = (FLASH char *) &formula_char_table_7len;
			break;
		case 8:
			char_table = (FLASH char *) &formula_char_table_8len;
			break;
		}
		row=0;
		do
		{
			memcpy_P(&code, char_table+row*(table_num+1), 1);
			row++;
		} while((code!=charcode) && (code!=0xFF));
		if(code==charcode)
			break;
		table_num++;
	}
	if(code==charcode)
	{
		*displen = table_num;
		row--;
		*char_address = char_table+row*(table_num+1)+1;
		return(True);
	}
	return(False);
}

//************************************************************************//
//Updates lcd_line0 to be displayed in the first line of the LCD.
void generate_disp_formula(void)
{
	int displeftpos;
	int space_len;
	char c;
	FLASH char *addr;
	unsigned char char_len;
	unsigned char char_index;
	int i;
	
	for(i=1;i<=12;i++)
		lcd_line0[i]=' ';
	
	if(formula_status.len==0)
	{
		lcd_status.col=1;
		lcd_status.row=0;
		formula_status.displeftpos = 0;
		formula_status.disprightpos = 0;
		return;
	}
	
	if(calc_status.submode==SM_FORMULA)
	{
		if(formula_status.cursorpos==formula_status.len)
			space_len=11;
		else
			space_len=12;
	}
	else if(calc_status.submode==SM_NEWFORMULA)
		space_len=12;
	
	if(formula_status.cursorpos<formula_status.len)
		displeftpos=formula_status.cursorpos;
	else
		displeftpos=formula_status.cursorpos-1;
	displeftpos++;
	do
	{
		c=formula[displeftpos-1];
		find_formula_char(c, &char_len, &addr);
		space_len-=char_len;
		if(space_len>=0)
			displeftpos--;
	} while((space_len>0) && (displeftpos>0));
	
	space_len=11;
	formula_status.displeftpos=displeftpos;
	char_index=1;
	do
	{
		c=formula[displeftpos];
		find_formula_char(c, &char_len, &addr);
		if(displeftpos==formula_status.cursorpos)
		{
			lcd_status.col=char_index;
		}
		else if((displeftpos+1)==formula_status.cursorpos)
		{
			lcd_status.col=char_index+char_len;
		}
		if((char_index+char_len)>13)
			char_len=13-char_index;
		memcpy_P(&lcd_line0[char_index], addr, char_len);
		char_index+=char_len;
		displeftpos++;
	} while((char_index<13) && (displeftpos<formula_status.len));
	lcd_status.row=0;
	formula_status.disprightpos = --displeftpos;
}

//************************************************************************//
//Puts special character codes in the LCD display line (lcd_line0) and also
//  changes character maps in the LCD RAM if necessary.
void update_special_chars(void)
{
	BOOLEAN showleftarrow = False;
	LCC lcc;
	FLASH char *lcc_p;
	
	//Update left arrow
	if((calc_status.submode==SM_FORMULA) || (calc_status.submode==SM_NEWFORMULA))
		showleftarrow=formula_status.displeftpos>0;
	if(calc_status.shift || showleftarrow)
	{
		if(calc_status.shift && showleftarrow)
			lcc_p = (FLASH char *) &LCDCHAR_LEFTARROWSHIFT;
		else if(calc_status.shift)
			lcc_p = (FLASH char *) &LCDCHAR_SHIFT;
		else if(showleftarrow)
			lcc_p = (FLASH char *) &LCDCHAR_LEFTARROWNOSHIFT;
		memcpy_P(&lcc, lcc_p, 8);
		lcd_set_custom_char(0, &lcc);
		lcd_line0[0] = 0;
	}
	else
		lcd_line0[0] = ' ';
		
	//Update right arrow
	if((calc_status.submode == SM_FORMULA) || (calc_status.submode == SM_NEWFORMULA))
	{
		if( (formula_status.disprightpos < (formula_status.len-1)) ||
			( (formula_status.disprightpos == (formula_status.len-1)) && (formula_status.len >= 12) && (formula_status.cursorpos != formula_status.len) ) )
			lcd_line0[13] = 1;
		else
			lcd_line0[13] = ' ';
	}
	else
	{
		lcd_line0[13] = ' ';
	}
	
	//Update angle base char
	switch(calc_status.anglebase)
	{
	case DEGREE:
		lcc_p = (FLASH char *) &LCDCHAR_DSMALL;
		break;
	case RADIANS:
			lcc_p = (FLASH char *) &LCDCHAR_RSMALL;
			break;
	case GRADIANS:
			lcc_p = (FLASH char *) &LCDCHAR_GSMALL;
			break;
	default:
			lcc_p = (FLASH char *) &LCDCHAR_DSMALL;
			calc_status.anglebase = DEGREE;
	}
	memcpy_P(&lcc, lcc_p, 8);
	lcd_set_custom_char(2, &lcc);
	lcd_line0[15] = 2;
	
	//Update hyp char
	if((calc_status.submode == SM_FORMULA) || (calc_status.submode == SM_NEWFORMULA))
	{
		if(calc_status.hyp)
			lcd_line0[14] = 3;
		else
			lcd_line0[14] = ' ';
	}
}

//************************************************************************//
//Writes one line to the LCD depending on the lineno parameter.
void lcd_updateline(unsigned char lineno)
{
	char *lcd_line;
	int i;
	
	if(lineno==0)
	{
		lcd_gotoxy(0,0);
		lcd_line = lcd_line0;
	}
	else
	{
		lcd_gotoxy(0, 1);
		lcd_line = lcd_line1;
	}

	for(i=0;i<16;i++)
		lcd_putchar(lcd_line[i]);
	
	lcd_applystatus();
}

//************************************************************************//
//Refreshes all LCD screen.
void lcd_refresh(void)
{
	if((calc_status.submode == SM_FORMULA) ||
			(calc_status.submode == SM_NEWFORMULA))
	{
		generate_disp_formula();
	}
		
	update_special_chars();
		
	lcd_updateline(0);
		
	lcd_updateline(1);
	
	
	switch(calc_status.submode)
	{
	case SM_FORMULA:
		{
			lcd_status.charblinking=formula_status.len>=FORMULA_BLINK_BOUND;
			lcd_status.cursorblinking = True && !lcd_status.charblinking;
			lcd_status.showcursor = !lcd_status.charblinking && !calc_status.insertmode;
			break;
		}
	case SM_NEWFORMULA:
	case SM_ERROR:
	case SM_DRGSELECT:
		{
			lcd_status.charblinking = False;
			lcd_status.cursorblinking = False;
			lcd_status.showcursor = False;
			break;
		}
	}
	
	lcd_applystatus();
}

//************************************************************************//
//Coverts the input button code to a correct button code depending on the calculator
//  mode and status (for example when HYP is on, this function converts sin to sinh or
//  arcsin to arcsinh).
unsigned char correct_button(unsigned char button)
{
	unsigned char new_button;
	
	new_button = button;
	
	if(calc_status.hyp)
	{
		switch(button)
		{
		case FUNCTION_SIN:
			new_button = FUNCTION_SINH;
			break;
		case FUNCTION_COS:
			new_button = FUNCTION_COSH;
			break;
		case FUNCTION_TAN:
			new_button = FUNCTION_TANH;
			break;
		case FUNCTION_ARCSIN:
			new_button = FUNCTION_ARCSINH;
			break;
		case FUNCTION_ARCCOS:
			new_button = FUNCTION_ARCCOSH;
			break;
		case FUNCTION_ARCTAN:
			new_button = FUNCTION_ARCTANH;
			break;
		}
		if((button != BUTTON_SHIFT) && (button != BUTTON_HYP))
		{
			calc_status.hyp = OFF;
		}
	}
	
	return(new_button);
}

//************************************************************************//
//Selects angle base
FLASH char drg_select_msg_line0[] = " Deg  Rad  Gra  ";
FLASH char drg_select_msg_line1[] = "  1    2    3   ";

void select_anglebase(void)
{
	unsigned char button;
	unsigned char old_mode;
	LCC lcc;
	
	old_mode = calc_status.submode;
	calc_status.submode = SM_DRGSELECT;
	lcd_refresh();
	lcd_clear();
	lcd_gotoxy(0,0);
	lcd_puts_P(drg_select_msg_line0);
	lcd_gotoxy(0,1);
	lcd_puts_P(drg_select_msg_line1);
	
	calc_status.shift = OFF;

	memcpy_P(&lcc, &LCDCHAR_SHIFT, 8);
	lcd_set_custom_char(0, &lcc);
	
	do
	{
		button = button_read();
		switch(button)
		{
		case NUMBER_1:
			{
				calc_status.anglebase = DEGREE;
				parser_status.anglebase = DEGREE;
				break;
			}
		case NUMBER_2:
			{
				calc_status.anglebase = RADIANS;
				parser_status.anglebase = RADIANS;
				break;
			}
		case NUMBER_3:
			{
				calc_status.anglebase = GRADIANS;
				parser_status.anglebase = GRADIANS;
				break;
			}
		case BUTTON_DRG:
			{
				break;
			}
		case BUTTON_SHIFT:
			{
				calc_status.shift = !calc_status.shift;
					lcd_gotoxy(0,0);
				if(calc_status.shift)
					lcd_putchar(0);
				else
					lcd_putchar(' ');
				break;
			}
		}
	} while((button != NUMBER_1) && (button != NUMBER_2) &&
			(button != NUMBER_3) && (button != BUTTON_DRG));
	calc_status.submode = old_mode;
	lcd_refresh();
}

//************************************************************************//
//Displays a string in the result line of the LCD indicating no result is available.
void show_empty_result(void)
{
	int i;
	
	for(i=0;i<15;i++)
		lcd_line1[i] = ' ';
	lcd_line1[15] = '.';
	lcd_line1[14] = '0';
	
	lcd_updateline(1);
}

//************************************************************************//
//Formula flags returned by the get_formula function to the caller function.
typedef struct
{
	BOOLEAN poweroff:1;
	BOOLEAN formuladone:1;
} FORMULA_FLAGS;

//Function to get a formula from user.
void get_formula(FORMULA_FLAGS *flags)
{
	unsigned char button = 0;
	
	flags->poweroff=False;
	flags->formuladone=False;
	
	while(True)
	{
		button = correct_button(button_read());
		
		switch(button)
		{
		case BUTTON_ON:
			{
				formula_status.len = 0;
				formula_status.cursorpos = 0;
				calc_status.submode = SM_FORMULA;
				calc_status.insertmode = False;
				show_empty_result();
				break;
			}
		case BUTTON_OFF:
			{
				calc_status.shift = OFF;
				flags->poweroff=True;
				return;
				break;
			}
		case FORMULA_LEFT:
			{
				if(calc_status.submode==SM_FORMULA)
				{
					formula_status.cursorpos--;
					if(formula_status.cursorpos<0)
						formula_status.cursorpos=0;
				}
				else if((calc_status.submode==SM_NEWFORMULA) ||
						(calc_status.submode==SM_ERROR))
				{
					formula_status.cursorpos=formula_status.len;
					calc_status.submode = SM_FORMULA;
				}
				break;
			}
		case FORMULA_RIGHT:
			{
				if(calc_status.submode==SM_FORMULA)
				{
					formula_status.cursorpos++;
					if(formula_status.cursorpos>=formula_status.len)
						formula_status.cursorpos=formula_status.len;
				}
				else if((calc_status.submode==SM_NEWFORMULA) ||
						(calc_status.submode==SM_ERROR))
				{
					formula_status.cursorpos=0;
					calc_status.submode = SM_FORMULA;
				}
				break;
			}
		case FORMULA_HOME:
			{
				if((calc_status.submode==SM_FORMULA) || (calc_status.submode==SM_NEWFORMULA))
				{
					formula_status.cursorpos=0;
					calc_status.submode = SM_FORMULA;
				}
				break;
			}
		case FORMULA_END:
			{
				if((calc_status.submode==SM_FORMULA) || (calc_status.submode==SM_NEWFORMULA))
				{
					formula_status.cursorpos=formula_status.len;
					calc_status.submode = SM_FORMULA;
				}
				break;
			}
		case FORMULA_DEL:
			{
				if((calc_status.submode==SM_FORMULA) && (formula_status.len>0))
				{
					if(calc_status.insertmode)
					{
						//Interpret DEL as backspace
						if(formula_status.cursorpos==0)
						{
							//Delete char at cursor
							formula_status.len--;
							memmove(&formula[formula_status.cursorpos], &formula[formula_status.cursorpos+1],
									formula_status.len - formula_status.cursorpos);
						}
						else
						{
							//Delete char at left
							memmove(&formula[formula_status.cursorpos-1], &formula[formula_status.cursorpos],
									formula_status.len-formula_status.cursorpos);
							formula_status.len--;
							formula_status.cursorpos--;
						}
					}
					else
					{
						if(formula_status.cursorpos==formula_status.len)
						{
							formula_status.cursorpos--;
							formula_status.len--;
						}
						else
						{
							formula_status.len--;
							memmove(&formula[formula_status.cursorpos], &formula[formula_status.cursorpos+1],
									formula_status.len - formula_status.cursorpos);
							if(formula_status.cursorpos>formula_status.len)
								formula_status.len--;
						}
					}
				}
				break;
			}
		case FORMULA_INS:
			{
				if(calc_status.submode==SM_FORMULA)
				{
					calc_status.insertmode=!calc_status.insertmode;
				}
				break;
			}
		case BUTTON_SHIFT:
			{
				calc_status.shift = !calc_status.shift;
				break;
			}
		case BUTTON_DRG:
			{
				select_anglebase();
				break;
			}
		case BUTTON_HYP:
			{
				if((calc_status.submode==SM_FORMULA) || (calc_status.submode==SM_NEWFORMULA))
				{
					calc_status.hyp = !calc_status.hyp;
				}
				break;
			}
		case BUTTON_EQUAL:
			{
				if((calc_status.submode==SM_FORMULA) || (calc_status.submode==SM_NEWFORMULA))
				{
					if(formula_status.len>0)
					{
						flags->formuladone=True;
						formula_status.cursorpos=0;
						calc_status.insertmode = False;
						return;
					}
				}
				break;
			}
		case BUTTON_UNDEFINED:
			break;
		default:
			{
				//Append character to formula
				if(calc_status.submode==SM_FORMULA)
				{
					if(formula_status.cursorpos<formula_status.len)
					{
						if(calc_status.insertmode && formula_status.len < FORMULA_MAX_LEN)
						{
							memmove(&formula[formula_status.cursorpos+1], &formula[formula_status.cursorpos],
									formula_status.len-formula_status.cursorpos);
							formula_status.len++;
							formula[formula_status.cursorpos]=button;
							formula_status.cursorpos++;
						}
						else if(!calc_status.insertmode && formula_status.cursorpos<formula_status.len)
						{
							formula[formula_status.cursorpos]=button;
							formula_status.cursorpos++;
						}
					}
					else if(formula_status.len < FORMULA_MAX_LEN)
					{
						formula[formula_status.len++]=button;
					  formula_status.cursorpos++;
					}
				}
				else if((calc_status.submode==SM_NEWFORMULA) || (calc_status.submode == SM_ERROR))
				{
					formula_status.len=1;
					formula_status.cursorpos=1;
					formula[0]=button;
					calc_status.submode = SM_FORMULA;
				}
				else if(calc_status.submode==SM_DRGSELECT)
				{
				}
			}
		}  //switch		

		if(button != BUTTON_SHIFT)
			calc_status.shift = False;

		lcd_refresh();
	}  //while
}

//************************************************************************//
//Parsable string representation of buttons that are not directly parsable with the parser.
FLASH char OPERATOR_MUL_PSTR[] = "*";  //Multiply sign(×) to '*'
FLASH char OPERATOR_DIV_PSTR[] = "/";  //Division sign(÷) to '/'
FLASH char CONSTANT_PI_PSTR[] = "3.141592653589";  //Pi to its constant value
FLASH char FUNCTION_SQRT_PSTR[] = "sqrt(";  //Sqrt sign to 'sqrt'
FLASH char VARIABLE_ANS_PSTR[] = "ans(0)";  //Ans to a function representation to be parsable by parser
FLASH char FUNCTION_RAN_PSTR[] = "rand(0)"; //Ran# to a function representation to be parsable by parser
FLASH char FUNCTION_ARCSIN_PSTR[] = "arcsin(";
FLASH char FUNCTION_ARCCOS_PSTR[] = "arccos(";
FLASH char FUNCTION_ARCTAN_PSTR[] = "arctan(";
FLASH char FUNCTION_ARCSINH_PSTR[] = "arcsinh(";
FLASH char FUNCTION_ARCCOSH_PSTR[] = "arccosh(";
FLASH char FUNCTION_ARCTANH_PSTR[] = "arctanh(";

typedef struct
{
	unsigned char code;
	FLASH char *romaddress;
} PARSABLE_STR;

//Table containing all extra parsable strings
PARSABLE_STR parsable_strs_table[] = {  //Mark end of table with 0xFF code
	{OPERATOR_MUL, 0},
	{OPERATOR_DIV, 0},
	{CONSTANT_PI, 0},
	{FUNCTION_SQRT, 0},
	{VARIABLE_ANS, 0},
	{FUNCTION_RAN, 0},
	{FUNCTION_ARCSIN, 0},
	{FUNCTION_ARCCOS, 0},
	{FUNCTION_ARCTAN, 0},
	{FUNCTION_ARCSINH, 0},
	{FUNCTION_ARCCOSH, 0},
	{FUNCTION_ARCTANH, 0},
	{0xFF, 0}  //End of table
};

//************************************************************************//
//Initializes parsable_strs_table with string addresses in the flash memory.
void fill_parsable_strs_table(void)
{
	parsable_strs_table[0].romaddress = (FLASH char *) &OPERATOR_MUL_PSTR;
	parsable_strs_table[1].romaddress = (FLASH char *) &OPERATOR_DIV_PSTR;
	parsable_strs_table[2].romaddress = (FLASH char *) &CONSTANT_PI_PSTR;
	parsable_strs_table[3].romaddress = (FLASH char *) &FUNCTION_SQRT_PSTR;
	parsable_strs_table[4].romaddress = (FLASH char *) &VARIABLE_ANS_PSTR;
	parsable_strs_table[5].romaddress = (FLASH char *) &FUNCTION_RAN_PSTR;
	parsable_strs_table[6].romaddress = (FLASH char *) &FUNCTION_ARCSIN_PSTR;
	parsable_strs_table[7].romaddress = (FLASH char *) &FUNCTION_ARCCOS_PSTR;
	parsable_strs_table[8].romaddress = (FLASH char *) &FUNCTION_ARCTAN_PSTR;
	parsable_strs_table[9].romaddress = (FLASH char *) &FUNCTION_ARCSINH_PSTR;
	parsable_strs_table[10].romaddress = (FLASH char *) &FUNCTION_ARCCOSH_PSTR;
	parsable_strs_table[11].romaddress = (FLASH char *) &FUNCTION_ARCTANH_PSTR;
}

//************************************************************************//
//Seraches for a button code and find and returns its parsable string representation.
//Returns True if successful.
BOOLEAN find_parse_str(unsigned char char_code, unsigned char *len,
		FLASH char **char_address)
{
	//First search parsable_strs_table,
	//if not in table, call find_formula_char to search in formula_char_table_?len table.

	int i;	

	//Search parsable_strs_table
	i=0;
	while((parsable_strs_table[i].code!=char_code) && (parsable_strs_table[i].code!=0xFF))
		i++;
	if(parsable_strs_table[i].code==char_code)
	{
		*char_address = parsable_strs_table[i].romaddress;
		*len = strlen_P(*char_address);
		return(True);
	}
	
	//Not in table, search formula_char_table_?len
	return(find_formula_char(char_code, len, char_address));
}

//************************************************************************//
//Variable to store the formula passed to parser.
char parser_formula[350];

//Generates a formula from the input formula which is parsable by the parser.
void create_parsable_formula(void)
{
	int i;
	int parse_len=0;
	unsigned char char_len;
	FLASH char *char_address;
	
/*	//realloc does NOT work properly.
	parser_formula = realloc(NULL, 1);
	
	for(i=0;i<formula_status.len;i++)
	{
		if(find_parse_str(formula[i], &char_len, &char_address))
		{
			parser_formula = realloc(parser_formula, parse_len+char_len);
			memcpy_P(parser_formula+parse_len, char_address, char_len); 
			parse_len+=char_len;
		}
	}
	parser_formula = realloc(parser_formula, parse_len + 1);
	parser_formula[parse_len] = 0;
	*/

	parser_formula[0] =0;
	
	for(i=0;i<formula_status.len;i++)
	{
		if(find_parse_str(formula[i], &char_len, &char_address))
		{
			memcpy_P(&parser_formula[parse_len], char_address, char_len); 
			parse_len+=char_len;
		}
	}
	parser_formula[parse_len] = 0;
	
}

//************************************************************************//
//Coverts the input floating point number to string and displays it on the LCD result line.
void show_result(double result)
{
	char temp[20];
	int i, old_len;
	
  dtostre(result, temp, 8, DTOSTR_UPPERCASE);  //Use 8 digit precision for conversion, but only show 6 digits
	// temp = "[-]d.dddddd00E±dd"
	memmove(&temp[strlen(temp) - 6], &temp[strlen(temp) - 4], 5);  //Truncate trailing zeros

	old_len = strlen(temp);
	memmove(&temp[16 - old_len], temp, old_len + 1);
	for(i=0;i<(16 - old_len);i++)
		temp[i] = ' ';
	for(i=0;i<=15;i++)
		lcd_line1[i] = temp[i];
	
	lcd_gotoxy(0, 1);
	lcd_puts(temp);
}

//************************************************************************//
//Initialize io ports
static void io_init(void)
{
	//{{WIZARD_MAP(General)
	//}}WIZARD_MAP(General)

	//{{WIZARD_MAP(I/O Ports)
	// PortA
	PORTA = 0x0;
	DDRA = 0x0;
	// PortB
	PORTB = 0x0;
	DDRB = 0x0;
	// PortC
	PORTC = 0x0;
	DDRC = 0x0;
	// PortD
	PORTD = 0x0;
	DDRD = 0x0;
	//}}WIZARD_MAP(I/O Ports)

	//{{WIZARD_MAP(Watchdog)
	// Watchdog Disabled
	wdt_disable();
	//}}WIZARD_MAP(Watchdog)

	//{{WIZARD_MAP(Analog Comparator)
	// Analog Comparator Disabled
	ACSR = 0x80;
	//}}WIZARD_MAP(Analog Comparator)
}

//************************************************************************//
//Sets lcd custom characters which will not be changed.
void set_lcd_fixed_custom_chars(void)
{
	LCC lcc;
	
	//Set Right Arrow character
	memcpy_P(&lcc, &LCDCHAR_RIGHTARROW, 8);
	lcd_set_custom_char(1, &lcc);
	//Set Hyp character
	memcpy_P(&lcc, &LCDCHAR_HYP, 8);
	lcd_set_custom_char(3, &lcc);
	//Set inser mode character
	memcpy_P(&lcc, &LCDCHAR_INSERT, 8);
	lcd_set_custom_char(4, &lcc);
	//Set inverse character
	memcpy_P(&lcc, &LCDCHAR_INVERSE, 8);
	lcd_set_custom_char(5, &lcc);
}

//************************************************************************//
//Initializes calculator.
void initialize_calculator(void)
{
	kbd_init(&KBD1_PORT);
	kbd_init(&KBD2_PORT);
	set_lcd_fixed_custom_chars();
	fill_parsable_strs_table();
}

//************************************************************************//
//Resets calculator.
void reset_calculator(BOOLEAN full_reset)
{
	if(full_reset)
	{
		//Discard all calculator settings if a full reset (power on reset) is requested.
		initialize_calculator();
		calc_status.anglebase=RADIANS;
		parser_status.anglebase = RADIANS;
		parser_status.ans = 0.0;
	}

	lcd_clear();
	lcd_status.cursorblinking=True;
	lcd_status.showcursor=True;
	lcd_status.showtext=True;
	lcd_status.col=0;
	lcd_status.row=0;
	lcd_applystatus();
	
	formula_status.len=0;
	formula_status.cursorpos=0;
	formula_status.displeftpos=0;
	formula_status.disprightpos=0;
	
	calc_status.alpha=OFF;
	calc_status.hyp=OFF;
	calc_status.insertmode=False;
	calc_status.poweron=True;
	calc_status.shift=OFF;
	calc_status.submode=SM_FORMULA;
	calc_status.autopoweroff_counter = 0;
	
	
	show_empty_result();
	
	lcd_refresh();
}

//************************************************************************//
//Displays welcome message.
void welcome(void)
{
	unsigned char i;

	lcd_clear();
  lcd_status.cursorblinking = False;
  lcd_status.charblinking = False;
	lcd_status.showcursor = False;
	lcd_status.showtext = True;
	lcd_status.col = 0;
	lcd_status.row = 0;
	lcd_applystatus();
	lcd_puts_P(welcome_msg);
	delay(2200, CLOCK_FREQ);
	for(i=1;i<=15;i++)
	{
		lcd_command(LCD_SHIFT_DISP_LEFT);
		delay(220, CLOCK_FREQ);
	}
}

//************************************************************************//
//Function invoked when the ON button wakes up the calculator.
void poweron(void)
{
	welcome();
	
	reset_calculator(False);
	
	DDRC = 0x00;
	PORTC = 0xFF;
}

//************************************************************************//
//Function invoked when the OFF button is pressed.
void poweroff(void)
{
	unsigned char button;
	
	lcd_status.showtext=False;
	lcd_status.showcursor=False;
	lcd_status.charblinking=False;
	lcd_status.cursorblinking=False;
	lcd_applystatus();
	
	calc_status.poweron=False;
	PORTC = 0x00;
	
	while((button = button_read()) != BUTTON_ON)
	{
	}
	
	poweron();
}

//************************************************************************//
//Function to display calculation error messages
void show_calc_error(void)
{
	calc_status.submode = SM_ERROR;
	strcpy(&lcd_line0[1], "Syntax ERROR");
	lcd_line0[13] =' ';
	lcd_refresh();
}

//************************************************************************//
//												M A I N  P R O G R A M													//
//************************************************************************//
int main(void)
{
	FORMULA_FLAGS formula_flags;
	BOOLEAN parser_success;
	double result_value;

	//{{WIZARD_MAP(Initialization)
	io_init();
	lcd_init(16, 2, &LCD_PORT);	// LCD Using PORTC
	timers_init();
	//}}WIZARD_MAP(Initialization)
	// TODO: Add extra initialization here
	
	
	//{{WIZARD_MAP(Global interrupt)
	sei();
	//}}WIZARD_MAP(Global interrupt)

	//Full reset calculator
	reset_calculator(True);
	
	poweron();
	
	while(1)
	{
		//Input formula
		get_formula(&formula_flags);
		
		//Check for power off
		if(formula_flags.poweroff)
		{
			poweroff();
		}
		else if(formula_flags.formuladone)
		{
			//Generate parsable formula and...
			create_parsable_formula();
			// pass it to the parser.
			parser_success = parser_init(parser_formula, &result_value);
			
			//If parsing the expression was successful, display the result, else show error message.
			if(parser_success)
			{
				calc_status.submode = SM_NEWFORMULA;
				formula_status.cursorpos=0;
				formula_status.displeftpos=0;
				lcd_refresh();
				show_result(result_value);
				parser_status.ans = result_value;
			}
			else
			{
				//Show error message
				show_calc_error();
			}
		}
	}
}
