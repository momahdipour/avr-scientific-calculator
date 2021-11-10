//**************************************************************************
/*{{
  Parser module for AVRCalculator

Notes:
  Th eoriginal parser was in objective pascal which is conveted to C and added some
    additional functions to it.

}}*/
//**************************************************************************

//==============================================================================
// ORIGINAL PARSER HEADER: --
// Product name: CalcExpress
// Copyright 2000-2002 AidAim Software.
// Description:
//  CalcExpress is an interpreter for quick and easy
//  evaluation of mathematical expressions.
//  It is a smart tool easy in use.
//  Supports 5 operators, parenthesis, 18 mathematical functions and
//  user-defined variables.
// Date: 06/14/2001
//==============================================================================

//********************************************************************
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <pgmspace.h>
#include "parser.h"
//********************************************************************


//********************************************************************
#ifndef NULL
#define NULL 0
#endif

#define DecimalSeparator '.'

//********************************************************************


//********************************************************************
//Type definitions

typedef struct {
  int num;
  char *con;
  void *l, *r;
} TTree;

typedef TTree* PTree;
//********************************************************************

//********************************************************************
//Names of functions stored as strings in FLASH
FLASH char COS_STR[] = "cos";
FLASH char SIN_STR[] = "sin";
FLASH char TAN_STR[] = "tan";
FLASH char ABS_STR[] = "abs";
FLASH char SIGN_STR[] = "sign";
FLASH char SQRT_STR[] = "sqrt";
FLASH char LN_STR[] = "ln";
FLASH char LOG_STR[] = "log";
FLASH char EXP_STR[] = "exp";
FLASH char ARCSIN_STR[] = "arcsin";
FLASH char ARCCOS_STR[] = "arccos";
FLASH char ARCTAN_STR[] = "arctan";
FLASH char SINH_STR[] = "sinh";
FLASH char COSH_STR[] = "cosh";
FLASH char TANH_STR[] = "tanh";
FLASH char ARCSINH_STR[] = "arcsinh";
FLASH char ARCCOSH_STR[] = "arccosh";
FLASH char ARCTANH_STR[] = "arctanh";

FLASH char RAND_STR[] = "rand";
FLASH char ANS_STR[] = "ans";
//********************************************************************

//********************************************************************
//Parser global variables
  BOOLEAN Err;
  int bc;
  int prevlex, curlex;
  int pos;
  
  int n;
  char *c;
	char *gn_res;  //getnumber() function result goes to this dynamically allocated string
	
//********************************************************************

//********************************************************************
//Function prototypes
char *strcpy_alloc(char **des, const char *src);
char *strcat_alloc(char **des, const char *src);
char *strcatchar_alloc(char **des, char srcchar);
BOOLEAN parser_init(char *formula, double *result);
double calc(PTree t);
void Error(void);
void *deltree(PTree t);
PTree gettree(char *formula);
void *getop(char *s);
void getlex(char *s, int *num, char **con);
char *getnumber(char *s);
void *getsingleop(char *s);
PTree newnode(void);
//********************************************************************

//********************************************************************
//Functions

//////////////////////////////////////////////////////////////////////
//Functions: TTree->num:
// 1 : (
// 2 : )
// 3 : +
// 4 : -
// 5 : *
// 6 : /
// 7 : Number
// 8 : User defined variable
//
// 10: cos
// 11: sin
// 12: tan
// 13: log
// 14: abs
// 15: sign
// 16: sqrt
// 17: ln
// 18: exp
// 19: arcsin
// 20: arccos
// 21: arctan
// 23: sinh
// 24: cosh
// 25: tanh
// 26: Ran#
// 27: Ans
// 28: arcsinh
// 29: arccosh
// 30: arctanh
// 31: ^
//////////////////////////////////////////////////////////////////////

//********************************************************************
char *strcpy_alloc(char **des, const char *src)
{
	free(*des);
	*des = (char *)malloc(strlen(src)+1);
	strcpy(*des, src);
	return *des;
}
//********************************************************************

//********************************************************************
char *strcat_alloc(char **des, const char *src)
{
	char *des_temp = NULL;

	strcpy_alloc(&des_temp, *des);
	free(*des);
	*des = (char *)malloc(strlen(des_temp)+strlen(src)+1);
	strcpy(*des, des_temp);
	free(des_temp);
	strcpy(&((*des)[strlen(*des)]), src);
	return *des;
}
//********************************************************************

//********************************************************************
char *strcatchar_alloc(char **des, char srcchar)
{
  char *des_temp = NULL;
	UCHAR l;

	strcpy_alloc(&des_temp, *des);
	free(*des);
	*des = (char *)malloc(strlen(des_temp)+2);
	strcpy(*des, des_temp);
	free(des_temp);
	l=strlen(*des);
	(*des)[l+1] = 0;
	(*des)[l] = srcchar;
	return *des;
}
//********************************************************************

//********************************************************************
double correct_angle(double angle)
{
	switch(parser_status.anglebase)
	{
	case DEGREE:
		{
			//Convert degree to radian
			return(angle * M_PI / 180);
			break;
		}
	case RADIANS:
		{
			//No change
			return(angle);
			break;
		}
	case GRADIANS:
		{
			//Convert gradian to radian
			return(angle * M_PI / 200);
			break;
		}
	default:
		{
			return(angle);
		}
	}
}
//********************************************************************

//********************************************************************
double correct_arcangle(double arcangle)
{
	switch(parser_status.anglebase)
	{
	case DEGREE:
		{
			//Convert radian to degree
			return(arcangle * 180 / M_PI);
			break;
		}
	case RADIANS:
		{
			//No change
			return(arcangle);
			break;
		}
	case GRADIANS:
		{
			//Convert radian to gradian
			return(arcangle * 200 / M_PI);
			break;
		}
	default:
		{
			return(arcangle);
		}
	}
}
//********************************************************************

//********************************************************************
BOOLEAN parser_init(char *formula, double *result)
{
  PTree tree = NULL;

  Err = False;
  
  prevlex = 0;  
  curlex = 0;  
  pos = 0;
  bc = 0;
  tree = gettree(strlwr(formula));
  Err = ( (bc != 0) || Err );
  if(!Err)
  {
		*result=calc(tree);
  }
	tree = deltree(tree);
	free(gn_res);
	gn_res = NULL;
  return !Err;
}
//********************************************************************

//********************************************************************
double calc(PTree t)
{
  double r;
  double cr;
	char *endpos;

	cr = 0.0;
	switch(t->num) {
	  case 3: cr = calc(t->l) + calc(t->r); break;
		case 4: cr = calc(t->l) - calc(t->r); break;
		case 5: cr = calc(t->l) * calc(t->r); break;
		case 6: cr = calc(t->l) / calc(t->r); break;
		case 7:
			{
				//cr = atof(t->con); break;
				cr = strtod(t->con, &endpos);
				break;
			}
		//case 8: cr = args[StrToInt(t->con)]; break;  //For user defined variables
		case 9: cr = -calc(t->l); break;
		case 10: cr = cos(correct_angle(calc(t->l))); break;
		case 11: cr = sin(correct_angle(calc(t->l))); break;
		case 12: cr = tan(correct_angle(calc(t->l))); break;
		case 13:
			{
				cr = log10(calc(t->l));
				break;
			}
		case 14: cr = abs(calc(t->l)); break;
		case 15:
	    {
				r = calc(t->l);
				if( r < 0 ) cr = -1 ;
				else if( r > 0 ) cr = 1;
				else
					cr = 0;
        break;
      }
		case 16: cr = sqrt(calc(t->l)); break;
		case 17: cr = log(calc(t->l)); break;
		case 18: cr = exp(calc(t->l)); break;
		case 19: cr = correct_arcangle(asin(calc(t->l))); break;
		case 20: cr = correct_arcangle(acos(calc(t->l))); break;
		case 21: cr = correct_arcangle(atan(calc(t->l))); break;
		case 23: 
      {
        r = calc(t->l);
        cr = (exp(r) - exp(-r)) / 2;
        break;
      }
		case 24: 
			{
        r = calc(t->l);
				cr = (exp(r) + exp(-r)) / 2;
				break;
      }
		case 25:
			{
        r = calc(t->l);
        cr = (exp(r) - exp(-r)) / (exp(r) + exp(-r));
        break;
      }
		case 26:
			{
				srand(rand());
				cr = (double)rand() / (double) RAND_MAX;
				break;
			}
		case 27:
			{
				cr = parser_status.ans;
				break;
			}
		case 28:
			{
				r = calc(t->l);
				cr = log(r + sqrt(r * r + 1));
				break;
			}
		case 29:
			{
				r = calc(t->l);
				cr = log(r + sqrt(r * r - 1));
				break;
			}
		case 30:
			{
				r = calc(t->l);
				cr = log((1 + r) / (1 - r)) / 2;
				break;
			}
		case 31: cr = pow(calc(t->l), calc(t->r)); break;
	} //switch
	return cr;
} 
//********************************************************************

//********************************************************************
void Error(void)
{
  Err = True;
}
//********************************************************************

//********************************************************************
//Tree deletion
void *deltree(PTree t)
{
  if( t == NULL ) return(NULL);
  if( t->l != NULL) deltree(t->l);
  if( t->r != NULL) deltree(t->r);
	free(t->con);
  free(t);
  return(NULL);
}
//********************************************************************

//********************************************************************
PTree gettree(char *formula)
{
	
  PTree l = NULL, r = NULL, res = NULL;
	int op;

	//#########################
	if(Err) return(NULL); //###
	//#########################

	//s=s1;
  l = NULL;
	//  try
	l = (PTree) getop(formula);
	while( True )
	{
		if( (n==0) || (n==2) )  //n in [0,2] )
		{
			if( n == 2 ) bc--;
			return(l);
		}
		if( !( (n==3) || (n==4) ) )  //n in [3,4]) ) Error();
		{
			deltree(l);
		  Error();
			return(NULL);
		}
		op = n;
		r = (PTree) getop(formula);
		res = malloc(sizeof(TTree));
		res->l = l;
		res->r = r;
		res->num = op;
		res->con = NULL;
		l = res;
	}
	return(l);
	//   except
  //     deltree(l);
  //     Result = NULL;
  //   end;
}
//********************************************************************

//********************************************************************
void *getop(char *s)
{
  BOOLEAN neg;
  int op;
  PTree l = NULL, r = NULL, res = NULL;

	//#########################
	if(Err) return(NULL); //###
	//#########################

	neg = False;
  getlex(s, &n, &c);
  // Unary - or +
  if( (prevlex==0) || (prevlex==1) )  //prevlex in [0,1] )
  {
    if( n == 4 )
    {
      neg = True; 
      getlex(s, &n, &c);
    }
    if( n == 3 ) getlex(s, &n, &c);
  }
	l = (PTree) getsingleop(s);
	// 2nd operand **************
	while( (n==5) || (n==6) )  //n in [5,6] )
	{
		op = n;
		getlex(s, &n, &c);
		r = (PTree) getsingleop(s);
		res = malloc(sizeof(TTree));
		res->l = l;
		res->r = r;
		res->num = op;
		res->con = NULL;
		l = res;
	}
	// Unary minus
	if( neg )
	{
		res = malloc(sizeof(TTree));
		res->l = l;
		res->r = NULL;
		res->num = 9;
		res->con = NULL;
		l = res;
	}
	return (l);
}
//********************************************************************

//********************************************************************
//Read lexem from string
void getlex(char *s, int *num, char **con)
{
	//#########################
	if(Err) return;  //###
	//#########################

	strcpy_alloc(con, "");  //con[0] = 0;
	//skip spaces
	while( (pos < strlen(s)) && (s[pos] == ' ') ) 
	{
		pos++;
	}
	if( pos >= strlen(s) )
	{
		*num = 0;
		return;
	}
	
	switch(s[pos])
	{
	  case '(':
			{
				*num = 1; break;
			}
		case ')':
			{
				*num = 2; break;
			}
		case '+':
			{
				*num = 3; break;
			}
		case '-': 
			{
				*num = 4;
				if( ( pos < (strlen(s)-1) ) && isdigit(s[pos + 1]) && ((curlex==0) || (curlex==1)) )
				{
					pos++;
					strcpy_alloc(con, "-");
				  strcat_alloc(con, getnumber(s));
					pos--;
					*num = 7;
				}
				break;
			}
		case '*':
			{
				*num = 5; break;
			}
		case '/':
			{
				*num = 6; break;
			}
		case '^':
			{
				*num = 31; break;
			}
		case 'a': case 'A':
		case 'b': case 'B':
		case 'c': case 'C':
		case 'd': case 'D':
		case 'e': case 'E':
		case 'f': case 'F':
		case 'g': case 'G':
		case 'h': case 'H':
		case 'i': case 'I':
		case 'j': case 'J':
		case 'k': case 'K':
		case 'l': case 'L':
		case 'm': case 'M':
		case 'n': case 'N':
		case 'o': case 'O':
		case 'p': case 'P':
		case 'q': case 'Q':
		case 'r': case 'R':
		case 's': case 'S':
		case 't': case 'T':
		case 'u': case 'U':
		case 'v': case 'V':
		case 'w': case 'W':
		case 'x': case 'X':
		case 'y': case 'Y':
		case 'z': case 'Z':
		case '_':
			{
				while( (pos < strlen(s)) && ( isalnum(s[pos]) || (s[pos]=='_') ) )  //(s[pos] in ['a'..'z', 'A'..'Z', '_', '1'..'9', '0']) )
				{
					strcatchar_alloc(con, s[pos]);
					pos++;
				}
				pos--;
				*num = 8;
        if( strcmp_P(*con , COS_STR) == 0 ) *num = 10;
        if( strcmp_P(*con , SIN_STR) == 0 ) *num = 11;
        if( strcmp_P(*con , TAN_STR) == 0 ) *num = 12;
        if( strcmp_P(*con , ABS_STR) == 0 ) *num = 14;
        if( strcmp_P(*con , SIGN_STR) == 0 ) *num = 15;
        if( strcmp_P(*con , SQRT_STR) == 0 ) *num = 16;
        if( strcmp_P(*con , LN_STR) == 0 ) *num = 17;
        if( strcmp_P(*con , LOG_STR) == 0 ) *num = 13;
        if( strcmp_P(*con , EXP_STR) == 0 ) *num = 18;
        if( strcmp_P(*con , ARCSIN_STR) == 0 ) *num = 19;
        if( strcmp_P(*con , ARCCOS_STR) == 0 ) *num = 20;
        if( strcmp_P(*con , ARCTAN_STR) == 0 ) *num = 21;
        if( strcmp_P(*con , SINH_STR) == 0 ) *num = 23;
        if( strcmp_P(*con , COSH_STR) == 0 ) *num = 24;
        if( strcmp_P(*con , TANH_STR) == 0 ) *num = 25;
        if( strcmp_P(*con , ARCSINH_STR) == 0 ) *num = 28;
        if( strcmp_P(*con , ARCCOSH_STR) == 0 ) *num = 29;
        if( strcmp_P(*con , ARCTANH_STR) == 0 ) *num = 30;

        if( strcmp_P(*con , RAND_STR) == 0 ) *num = 26;
        if( strcmp_P(*con , ANS_STR) == 0 ) *num = 27;
				//if( *num == 8 ) con = IntToStr(FVariables.IndexOf(con));
        break;
			}
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
			{
        strcpy_alloc(con, getnumber(s));
        pos--;
        *num = 7;
        break;
      }
	}  //switch
	pos++;
	prevlex = curlex;
	curlex = *num;
}
//********************************************************************

//********************************************************************
//Get number from string
char *getnumber(char *s)
{
	//#########################
	if(Err) return(NULL); //###
	//#########################

	strcpy_alloc(&gn_res, "");
	
// try
 
      //Begin
	while( (pos < strlen(s)) && isdigit(s[pos]) )
	{
		strcatchar_alloc(&gn_res, s[pos]);
		pos++;
	}  
	if(pos >= strlen(s)) return(gn_res);
	if(s[pos] == DecimalSeparator)
	{
		//Fraction part
		strcatchar_alloc(&gn_res, DecimalSeparator);
		pos++;
		if( (pos >= strlen(s)) || (!(isdigit(s[pos]))) )
		{
			Error();  //"Wrong number.");
			return(NULL);
		}
		while( (pos < strlen(s)) && isdigit(s[pos]) )
		{
			strcatchar_alloc(&gn_res, s[pos]);
			pos++;
		}
	}
	if(pos >= strlen(s))
	{
		return(gn_res);
	}
	//Power
	if( (s[pos] != 'e') && (s[pos] != 'E') ) return(gn_res);
	strcatchar_alloc(&gn_res, s[pos]);
	pos++;
	if( pos >= strlen(s) )
	{
		Error();  //"Wrong number.");
		return(NULL);
	}
	if( (s[pos]=='-') || (s[pos]=='+') )
	{
		strcatchar_alloc(&gn_res, s[pos]);
		pos++;
	}
	if( (pos >= strlen(s)) || (!(isdigit(s[pos]))) )
	{
		Error();  //"Wrong number.");
		return(NULL);
	}
	while( (pos < strlen(s)) && isdigit(s[pos]) )
	{
		strcatchar_alloc(&gn_res, s[pos]);
		pos++;
	}

//    except  **********
    //    end;

	return gn_res;
}
//********************************************************************

//********************************************************************
void *getsingleop(char *s)
{
  int op, bracket;
  PTree l = NULL, r = NULL, res = NULL;
	char *opc = NULL;

	//#########################
	if(Err) return(NULL); //###
	//#########################
	
  l = NULL;
//    try
	if( n == 1 )
	{
		bc++;
		l = gettree(s);
	}
	else
	{
		// First operand
		if( !( (n==7) || (n==8) || ( (n>=10) && (n<=30) ) ) )  //n in [7,8,10..30]) ) Error('');
		{
		  Error();  //Error('');
			return(NULL);
		}
		op = n;
		strcpy_alloc(&opc, c);
		if( (n==7) || (n==8) )  // n in [7,8] )
		{
			// Number or variable
			l = newnode();
			l->num = op;
			strcpy_alloc(&(l->con), opc);
		}
		else
		{
			//Function
			getlex(s, &n, &c);
			if( n != 1 )
			{
				Error();  //'');
				free(opc);
				opc = NULL;
				return(NULL);
			}
			bc++;
			l = newnode();
			l->l = gettree(s);
			l->num = op;
			strcpy_alloc(&(l->con), opc);
		}
		free(opc);
		opc = NULL;
	}
	//Operation symbol
	getlex(s, &n, &c);
	//Power symbol
	while( n == 31 )
	{
		getlex(s, &n, &c);
		bracket = 0;
		if( n == 1 )
		{
			bracket = 1;
			getlex(s, &n, &c);
		}
		if( (n != 7) && (n != 8) )
		{
			Error();  //'');
			deltree(l);
			return(NULL);
		}
		r = newnode();
		r->num = n;
		strcpy_alloc(&(r->con) , c);
		res = newnode();
		res->l = l;
		res->r = r;

		res->num = 31;
		l = res;
		if( bracket == 1 )
		{
			getlex(s, &n, &c);
			if( n != 2 )
			{
				Error();  //'');
				deltree(res);
				return(NULL);
			}
		}
		getlex(s, &n, &c);
	}
	return(l);
	//     except
	//       deltree(l);
	//       return(NULL);
	//     end;
}
//********************************************************************

//********************************************************************
PTree newnode(void)
{
  PTree Result;
	
	//#########################
	if(Err) return(NULL); //###
	//#########################

	Result = malloc(sizeof(TTree));
  Result->l = NULL;
  Result->r = NULL;
	Result->con = NULL;
  return(Result);
}
//********************************************************************
