
#ifndef _PARSER_INCLUDED_
#define _PARSER_INCLUDED_
#include "types.h"

typedef struct
{
	unsigned char anglebase;
	double ans;
} PARSER_STATUS;

PARSER_STATUS parser_status;

BOOLEAN parser_init(char *formula, double *result);

#endif
