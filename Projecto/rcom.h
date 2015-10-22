#define BAUDRATE B38400
#define MODEMDEVICE "/dev/ttyS1"
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1
#define F 0x7E
#define A_SR 0x03
#define A_RS 0x01
#define C_SET 0x07
#define BCC (A^C_SET)

typedef enum {TRANSMITTER, RECEIVER} Status;

struct applicationLayer {
	int fileDescriptor; /*Descritor correspondente à porta série*/
	int status; /*TRANSMITTER | RECEIVER*/
}

struct linkLayer {
	char port[20]; /*Dispositivo /dev/ttySx, x = 0, 1*/
	int baudRate; /*Velocidade de transmissão*/
	unsigned int sequenceNumber;   /*Número de sequência da trama: 0, 1*/
	unsigned int timeout; /*Valor do temporizador: 1 s*/
	unsigned int numTransmissions; /*Número de tentativas em caso de falha*/
	char frame[MAX_SIZE]; /*Trama*/
}
/*
argumentos
 - porta: COM1, COM2, ...
 - flag: TRANSMITTER / RECEIVER
retorno
	identificador da ligação de dados
	valor negativo em caso de erro
*/
int llopen(int porta, TRANSMITTER | RECEIVER);

/*
argumentos
	fd: identificador da ligação de dados
	buffer: array de caracteres a transmitir
	length: comprimento do array de caracteres
retorno
	número de caracteres escritos
	valor negativo em caso de erro
*/
int llwrite(int fd, char * buffer, int length);

/*
argumentos
	fd: identificador da ligação de dados
	buffer: array de caracteres recebidos
retorno
	comprimento do array (número de caracteres lidos)
	valor negativo em caso de erro
*/
int llread(int fd, char * buffer);

/*
argumentos
	fd: identificador da ligação de dados
retorno
	valor positivo em caso de sucesso
	valor negativo em caso de erro
*/
int llclose(int fd);
