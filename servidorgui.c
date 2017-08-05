#include <sys/msg.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <unistd.h>
#include <signal.h>
#include <netdb.h>
#include <time.h>
#include <pthread.h>
#include "tipomensaje.h"

void enviarMensaje(int ti, enum tipo t, char *men);
int extraercomando(char *com);

int newsockfd, n;
key_t Clave1;
int Id_Cola_Mensajes;
Mi_Tipo_Mensaje Un_Mensaje;

void error(const char *msg)
{
    perror(msg);
    exit(1);
}

void* recvFn( void* data )
{
    char buffer[256];
    while(1){

    memset( buffer, sizeof(buffer), 0);
        n = read(newsockfd,buffer,255);
        if(n>0){
            printf("cliet: ");
            printf("%s",buffer);    
        }
	char *comm;
	char *num;
	comm=strtok(buffer, " ");
	switch (extraercomando(comm)){
		case 2:
			printf("setear\n");
			num = strtok(NULL, "\n");
			enviarMensaje(A_MASTER,  TEMPERATURA_DESEADA, num);
			break;
		default :
			printf("default\n");
			break;


	}

    }
    return NULL;
}

void* sendFn( void* data )
{
    char temp[255], buffer[255];
    while(1){
    memset( buffer, sizeof(buffer), 0);
           printf("enviando\n");
	msgrcv (Id_Cola_Mensajes, (struct msgbuf *)&Un_Mensaje,
			sizeof(Un_Mensaje.tip) + sizeof(Un_Mensaje.Mensaje), 
			A_GUI, 0);    
	//strcpy(temp, Un_Mensaje.Mensaje);           
        sprintf(buffer,"clent: %s",Un_Mensaje.Mensaje);
        n = write(newsockfd,buffer,strlen(buffer));
	           printf("enviando %s\n", buffer);
    }
    return NULL;
}
int main(int argc, char *argv[])
{

	//
	// Igual que en cualquier recurso compartido (memoria compartida, semaforos 
	//  o colas) se obtien una clave a partir de un fichero existente cualquiera 
	//  y de un entero cualquiera. Todos los procesos que quieran compartir este
	//  semaforo, deben usar el mismo fichero y el mismo entero.
	//
	Clave1 = ftok ("/bin/ls", 33);
	if (Clave1 == (key_t)-1)
	{
		printf("Error al obtener clave para cola mensajes\n");
		exit(-1);
	}

	//
	//	Se crea la cola de mensajes y se obtiene un identificador para ella.
	// El IPC_CREAT indica que cree la cola de mensajes si no lo está ya.
	// el 0600 son permisos de lectura y escritura para el usuario que lance
	// los procesos. Es importante el 0 delante para que se interprete en
	// octal.
	//
	Id_Cola_Mensajes = msgget (Clave1, 0600 | IPC_CREAT);
	if (Id_Cola_Mensajes == -1)
	{
		printf("Error al obtener identificador para cola mensajes\n");
		exit (-1);
	}

    int sockfd, portno;
    socklen_t clilen;

    struct sockaddr_in serv_addr, cli_addr;

    if (argc < 2) {
        fprintf(stderr,"ERROR, no port provided\n");
        exit(1);
    }

    pthread_t recvThread, sendThread;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
        error("ERROR opening socket");

memset( &serv_addr, sizeof(serv_addr), 0);
    portno = atoi(argv[1]);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);

    int on = 1;
    if ( setsockopt( sockfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof( on ) ) != 0 ) {
        close( sockfd );
        return -1;
    }
	enviarMensaje(A_MASTER,  SIN_CLIENTE, "22");

    if (bind(sockfd, (struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
        error("ERROR on binding");
	while(1){
	
    listen(sockfd,1);

    clilen = sizeof(cli_addr);
	printf("antes de aceptar\n");
    newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
	printf("despues de aceptar\n");
    if (newsockfd < 0) 
        error("ERROR on accept");

        n = 0;
        int rc;
		printf("antes de enviar\n");
		enviarMensaje(A_MASTER,  CLIENTE_NUEVO, "22");
        rc = pthread_create( &recvThread, NULL, recvFn, NULL);
        if(rc){
            printf("error in receive-message thread\n");
            return -1;
        }


        rc = pthread_create( &sendThread, NULL, sendFn, NULL);
        if(rc){
            printf("error in send-message thread\n");
            return -1;
        }
	}
	
        close(newsockfd);
        close(sockfd);
    pthread_cancel(recvThread);
    pthread_cancel(sendThread);
    return 0; 
}
int extraercomando(char *com){
	printf("com %s\n", com);
	if(!strcmp(com,"desconectar\n"))return 0;
	if(!strcmp(com,"listar\n"))return 1;
	if(!strcmp(com,"setear"))return 2;	
	if(!strcmp(com,"diario_precipitacion"))return 3;
	if(!strcmp(com,"mensual_precipitacion"))return 4;
	if(!strcmp(com,"promedio"))return 5;
	return 6;	
}

void enviarMensaje(int ti, enum tipo t, char *men){
	Un_Mensaje.Id_Mensaje = ti;
	Un_Mensaje.tip = t;
	strcpy(Un_Mensaje.Mensaje , men);
	msgsnd (Id_Cola_Mensajes, (struct msgbuf *)&Un_Mensaje, 
			sizeof(Un_Mensaje.tip)+sizeof(Un_Mensaje.Mensaje), 
			IPC_NOWAIT);
		
}

