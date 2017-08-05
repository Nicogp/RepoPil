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
#include "tipomensaje.h"

#define TINI 60 //tiempo del periodo de conexion al servidor con el sensor de T
#define TAM 256 //tamaño del buffer del soccket
//#define PORT 8000 //puerto de la conexion


char buffer[TAM];
char dato[TAM];
char tchar[TAM];
FILE *fs;
char *HOST = "192.168.0.15";
int PORT= 8000;
int sockfd, puerto, n;
float tactual;
bool actualizar;
struct sockaddr_in serv_addr;
struct hostent *server;
key_t Clave1;
int Id_Cola_Mensajes;
Mi_Tipo_Mensaje Un_Mensaje;




void tratamientoSenhal (int);
void enviarMensaje(int ti, enum tipo t, char *men);

main()
{
	
	
	
	tactual=0.0;
	actualizar=false;


	puerto= PORT;
	sockfd=socket(AF_INET, SOCK_STREAM, 0);
	if(sockfd <0){
		perror("Error apertura de socket");
		exit(1);
	}
	server = gethostbyname(HOST);
	memset((char *) &serv_addr, '0', sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	bcopy ((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
	serv_addr.sin_port= htons(puerto);
	if(server==NULL){
		fprintf(stderr, "Error, host no existe\n");
		exit(0);
	}
	if(connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr))<0){
		perror("conexion");
		//exit(0);
	}

	/* Estructura con el contador de tiempo */
	struct itimerval contador;

	/* Valor inicial del contador */
	struct timeval tiempoInicial;

	/* Tiempo de repetición del contador */
	struct timeval tiempoRepeticion;

	/* Se rellena el tiempo inicial del contador con 1 segundos */
	tiempoInicial.tv_sec=TINI;
	tiempoInicial.tv_usec=0;

	/* Se rellena el tiempo de repetición con 1 segundo */
	tiempoRepeticion.tv_sec=TINI;
	tiempoRepeticion.tv_usec=0;

	/* Se rellenan los datos del contador */
	contador.it_value=tiempoInicial;
	contador.it_interval=tiempoRepeticion;
	
	/* Se cambia el tratamiento de la señal por defecto para que llame a
	 * nuestra función tratamientoSenhal */
	signal (SIGALRM, tratamientoSenhal);

	/* Se pone en marcha el contador.
	 * La primera vez tardará 1 segundos en saltar, según indicamos en
	 * tiempoInicial. Luego saltará automáticamente cada 1 segundo, como
	 * indicamos en tiempoRepeticion. */
	setitimer (ITIMER_REAL, &contador, NULL);


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

	//
	//	Se recibe un mensaje del otro proceso. Los parámetros son:
	//	- Id de la cola de mensajes.
	//	- Dirección del sitio en el que queremos recibir el mensaje,
	//	convirtiéndolo en puntero a (struct msgbuf *).
	//	- Tamaño máximo de nuestros campos de datos. 
	//	- Identificador del tipo de mensaje que queremos recibir. En este caso
	//	se quiere un mensaje de tipo 1, que es el que envia el proceso cola1.cc
	//	- flags. En este caso se quiere que el programa quede bloqueado hasta
	//	que llegue un mensaje de tipo 1. Si se pone IPC_NOWAIT, se devolvería
	//	un error en caso de que no haya mensaje de tipo 1 y el programa
	//	continuaría ejecutándose.
	//
	int ret;
	while(1){
		ret=  msgrcv (Id_Cola_Mensajes, (struct msgbuf *)&Un_Mensaje,
			sizeof(Un_Mensaje.tip) + sizeof(Un_Mensaje.Mensaje), 
			A_CALENTADOR, 0);
		if(ret!=-1){

			switch(Un_Mensaje.tip){
				
				case NADA:
					
					break;
	
				case ACTUALIZAR:
					printf("Tipo = %d\n",Un_Mensaje.tip);
					printf("Tipo = ACTUALIZAR\n");
					printf( "Mensaje = %s\n",Un_Mensaje.Mensaje);
					actualizar=true;//activo flag de actualizacion y mando dato actual
					break;
				
				case NO_ACTUALIZAR:
					printf("Tipo = %d\n",Un_Mensaje.tip);
					printf("Tipo = NO_ACTUALIZAR\n");
					printf( "Mensaje = %s\n",Un_Mensaje.Mensaje);
					actualizar=false;
					break;
			}
			

		}
		else{
			//Un_Mensaje.Id_Mensaje = 2;
			//Un_Mensaje.Dato_Numerico = 13;
			//strcpy (Un_Mensaje.Mensaje, dato);
			//printf("else\n");
	//
	//	Se envia el mensaje. Los parámetros son:
	//	- Id de la cola de mensajes.
	//	- Dirección al mensaje, convirtiéndola en puntero a (struct msgbuf *)
	//	- Tamaño total de los campos de datos de nuestro mensaje, es decir
	//	de Dato_Numerico y de Mensaje
	//	- Unos flags. IPC_NOWAIT indica que si el mensaje no se puede enviar
	//	(habitualmente porque la cola de mensajes esta llena), que no espere
	//	y de un error. Si no se pone este flag, el programa queda bloqueado
	//	hasta que se pueda enviar el mensaje.
	//
			//msgsnd (Id_Cola_Mensajes, (struct msgbuf *)&Un_Mensaje, 
			//	sizeof(Un_Mensaje.Dato_Numerico)+sizeof(Un_Mensaje.Mensaje), 
			//	IPC_NOWAIT);
		}
	}
}



void enviarMensaje(int ti, enum tipo t, char *men){
	Un_Mensaje.Id_Mensaje = ti;
	Un_Mensaje.tip = t;
	strcpy(Un_Mensaje.Mensaje , men);
	msgsnd (Id_Cola_Mensajes, (struct msgbuf *)&Un_Mensaje, 
			sizeof(Un_Mensaje.tip)+sizeof(Un_Mensaje.Mensaje), 
			IPC_NOWAIT);
		
}


/* Tratamiento de la señal SIGALRM.
 */
void tratamientoSenhal (int idSenhal)
{	
	fs = fopen("datos.txt" , "a");
	if(fs == NULL) 
	   {
	      perror("Error abrir archivo de escritura");
	      //return(-1);
	}
	
	n=write(sockfd, "temperatura\n", 13);
	if(n<0){
		perror("escritura");
		exit(0);
	}
	buffer[strlen(buffer)-1]='\0';
	memset(dato, '\0', TAM);
	n = read(sockfd, dato, TAM);
	if(n<0){
		perror("lectura de sock");
		exit(0);
	}
	printf("Respuesta: %s ", dato);
	strcpy(tchar, dato);
	tactual=atof(dato);
	printf("T Actual: %f\n",tactual);
	time_t tiempo = time(0);
        struct tm *tlocal = localtime(&tiempo);
        char output[128];
        strftime(output,128,",%d/%m/%y,%H:%M:%S\n",tlocal);
        printf("%s\n",output);
	strcat( dato, output);
        printf("%s\n",dato);
	fprintf(fs, dato);
	memset(buffer, '\0', TAM);
	n = read(sockfd, buffer, TAM);
	if(n<0){
		perror("lectura de sock");
		exit(0);
	}	
	
	fclose(fs);

	//una vez q se recibio dato del sensor y se guardo en el archivo
	//chequeo si hay q enviar la informacion actualizada o si se debe enviar cuando la temperatura supera la deseada
	if(actualizar){
		enviarMensaje(A_MASTER,  ACTUALIZADO, dato);
	}
}



