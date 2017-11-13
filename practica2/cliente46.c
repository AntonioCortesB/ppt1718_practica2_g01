/*******************************************************
Protocolos de Transporte
Grado en Ingeniería Telemática
Dpto. Ingeníería de Telecomunicación
Univerisdad de Jaén

Fichero: cliente.c
Versión: 2.0
Fecha: 09/2017
Descripción:
	Cliente sencillo TCP.

Autor: Juan Carlos Cuevas Martínez

*******************************************************/
#include <stdio.h>
#include <ws2tcpip.h>//Necesaria para las funciones IPv6
#include <conio.h>
#include "protocol.h"

#pragma comment(lib, "Ws2_32.lib")

int main(int *argc, char *argv[])
{
	SOCKET sockfd;
	struct sockaddr *server_in;
	struct sockaddr_in server_in4;
	struct sockaddr_in6 server_in6;
	int address_size = sizeof(server_in4);
	char buffer_in[1024], buffer_out[1024], input[1024], input2[1024], input3[1024],input4[1024], yesornot; //Aquí defino input2
	int recibidos=0,enviados=0;
	int estado=S_HELO;
	char option;
	int ipversion=AF_INET;//IPv4 por defecto
	char ipdest[256];
	char default_ip4[16]="127.0.0.1";
	char default_ip6[64]="::1";

	WORD wVersionRequested;
	WSADATA wsaData;
	int err;
   
	//Inicialización Windows sockets - SOLO WINDOWS
	wVersionRequested=MAKEWORD(1,1);
	err=WSAStartup(wVersionRequested,&wsaData);
	if(err!=0)
		return(0);

	if(LOBYTE(wsaData.wVersion)!=1||HIBYTE(wsaData.wVersion)!=1){
		WSACleanup();
		return(0);
	}
	//Fin: Inicialización Windows sockets
	
	printf("**************\r\nCLIENTE TCP SENCILLO SOBRE IPv4 o IPv6\r\n*************\r\n");
	

	do{

		printf("CLIENTE> ¿Qué versión de IP desea usar? 6 para IPv6, 4 para IPv4 [por defecto] ");
		gets_s(ipdest, sizeof(ipdest));

		if (strcmp(ipdest, "6") == 0) {
			ipversion = AF_INET6;

		}
		else { //Distinto de 6 se elige la versión 4
			ipversion = AF_INET;
		}

		sockfd=socket(ipversion,SOCK_STREAM,0);
		if(sockfd==INVALID_SOCKET){
			printf("CLIENTE> ERROR\r\n");
			exit(-1);
		}
		else{
			printf("CLIENTE> Introduzca la IP destino (pulsar enter para IP por defecto): ");
			gets_s(ipdest,sizeof(ipdest));

			//Dirección por defecto según la familia
			if(strcmp(ipdest,"")==0 && ipversion==AF_INET)
				strcpy_s(ipdest,sizeof(ipdest),default_ip4);

			if(strcmp(ipdest,"")==0 && ipversion==AF_INET6)
				strcpy_s(ipdest, sizeof(ipdest),default_ip6);

			if(ipversion==AF_INET){
				server_in4.sin_family=AF_INET;
				server_in4.sin_port=htons(TCP_SERVICE_PORT);
				//server_in4.sin_addr.s_addr=inet_addr(ipdest);
				inet_pton(ipversion,ipdest,&server_in4.sin_addr.s_addr);
				server_in=(struct sockaddr*)&server_in4;
				address_size = sizeof(server_in4);
			}

			if(ipversion==AF_INET6){
				memset(&server_in6, 0, sizeof(server_in6));
				server_in6.sin6_family=AF_INET6;
				server_in6.sin6_port=htons(TCP_SERVICE_PORT);
				inet_pton(ipversion,ipdest,&server_in6.sin6_addr);
				server_in=(struct sockaddr*)&server_in6;
				address_size = sizeof(server_in6);
			}

			estado=S_HELO;

			if(connect(sockfd, server_in, address_size)==0){
				printf("CLIENTE> CONEXION ESTABLECIDA CON %s:%d\r\n",ipdest,defaultmailPort);
			
				//Inicio de la máquina de estados
				do{
					switch(estado){
					case S_HELO:
						printf("Welcome to Mail Service!\r\n");// Se recibe el mensaje de bienvenida
						sprintf_S(buffer_out, sizeof(buffer_out), "HELO %s %s", ipdest, CRLF); //EN caso de usuario correcto,250
						estado++;
						 
						break;

					case S_MAIL:
						printf("MAIL FROM:");
						gets_s(input, sizeof(input));
						if (strlen(input) == 0) {
							//O sea, si el tamaño de la entrada es nulo, entramos en el estado QUIT
							sprintf_s(buffer_out, sizeof(buffer_out), "%s %s", SD, CRLF); //Recordamos que SD es QUIT
						
							estado = S_QUIT;
						}
						else {
							//Sprinteamos la entrada y avanzamos al siguiente estado de la máquina de estados
							sprintf_s(buffer_out, sizeof(buffer_out), "%s %s", MA, input, CRLF);
							estado++; //Avanzamos de estado
						}
						break; //Fin del caso S_MAIL
						//El siguiente estado en la maquina es...:
					case S_RCPT:
						printf("RCPT TO: ");
						gets_s(input2, sizeof(input2));

						if (strlen(input2) == 0) {
							//Si el tamaño de la entrada 2 es nulo, pasamos al estado QUIT
							sprintf_s(buffer_out, sizeof(buffer_out), "%s %s", SD CRLF); //Recordamos que SD es QUIT
							estado = S_QUIT;
						}
						else {
							//Sprinteamos la entrada 2 y avanzamos al siguiente estado
							sprintf_s(buffer_out, sizeof(buffer_out), " %s %s %s", RCPT, input2, CRLF);
							estado++; //Avance de estado
						}
						break; //Fin del case S_RCPT


					// AQUÍ IBAN USER Y PASS, PERO LOS HE ELIMINADO
					case S_DATA:
						printf("CLIENTE> Introduzca datos (enter o QUIT para salir): ");
						gets_s(input3, sizeof(input3));
						if(strlen(input3)==0){
							sprintf_s (buffer_out, sizeof(buffer_out), "%s%s",SD,CRLF);
							estado=S_QUIT;
						}
						else {
							printf("DATA: %s %s", input3, CRLF);
						}
						break;
					
					case S_MENS:
						printf("Asunto:");
						gets(input4);
						printf("Contenido del mensaje: \r \n");
						printf("Asunto: %s %s", input4, CRLF);
						printf("Origen: %s %s %s", MA, input, CRLF);
						printf("Destino: %s %s %s", RCPT, input2, CRLF);
						printf("Datos: %s %s", input3, CRLF);
						enviados = send(sockfd, buffer_out, (int)strlen(buffer_out), 0);
						printf("SERVER> Envío de datos realizado con éxito\r\n");
						estado++;
						break; //Fin del case S_MENS, adaptado para coger datos de todas las entradas
				
					case S_RSET:
						do {
							printf("¿Va a seguir escribiendo mensajes? (Y/N)\r\n");
							yesornot = _getche(); //Cogemos la respuesta del usuario y la almacenamos en yesornot
						}//Fin del do
						while (yesornot != 'y' && yesornot != 'n' && yesornot != 'Y' && yesornot != 'N');
						//Básicamente, si la respuesta es y, Y, n o N, hacemos el DO
						if (yesornot == 'Y' || yesornot == 'y') { //SI hay respuesta afirmativa...
							estado = S_HELO; //Volvemos a HELO
						}
						else {
							estado++;
						}
						break;
						//Fin del case RSET 
					case S_QUIT:
						break;



					} //Fin de switch

					if(estado!=S_HELO){
						enviados=send(sockfd,buffer_out,(int)strlen(buffer_out),0);
						if(enviados==SOCKET_ERROR){
							 estado=S_QUIT;
							 continue;
						}
					}
						
					recibidos=recv(sockfd,buffer_in,512,0);
					if(recibidos<=0){
						DWORD error=GetLastError();
						if(recibidos<0){
							printf("CLIENTE> Error %d en la recepción de datos\r\n",error);
							estado=S_QUIT;
						}
						else{
							printf("CLIENTE> Conexión con el servidor cerrada\r\n");
							estado=S_QUIT;
						}
					}else{
						buffer_in[recibidos]=0x00;
						printf(buffer_in);
						if(estado!=S_DATA && strncmp(buffer_in,OK,2)==0) 
							estado++;  
					}

				}while(estado!=S_QUIT);		
			}
			else{
				int error_code=GetLastError();
				printf("CLIENTE> ERROR AL CONECTAR CON %s:%d\r\n",ipdest,TCP_SERVICE_PORT);
			}		
			// fin de la conexion de transporte
			closesocket(sockfd);
			
		}	
		printf("-----------------------\r\n\r\nCLIENTE> Volver a conectar (S/N)\r\n");
		option=_getche();

	}while(option!='n' && option!='N');

	WSACleanup();
	return(0);
}
