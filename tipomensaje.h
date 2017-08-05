//Definimos los tipos de mensajes q se intercambian

#define A_CALENTADOR 	1
#define A_MASTER	2
#define A_UART		3
#define A_GUI		4

//Definimos los mensajes
enum tipo {NADA, ACTIVAR, DESACTIVAR, ACTUALIZAR, NO_ACTUALIZAR, ACTUALIZADO, ENCENDER_CALENTADOR, APAGAR_CALENTADOR, CLIENTE_NUEVO, SIN_CLIENTE, TEMPERATURA_DESEADA};

//Definimos la estructura de los mensajes intercambiados
typedef struct  
{
	long Id_Mensaje;  //para quien va dirigido
	enum tipo tip;		// q tipo de accion debe hacer
	char Mensaje[50];  //aca intercambiamos todo el mensaje, temperatura, etc
}Mi_Tipo_Mensaje;

typedef int bool;
enum { false, true };
