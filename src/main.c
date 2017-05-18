#include <stdio.h>
#include <serial.h>

#define TTYSX "/dev/ttyS1"

int main(int argc, char *argv[])
{
	printf("Module: Testing!\n");

	int ttyfd = 0;
	char buffer[512];
	ttyfd = UART_Open(ttyfd, TTYSX);
	if(ttyfd < 0)
	{
		printf("ttySx open fail!\n");
		return -1;
	}

	if(UART_Init(ttyfd, 115200, 0, 8, 1, 'N') < 0)
	{
		printf("ttySx init fail!\n");
		return -1;
	}

	while(1)
	{
		if(UART_Recv(ttyfd, buffer, 512) > 0)
		{
			printf("%s\n", buffer);
		}
		
	}
	return 0;
}
