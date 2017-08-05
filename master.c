//
//	
//

#include <sys/msg.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tipomensaje.h"

#define TAM 512
#define PH 7.2
#define CL 300
#define DEFAULT 28




float tCal;
float tAgua;
float tDes;
float pH;
float cl;
float nivelPH;
float nivelCL;
key_t Clave1;
int Id_Cola_Mensajes;
Mi_Tipo_Mensaje Un_Mensaje;
char buffer2[TAM];
char dato[TAM];
bool cliente;		//indica si hay algun cliente conectado para recibir datos de interface grafica
bool calentador;	//indica si el usuario desea o no usar el calentador


void enviarMensaje(int ti, enum tipo t, char *men);
void pedirTempCalentador();
void ActivarCalentador();


//
// Estructura para los mensajes que se quieren enviar y/o recibir. Deben llevar
// obligatoriamente como primer campo un long para indicar un identificador
// del mensaje. 
// Los siguientes campos son la información que se quiera transmitir en el  
// mensaje. Cuando más adelante, en el código, hagamos un cast a 
// (struct msgbuf *), todos los campos de datos los verá el sistema como
// un único (char *)
//


main()
{
	cliente = false;
	calentador = false;
	tCal=0.0;
	tAgua=0.0;
	tDes=0.0;
	pH=0.0;
	cl=0.0;
	nivelPH=0.0;
	nivelCL=0.0;

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
	msgctl (Id_Cola_Mensajes, IPC_RMID, (struct msqid_ds *)NULL);
	Id_Cola_Mensajes = msgget (Clave1, 0600 | IPC_CREAT);
	if (Id_Cola_Mensajes == -1)
	{
		printf("Error al obtener identificador para cola mensajes\n");
		exit (-1);
	}
	//pedirTempCalentador();
	//ActivarCalentador();
	while(1){


	//
	//	Se recibe un mensaje del otro proceso. Los parámetros son:
	//	- Id de la cola de mensajes.
	//	- Dirección del sitio en el que queremos recibir el mensaje,
	//	convirtiéndolo en puntero a (struct msgbuf *).
	//	- Tamaño máximo de nuestros campos de datos. 
	//	- Identificador del tipo de mensaje que queremos recibir. En este caso
	//	se quiere un mensaje de tipo 2. Si ponemos tipo 1, se extrae el mensaje
	//	que se acaba de enviar en la llamada anterior a msgsnd().
	//	- flags. En este caso se quiere que el programa quede bloqueado hasta
	//	que llegue un mensaje de tipo 2. Si se pone IPC_NOWAIT, se devolvería
	//	un error en caso de que no haya mensaje de tipo 2 y el programa
	//	continuaría ejecutándose.
	//
		msgrcv (Id_Cola_Mensajes, (struct msgbuf *)&Un_Mensaje,
				sizeof(Un_Mensaje.tip) + sizeof(Un_Mensaje.Mensaje), 
				A_MASTER, 0);


		switch(Un_Mensaje.tip){
				
				case CLIENTE_NUEVO:
					printf("Recibido mensaje de %d\n", Un_Mensaje.Id_Mensaje);
					printf("Tipo = %d\n",Un_Mensaje.tip);
					printf("Tipo = CLIENTE_NUEVO\n");
					printf( "Mensaje = %s\n",Un_Mensaje.Mensaje);
					cliente=true;
					enviarMensaje(A_CALENTADOR,  ACTUALIZAR, "ddd");
					break;

				case ACTUALIZADO:
					printf("Recibido mensaje de %d\n", Un_Mensaje.Id_Mensaje);
					printf("Tipo = %d\n",Un_Mensaje.tip);
					printf("Tipo = ACTUALIZADO\n");
					printf( "Mensaje = %s\n",Un_Mensaje.Mensaje);
					strcpy(dato, Un_Mensaje.Mensaje);
					if(cliente){
						
						enviarMensaje(A_GUI,  ACTUALIZAR, dato);
					}
					break;
				case SIN_CLIENTE:
					printf("Recibido mensaje de %d\n", Un_Mensaje.Id_Mensaje);
					printf("Tipo = %d\n",Un_Mensaje.tip);
					printf("Tipo = SIN_CLIENTE\n");
					printf( "Mensaje = %s\n",Un_Mensaje.Mensaje);
					cliente=true;
					enviarMensaje(A_CALENTADOR,  NO_ACTUALIZAR, "ddd");
					break;

				case TEMPERATURA_DESEADA:
					printf("Recibido mensaje de %d\n", Un_Mensaje.Id_Mensaje);
					printf("Tipo = %d\n",Un_Mensaje.tip);
					printf("Tipo = TEMPERATURA_DESEADA\n");
					printf( "Mensaje = %s\n",Un_Mensaje.Mensaje);
					tDes=atof(Un_Mensaje.Mensaje);
					printf( "Tdes = %f\n", tDes);
					break;


			}
	}

	//
	//	Se borra y cierra la cola de mensajes.
	// IPC_RMID indica que se quiere borrar. El puntero del final son datos
	// que se quieran pasar para otros comandos. IPC_RMID no necesita datos,
	// así que se pasa un puntero a NULL.
	//
	msgctl (Id_Cola_Mensajes, IPC_RMID, (struct msqid_ds *)NULL);
}


void enviarMensaje(int ti, enum tipo t, char *men){
	Un_Mensaje.Id_Mensaje = ti;
	Un_Mensaje.tip = t;
	strcpy(Un_Mensaje.Mensaje , men);
	msgsnd (Id_Cola_Mensajes, (struct msgbuf *)&Un_Mensaje, 
			sizeof(Un_Mensaje.tip)+sizeof(Un_Mensaje.Mensaje), 
			IPC_NOWAIT);
		
}

void pedirTempCalentador(){
	//
	//	Se rellenan los campos del mensaje que se quiere enviar.
	//	El Id_Mensaje es un identificador del tipo de mensaje. Luego se podrá
	//	recoger aquellos mensajes de tipo 1, de tipo 2, etc.
	//	Dato_Numerico es un dato que se quiera pasar al otro proceso. Se pone, 
	//	por ejemplo 29.
	//	Mensaje es un texto que se quiera pasar al otro proceso.
	//
	Un_Mensaje.Id_Mensaje = A_CALENTADOR;
	Un_Mensaje.tip = ACTUALIZAR;
	strcpy (Un_Mensaje.Mensaje, "29");
	msgsnd (Id_Cola_Mensajes, (struct msgbuf *)&Un_Mensaje, 
		sizeof(Un_Mensaje.tip)+sizeof(Un_Mensaje.Mensaje), 
		IPC_NOWAIT);
}

void ActivarCalentador(){
	//
	//	Se rellenan los campos del mensaje que se quiere enviar.
	//	El Id_Mensaje es un identificador del tipo de mensaje. Luego se podrá
	//	recoger aquellos mensajes de tipo 1, de tipo 2, etc.
	//	Dato_Numerico es un dato que se quiera pasar al otro proceso. Se pone, 
	//	por ejemplo 29.
	//	Mensaje es un texto que se quiera pasar al otro proceso.
	//
	Un_Mensaje.Id_Mensaje = A_CALENTADOR;
	Un_Mensaje.tip = ACTIVAR;
	strcpy (Un_Mensaje.Mensaje, "29");
	msgsnd (Id_Cola_Mensajes, (struct msgbuf *)&Un_Mensaje, 
		sizeof(Un_Mensaje.tip)+sizeof(Un_Mensaje.Mensaje), 
		IPC_NOWAIT);
}
