#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "stm32f0xx.h"

void vApplicationStackOverflowHook (void)
{

}

int main(void)
{
	SystemInit();
	while(1);
	return 1;
}
