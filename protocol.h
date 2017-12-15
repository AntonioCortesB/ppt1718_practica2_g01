#ifndef protocolostpte_practicas_headerfile
#define protocolostpte_practicas_headerfile
#define _WINSOCK_DEPRECATED_NO_WARNINGS //lo necesitamos para evitar errores en VisualStudio2017 y que funcione el apartado del dominio
#endif
// Línea para commit de prueba, vamos a usarla para probar un commit.
// COMANDOS DE APLICACION
#define SC "USER"  // SOLICITUD DE CONEXION USER usuario 
#define PW "PASS"  // Password del usuario  PASS password
#define SUM "SUM "  // Definimos el comando SUM (con espacio)
 //Comandos de la Práctica 2
#define HE "HELO"
//_________________________________________________________________
#define SD  "QUIT"  // Finalizacion de la conexion de aplicacion (También de la práctica 2 pero ya estaba implementado)
#define SD2 "EXIT"  // Finalizacion de la conexion de aplicacion 
#define ECHO "ECHO" // Definicion del comando "ECHO" para el servicio de eco


// RESPUESTAS A COMANDOS DE APLICACION
#define OK  "OK"
#define ER  "ER"
#define UU "554 User Unknown" //Este error lo hemos buscado en google, es el error 5xx (error de servidor) que corresponde a usuario desconocido.


//FIN DE RESPUESTA
#define CRLF "\r\n"

//ESTADOS (MODIFICADA)
#define S_HELO 0
#define S_MAIL 1
#define S_RCPT 2
//#define S_USER 1
//#define S_PASS 2          Estas dos las elimino porque no sirven
#define S_DATA 3
#define S_MENS 4
#define S_RSET 5
#define S_QUIT 6
#define S_EXIT 7

//Estos no se necesitan aún
#define S_VRFY 8
#define S_NOOP 9 // Esto asiente con el 250 (ACK?)
#define S_HELP 10

//PUERTO DEL SERVICIO
//#define TCP_SERVICE_PORT	6000 //Ya no sirve, LO COMENTO PORQUE LO COGÍA AUNQUE ESTUVIERA DEFAULTMAILPORT, SOLUCIONADO 12/12/17

// NOMBRE Y PASSWORD AUTORIZADOS
#define USER		"alumno"  //Ya no sirve
#define PASSWORD	"123456"  //Ya no sirve

//Definimos el puerto por defecto
#define defaultmailPort 25  //Lo fijamos en el puerto 25

//DEFINICION DE COMANDOS DE APLICACION: TAREA 3

//Definicion de comando HELO
//(Está arriba)
//Definición de comandos de envío de mensaje 
//Definición del comando MAILFROM
#define MA "MAIL FROM:"
//Definición de RCPTTO
#define RCPT "RCPT TO: "
//Definición de DATA
#define DATA "DATA"
//Definición de MENS (Mensajes)
#define MENS "MENS"

//REDACCIÓN DE CORREOS DE CUALQUIER LONGITUD: TAREA 4
//Definición de RSET, comando que aborta la transmisión de datos que se está
//realizando y reinicia la sesión actual.
#define RSET "RSET"