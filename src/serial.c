#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<termios.h>
#include<errno.h>
#include<string.h>
 
#include "serial.h"

int UART_Open(int fd,char* port)
{
	fd = open( port, O_RDWR|O_NOCTTY|O_NDELAY);
	if (FALSE == fd){
		perror("Can't Open Serial Port");
		return(FALSE);
	}
	if(fcntl(fd, F_SETFL, 0) < 0){
		printf("fcntl failed!\n");
		return(FALSE);
#ifdef MSG_TEST
	}else{
		printf("fcntl:= %d\n",fcntl(fd, F_SETFL,0));
#endif
	}
	freopen("/dev/console", "r", stdin);
	if(0 == isatty(STDIN_FILENO)){
		perror("Isatty");
		printf("Standard input is not a terminal device\n");
		return(FALSE);
		
#ifdef MSG_TEST
	}else{
		printf("Is a tty!\n");
#endif
	}  
	printf("File Descriptor:= %d\n", fd);
	return fd;
}
 
void UART_Close(int fd)
{
	    close(fd);
}

static int UART_Set(int fd,int speed,int flow_ctrl,int databits,int stopbits,int parity)
{
	   
	int   i;
	//int   status;
	int   speed_arr[] = { B115200, B19200, B9600, B4800, B2400, B1200, B300};
	int   name_arr[] = {115200,  19200,  9600,  4800,  2400,  1200,  300};
	struct termios options;

	if  ( tcgetattr( fd,&options)  !=  0){
		perror("SetupSerial 1");    
		return(FALSE); 
	}

	for ( i= 0;  i < sizeof(speed_arr) / sizeof(int);  i++){
		if  (speed == name_arr[i]){             
			cfsetispeed(&options, speed_arr[i]); 
			cfsetospeed(&options, speed_arr[i]);  
		}
	}     

	options.c_cflag |= CLOCAL;

	options.c_cflag |= CREAD;

	switch(flow_ctrl){
		case 0 :
			options.c_cflag &= ~CRTSCTS;
		break;   
		case 1 :
			options.c_cflag |= CRTSCTS;
		break;
		case 2 :
			options.c_cflag |= IXON | IXOFF | IXANY;
		break;
	}


	options.c_cflag &= ~CSIZE;
	switch (databits){  
		case 5:
			options.c_cflag |= CS5;
		break;
		case 6:
			options.c_cflag |= CS6;
		break;
		case 7:    
			options.c_cflag |= CS7;
		break;
		case 8:    
			options.c_cflag |= CS8;
		break;  
		default:   
			fprintf(stderr,"Unsupported data size\n");
			return (FALSE); 
	}

	switch (parity){  
		case 'n':
		case 'N':
			options.c_cflag &= ~PARENB; 
			options.c_iflag &= ~INPCK;    
		break; 
		case 'o':  
		case 'O':
			options.c_cflag |= (PARODD | PARENB); 
			options.c_iflag |= INPCK;             
		break; 
		case 'e': 
		case 'E':
			options.c_cflag |= PARENB;       
			options.c_cflag &= ~PARODD;       
			options.c_iflag |= INPCK;      
		break;
		case 's':
		case 'S':
			options.c_cflag &= ~PARENB;
			options.c_cflag &= ~CSTOPB;
		break; 
		default:  
			fprintf(stderr,"Unsupported parity\n");    
			return (FALSE); 
	} 

	switch (stopbits){  
		case 1:   
			 options.c_cflag &= ~CSTOPB; break; 
		case 2:   
			 options.c_cflag |= CSTOPB; break;
		default:   
			fprintf(stderr,"Unsupported stop bits\n"); 
			return (FALSE);
	}

	options.c_oflag &= ~OPOST;
	options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
	//options.c_lflag &= ~(ISIG | ICANON);

	options.c_cc[VTIME] = 1;
	options.c_cc[VMIN] = 1;

	options.c_oflag &= ~(ONLCR | OCRNL);      
	options.c_iflag &= ~(ICRNL | INLCR | IXOFF | IXON);

	tcflush(fd,TCIFLUSH);

	if (tcsetattr(fd,TCSANOW,&options) != 0){
		perror("com set error!\n");  
		return (FALSE); 
	}
	return (TRUE); 
}

int UART_Init(int fd, int speed,int flow_ctrl,int databits,int stopbits,int parity)
{
	//int err;

	if (UART_Set(fd, speed, flow_ctrl, databits, stopbits, parity) == FALSE) {                                                         
		return FALSE;
	}else{
		return  TRUE;
	}
}
 
int UART_Recv(int fd, char *rcv_buf,int data_len)
{
	int len,fs_sel;
	fd_set fs_read;
	struct timeval time;
	FD_ZERO(&fs_read);
	FD_SET(fd,&fs_read);
	time.tv_sec = 10;
	time.tv_usec = 0;

	fs_sel = select(fd+1,&fs_read,NULL,NULL,&time);
	if (fs_sel < 0)
		perror("select");
	if(fs_sel){
		len = read(fd,rcv_buf,data_len);
#ifdef MSG_TEST
		printf("GET len:= %d Date fs_sel = %d\n",len,fs_sel);
#endif
		return len;
	}else{
		printf("Fail to GET");
		return FALSE;
	}     
}

int UART_Send(int fd, char *send_buf,int data_len)
{
	int len = 0;
	len = write(fd,send_buf,data_len);
	if (len == data_len ){
		return len;
	}else{
		tcflush(fd,TCOFLUSH);
		return FALSE;
	}
}
