/*Non-Canonical Input Processing*/

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>


#define BAUDRATE B38400
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1
#define F 0x7e
#define A 0x03
#define C_UA 0x03
#define C_SET 0x07
#define BCC A^C_SET

unsigned char SET[5];
unsigned char UA[5];

volatile int STOP=FALSE;


int main(int argc, char** argv)
{
    int fd,c, res;
    struct termios oldtio,newtio;
    char buf[255];

    if ( (argc < 2) || 
  	     ((strcmp("/dev/ttyS0", argv[1])!=0) && 
  	      (strcmp("/dev/ttyS1", argv[1])!=0) )) {
      printf("Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS1\n");
      exit(1);
    }


  /*
    Open serial port device for reading and writing and not as controlling tty
    because we don't want to get killed if linenoise sends CTRL-C.
  */
  
    
    fd = open(argv[1], O_RDWR | O_NOCTTY );
    if (fd <0) {perror(argv[1]); exit(-1); }

    if ( tcgetattr(fd,&oldtio) == -1) { /* save current port settings */
      perror("tcgetattr");
      exit(-1);
    }

    bzero(&newtio, sizeof(newtio));
    newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
    newtio.c_iflag = IGNPAR;
    newtio.c_oflag = 0;

    /* set input mode (non-canonical, no echo,...) */
    newtio.c_lflag = 0;

    newtio.c_cc[VTIME]    = 0;   /* inter-character timer unused */
    newtio.c_cc[VMIN]     = 1;   /* blocking read until 5 chars received */



  /* 
    VTIME e VMIN devem ser alterados de forma a proteger com um temporizador a 
    leitura do(s) próximo(s) caracter(es)
  */



    tcflush(fd, TCIOFLUSH);

    if ( tcsetattr(fd,TCSANOW,&newtio) == -1) {
      perror("tcsetattr");
      exit(-1);
    }

    printf("New termios structure set\n");

///////////////////


UA[0] = F;
UA[1] = A;
UA[2] = C_UA;
UA[3] = UA[1]^UA[2];
UA[4] = F;

int i,n= 0;	

sleep(4);
for(i = 0;i < 5;i++)
{
	res = read(fd, &SET[i], 1);
}
if(res < 0) 
	printf("erro bitches \n");

printf("SET 0: %x \n SET 1:  %x \n SET 2:  %x \n  SET 3:  %x \n  SET 4: %x\n", SET[0], SET[1], SET[2], SET[3], SET[4]);


write(fd,UA,5);

	

/*
	int cur = 0;
	char cc;
    while (STOP==FALSE) {     

      res = read(fd, &cc, 1);
		if(res > 0) {
     		 buf[cur] = cc;
		cur++;

		if(cc == '\0')
			break;
	} else {
		printf("ERROR!\n");
    }
}

	
	printf("Received: %s\n", buf);
	write(fd, buf, cur);

*/
  /* 
    O ciclo WHILE deve ser alterado de modo a respeitar o indicado no guião 
  */


	
	sleep(5);
    tcsetattr(fd,TCSANOW,&oldtio);
    close(fd);
    return 0;

}

