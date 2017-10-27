/*******************************************************
Protocolos de Transporte
Grado en Ingenier�a Telem�tica                                              //AUTORES: ANTONIO CORT�S BARRAG�N Y DANIEL MESA GONZ�LEZ
Dpto. Ingen�er�a de Telecomunicaci�n
Univerisdad de Ja�n

Fichero: cliente.c
Versi�n: 1.0
Fecha: 09/2017
Descripci�n:
	Cliente sencillo TCP.

Autor: Juan Carlos Cuevas Mart�nez
//ESTO ES UNA L�NEA DE C�DIGO PARA COMMIT DE PRueba

*******************************************************/
#include <stdio.h>
#include <winsock.h>
#include <time.h>
#include <conio.h>
#include "protocol.h"

int main(int *argc, char *argv[])
{
	SOCKET sockfd; 
	struct sockaddr_in server_in;
	char buffer_in[1024], buffer_out[1024],input[1024],input2[1024]; //Aqu� he metido input2
	int recibidos=0,enviados=0;
	int estado=S_HELO; //++ 0
	char option;

	//Esto siguiente es nuevo
	int NUM1;
	int NUM2;
	int suma = 0; //fin de lo nuevo

	WORD wVersionRequested;
	WSADATA wsaData;
	int err;

    char ipdest[16];
	char default_ip[16]="127.0.0.1";
	
	//Inicializaci�n Windows sockets - SOLO WINDOWS
	wVersionRequested=MAKEWORD(1,1);
	err=WSAStartup(wVersionRequested,&wsaData);
	if(err!=0)
		return(0);

	if(LOBYTE(wsaData.wVersion)!=1||HIBYTE(wsaData.wVersion)!=1){
		WSACleanup();
		return(0);
	}
	//Fin: Inicializaci�n Windows sockets
	printf("***********************\r\nCLIENTE TCP SENCILLO\r\n***********************\r\n");
	do{
		sockfd=socket(AF_INET,SOCK_STREAM,0); //++ Creamos el descriptor del socket "sockfd"
		if(sockfd==INVALID_SOCKET){
			printf("CLIENTE> ERROR\r\n");
			exit(-1);
		}
		else{
			printf("CLIENTE> Introduzca la IP destino (pulsar enter para IP por defecto): ");
			gets(ipdest);

			if(strcmp(ipdest,"")==0)
				strcpy_s(ipdest,sizeof(ipdest),default_ip); //originalmente est� como: strcpy(ipdest, default_ip), pero en la versi�n de 2017 hay que copiar tambi�n el tama�o de la ipdest y _s

			server_in.sin_family=AF_INET;
			server_in.sin_port=htons(TCP_SERVICE_PORT);
			server_in.sin_addr.s_addr=inet_addr(ipdest);
			
			estado=S_HELO; //++ 0

			if(connect(sockfd,(struct sockaddr*)&server_in,sizeof(server_in))==0){ //++ Iniciamos conexi�n con conector remoto
				printf("CLIENTE> CONEXION ESTABLECIDA CON %s:%d\r\n",ipdest,TCP_SERVICE_PORT);
			
				//Inicio de la m�quina de estados
				do{
					switch(estado){
					case S_HELO: //++ caso 0
						// Se recibe el mensaje de bienvenida
						break;
					case S_USER: //++ caso 1
						// establece la conexion de aplicacion 
						printf("CLIENTE> Introduzca el usuario (enter para salir): ");
						gets(input);
						if(strlen(input)==0){
							sprintf_s (buffer_out, sizeof(buffer_out), "%s%s",SD,CRLF); //++ SD (quit, salir), CRLF (Clear y salto de l�nea)
							estado=S_QUIT; //++ 4 salir
						}
						else

						sprintf_s (buffer_out, sizeof(buffer_out), "%s %s%s",SC,input,CRLF); //++ SD solicita conexi�n del usuario, CRLF (Clear y salto de l�nea)
						break;
					case S_PASS: //++ 2
						printf("CLIENTE> Introduzca la clave (enter para salir): ");
						gets(input);
						if(strlen(input)==0){
							sprintf_s (buffer_out, sizeof(buffer_out), "%s%s",SD,CRLF); //++ SD solicita conexi�n del usuario, CRLF (Clear y salto de l�nea)
							estado=S_QUIT;
						}
						else
							sprintf_s (buffer_out, sizeof(buffer_out), "%s %s%s",PW,input,CRLF); //++ Password del usuario y CRLF (Clear y salto de l�nea)
						break;

					case S_DATA: //++ 3

						// METEMOS EL VALOR 1
						printf("CLIENTE> Introduzca valor 1 (Distinto de cero) (enter o QUIT para salir): "); //Aqu� he metido lo de la suma
						gets(input);
						printf("CLIENTE> Introduzca valor 2 (Distinto de cero) (enter o QUIT para salir): ");
						gets(input2);
						if (strlen(input) == 0) { //Si el tama�o de lo que se introduce es 0 caracteres...
							sprintf_s(buffer_out, sizeof(buffer_out), "%s%s", SD, CRLF); //++ SD (quit, salir), CRLF (Clear y salto de l�nea)
							// Se muestra el buffer de salida y el tama�o del buffer
							estado = S_QUIT;
						}
						else // Si el tama�o de lo que se introduce no es 0...
							//sscanf_s("%i", &valor1);
							sprintf_s(buffer_out, sizeof(buffer_out), "%s %s %s %s", SUM, input, input2, CRLF);
						//sprintf_s (buffer_out, sizeof(buffer_out), "%s %s%s",ECHO,input,CRLF); //++ Definicion del comando "ECHO" para el servicio de eco y CRLF (Clear y salto de l�nea)
						break;

					


				       // PARA LA SESH 2 NOS CENTRAREMOS AQU�, QUE PIDE UN DATO Y NO HACE NADA CON �L
					}

					if(estado!=S_HELO){ //++ 0
						enviados=send(sockfd,buffer_out,(int)strlen(buffer_out),0); //++ Se env�a un mensaje (buffer de salida)
						if(enviados==SOCKET_ERROR){
							 estado=S_QUIT; //++ 4
							 continue;
						}
					}
						
					recibidos=recv(sockfd,buffer_in,512,0); //++ Se recibe un mensaje (buffer de entrada)
					if(recibidos<=0){
						DWORD error=GetLastError();
						if(recibidos<0){
							printf("CLIENTE> Error %d en la recepci�n de datos\r\n",error);
							estado=S_QUIT; //++ 4
						}
						else{
							printf("CLIENTE> Conexi�n con el servidor cerrada\r\n");
							estado=S_QUIT; //++ 4
						}
					}else{
						buffer_in[recibidos]=0x00;
						printf(buffer_in);

						//ESTO LO HE INTRODUCIDO PARA CONTROLAR LOS CASOS (METER CONTRASE�A ERR�NEA Y DEM�S)
						switch (estado) {
						case S_USER:
							if (strncmp(buffer_in, OK, 2) == 0)
								estado++;
							else {
								estado = S_USER;
							}
							break;

						case S_PASS:
							if (strncmp(buffer_in, OK, 2) == 0)
								estado++;
							else {
								estado = S_USER;
							}
							break;

						case S_DATA:
							break;
						default: //Este default lo pongo para que para cualquier estado (helo o quit) siempre avance al siguiente estado
							if (strncmp(buffer_in, OK, 2) == 0)
								estado++;
						}
						
						//if(estado!=S_DATA && strncmp(buffer_in,OK,2)==0) //++ 3 y "OK"
							//estado++;  ESTE ERA EL CODIGO QUE HAB�A
					}

				}while(estado!=S_QUIT);		//++ 4
			}
			else{
				printf("CLIENTE> ERROR AL CONECTAR CON %s:%d\r\n",ipdest,TCP_SERVICE_PORT); //++ El puerto de servicio ser� el 6000 por defecto como est� declarado en el protocol.h
			}		
			// fin de la conexion de transporte
			closesocket(sockfd); //++ Cerramos el socket "sockfd"
			
		}	
		printf("-----------------------\r\n\r\nCLIENTE> Volver a conectar (S/N)\r\n");
		option=_getche();

	}while(option!='n' && option!='N');

	return(0);
}
