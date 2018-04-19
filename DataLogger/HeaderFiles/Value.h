#ifndef _VALUE_H_
#define _VALUE_H_

#ifndef _WIN32
#include  <byteswap.h>
#else
#include <winsock2.h>
#endif

#define LEORDER	0x12AB
#define BEORDER	0xAB12

#ifdef _WIN32
#define swap_l(X) ntohl(X)
#define swap_s(X) ntohs(X)
#else
#define swap_l(X) __bswap_32(X) 
#define swap_s(X) __bswap_16(X)
#endif

#define SWAPVAL_L(X); X=swap_l(X);
#define SWAPVAL_S(X); X=swap_s(X);

#ifdef _WIN32
#define usleep(x); Sleep((x)/1000);
#define sleep(x); Sleep(x);
#endif

#define 	FunctionCode_DI  1
#define 	FunctionCode_DO  2
#define 	FunctionCode_AI  3

typedef struct _VALUE
{
	union
	{
		unsigned char BYTE;
		unsigned short USINT;
		unsigned int UDINT;
		short SINT;
		float REAL;
	};
	unsigned short int ORDERINFO;

	_VALUE() :	UDINT(0), ORDERINFO(LEORDER) {}

	bool Reorder() {
		if (ORDERINFO == BEORDER) {
			SWAPVAL_S(ORDERINFO);
			SWAPVAL_L(UDINT);
		} else if (ORDERINFO != LEORDER) {
			return false;
		}
		return true;
	}

	bool operator!=(_VALUE inp) {
		return (UDINT != inp.UDINT);
	}

} VALUE;

typedef struct _COMMAND
{
	bool  ACT;
	VALUE CMD;
	int   IOA;
	int   FNC;
}COMMAND;

#endif
