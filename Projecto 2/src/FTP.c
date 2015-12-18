#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>

#define DATASIZE 2000
#define PORT "ftp"


void progressBar(float current, float total) {
	float percentage = 100.0 * current / total;

	printf("\rCompleted: %6.2f%% [", percentage);

	int i, len = 51;
	int pos = percentage * len / 100.0;

	for (i = 0; i < len; i++)
		i <= pos ? printf("=") : printf(" ");

	printf("]");

	fflush(stdout);
}

// get sockaddr, IPv4 or IPv6:
void * get_in_addr(struct sockaddr *sa)
{
 if (sa->sa_family == AF_INET) {
   return &(((struct sockaddr_in*)sa)->sin_addr);
 }

 return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

void welcomeMessage(int sockfd){
  char msg[DATASIZE] = "";
	if (read(sockfd, msg, DATASIZE-1) > 0) {
		printf("%s\n", msg);
	}
}

void ftp_passive(int sockfd, char * serverHost, int * serverPort){
    char rsp[DATASIZE]= "";
  	write(sockfd, "pasv\n", strlen("pasv\n"));
  	read(sockfd, rsp, DATASIZE-1);

  	printf("%s\n", rsp);

  	int ip1, ip2, ip3, ip4, port1, port2;
  	sscanf(rsp, "227 Entering Passive Mode (%d,%d,%d,%d,%d,%d)", &ip1,
  			&ip2, &ip3, &ip4, &port1, &port2);

  	sprintf(serverHost, "%d.%d.%d.%d", ip1, ip2, ip3, ip4);

  	(*serverPort) = port1 * 256 + port2;
}
int ftp_login(int sockfd, char * user, char * password){
  char rsp[DATASIZE] = "";
  char userCmd[50] = "USER ";
  char passCmd[50] = "PASS ";

  strncat(userCmd, user, strlen(user));
  strncat(userCmd, "\n", strlen("\n"));

  strncat(passCmd, password, strlen(password));
  strncat(passCmd, "\n", strlen("\n"));

  write(sockfd, userCmd, strlen(userCmd));
  read(sockfd, rsp, DATASIZE-1);
  memset(&rsp[0], 0, sizeof(rsp));
  write(sockfd, passCmd, strlen(passCmd));
  read(sockfd, rsp, DATASIZE-1);
  printf("%s\n", rsp);
	if(rsp[0] == '5' && rsp[1] == '3' && rsp[2] == '0'){
		return -1;
	}
	return 0;
}

void ftp_specifie_file(int sockfd, char * path){
  char pathCmd[DATASIZE] = "RETR ";

  strncat(pathCmd, path, strlen(path));
  strncat(pathCmd, "\n", strlen("\n"));


  write(sockfd, pathCmd, strlen(pathCmd));

}
int ftp_file_size(int sockfd, char * path){
  char rsp[DATASIZE] = "";
  char fileCmd[DATASIZE] = "SIZE ";

  strncat(fileCmd, path, strlen(path));
  strncat(fileCmd, "\n", strlen("\n"));
  write(sockfd, fileCmd, strlen(fileCmd));
  read(sockfd, rsp, DATASIZE-1);

  int size;
  sscanf(rsp, "213 %d", &size);
  return size;

}
int ftp_connect(char * serverHost, int serverPort){
  struct	sockaddr_in tranfer_server; //server address handling

	memset(&tranfer_server,0,sizeof(tranfer_server)); // clean struct

	tranfer_server.sin_family = AF_INET;
	tranfer_server.sin_addr.s_addr = inet_addr(serverHost); //converts the host address from IPv4 numbers-and-dots notation into binary data in network byte order
	tranfer_server.sin_port = htons(serverPort); //converts the integer serverPort from host byte order to network byte order

  int transfer_sockfd;

  //open socket
	if ((transfer_sockfd = socket(AF_INET,SOCK_STREAM,0)) < 0){
    return -1;
  }

	//connect to the the transfer server
  if(connect(transfer_sockfd,(struct sockaddr *)&tranfer_server, sizeof(tranfer_server)) < 0){
    return -1;
	}
  return transfer_sockfd;
}

int parseURL(char * url, char * host, char * path, char * user, char * pass, char * filename){
	if(strchr(url, '@') != NULL){
		int p = sscanf(url, "ftp://%[^:]:%[^@]@%[^/]%s\n", user, pass, host, path);
		if(p != 4){
			if(sscanf(url, "ftp://%[^:]:@%[^/]%s\n", user, host, path) != 3){
				return -1;
			}
			strcpy(pass, "pass");
		}
	}
	else{
		sscanf(url, "ftp://%[^/]%s\n", host, path);
		strcpy(user, "ftp");
		strcpy(pass, "pass");
	}
	char * last = strrchr(path,'/');
	strcpy(filename, last+1);
	printf("%s\n", filename);
	return 0;
}

int main(int argc, char **argv){

  char host[200] = "";
  char path[DATASIZE] = "";
  char user[200] = "";
  char pass[200] = "";
  char filename[200] = "";
	//"ftp://ftp:pass@speedtest.tele2.net/100MB.zip"

	if(argc != 2){
		printf("usage: download ftp://[<user>:<password>@]<host>/<url-path> \n");
		exit(1);
	}

	if(parseURL(argv[1], host, path, user, pass, filename) == -1){
		printf("url does not match RFC1738 ftp://[<user>:<password>@]<host>/<url-path> \n");
		exit(1);
	}

  int status, sockfd;
  struct addrinfo hints;
  struct addrinfo *servinfo, *p; // will point to the results
  char s[INET6_ADDRSTRLEN];


  memset(&hints, 0, sizeof hints); // make sure the struct is empty
  hints.ai_family = AF_UNSPEC; // don't care IPv4 or IPv6
  hints.ai_socktype = SOCK_STREAM; // TCP stream sockets

  if ((status = getaddrinfo(host, PORT, &hints, &servinfo)) != 0) {
   fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
   exit(1);
  }


  p = servinfo;
  while(p != NULL){
    sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
    if(sockfd != -1){
      if(connect(sockfd, p->ai_addr, p->ai_addrlen) != -1){
        break;
      }
      else{
        close(sockfd);
      }
    }
    p = p->ai_next;
  }
  if (p == NULL) {
    fprintf(stderr, "Failed to connect to FTP server\n");
    exit(2);
  }

  inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr), s, sizeof s);
  freeaddrinfo(servinfo); // all done with this structure


  int serverPort;
  char serverHost[INET6_ADDRSTRLEN];

  welcomeMessage(sockfd);
  if(ftp_login(sockfd, user, pass) == -1){
		exit(3);
	}
  int size = ftp_file_size(sockfd, path);
	if(size <= 0){
		printf("File does not exist\n");
		exit(4);
	}
  printf("File of the size: %d bytes \n\n", size);
  ftp_passive(sockfd, serverHost, &serverPort);
  ftp_specifie_file(sockfd, path);

  int tranferfd;
  if((tranferfd = ftp_connect(serverHost,serverPort)) == -1){
    exit(5);
  }

	int file = open(filename, O_WRONLY | O_CREAT, 0777);

  int total = 0;
  progressBar(total,size);
	//read to the file
	char print[DATASIZE];
	int r=0;
	while ((r=read(tranferfd, print, DATASIZE)) != 0) {
    total+=r;
		write(file, print, r);
    progressBar(total,size);
	}

	close(file);
	close(tranferfd);
	close(sockfd);

  printf("\n");

  return EXIT_SUCCESS;
}
