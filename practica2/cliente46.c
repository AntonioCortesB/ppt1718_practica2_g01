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

*********************************************************************************************************/
#include <stdio.h>
#include <ws2tcpip.h>//Necesaria para las funciones IPv6
#include <conio.h>
#include "protocol.h"
#include <time.h> //Hemos añadido la librería para la fecha y hora

#pragma comment(lib, "Ws2_32.lib") 
#pragma warning(disable : 4996)  //Desactivamos el error 4996 (visto en stackoverflow)

#define _WINSOCK_DEPRECATED_NO_WARNINGS //Esto tenemos que implementarlo para evitar fallos en Visual Studio 2017

int main(int *argc, char *argv[])
{
	SOCKET sockfd;  //Aquí estamos creando el Socket
	struct hostent *host; //Esto forma parte de la estructura host, la hemos definido para la SE5
	struct in_addr address; //Implementado también para la SE5
	struct sockaddr *server_in;
	struct sockaddr_in server_in4;
	struct sockaddr_in6 server_in6;
	int address_size = sizeof(server_in4);
	char buffer_in[1024], buffer_out[9999], sender[1024], receiver[1024], subject[1024], data[1024], message[4096]; //Aquí defino varios inputs y yesornot que servirá para saber si el usuario quiere continuar 
						  //Al buffer de salida le doy un tamaño lo mas grande que pueda, con 9999 bastará	//enviando mensajes
						// Message también es grande (4 * 1024) para que se pueda introducir bastante texto
	int recibidos = 0, enviados = 0;
	int estado = S_HELO;
	char option;
	int ipversion = AF_INET;//IPv4 por defecto
	char ipdest[256]; //Esto es la IP de destino
	char ipdestdom; //Esto será la ipdest del dominio
	char default_ip4[16] = "127.0.0.1";
	char default_ip6[64] = "::1";

	//Variables para el loop:
	char yesornot = "";
	char punto = "";
	//Fin de vloop
	//Variables para fecha y hora
	time_t hora = time(0);
	struct tm *hlocal = localtime(&hora);
	char salida_hora[128];
	strftime(salida_hora, 128, "%d/%m/%y %H:%M", hlocal); //Definimos el formato como Día/mes/Año, Horas/minutos. Strftime es String Format Time, el formato de string de tiempo
	//Fin de fechayhora
	WORD wVersionRequested;
	WSADATA wsaData;
	int err;

	//Inicialización Windows sockets - SOLO WINDOWS
	wVersionRequested = MAKEWORD(1, 1);
	err = WSAStartup(wVersionRequested, &wsaData);
	if (err != 0)
		return(0);

	if (LOBYTE(wsaData.wVersion) != 1 || HIBYTE(wsaData.wVersion) != 1) {
		WSACleanup();
		return(0);
	}
	//Fin: Inicialización Windows sockets

	printf("**************\r\nSIMPLE TCP CLIENT OVER IPv4 or IPv6\r\n*************\r\n");


	do {

		printf("CLIENTE> Which IP version would you like to use? Enter '6' for IPv6 and '4' for IPv4 [IPv4 will be set as default] ");
		gets_s(ipdest, sizeof(ipdest));

		if (strcmp(ipdest, "6") == 0) {
			ipversion = AF_INET6;                  //Si la entrada por teclado es 6, asignamos IPv6

		}
		else { //Distinto de 6 se elige la versión 4
			ipversion = AF_INET;                 //Si la entrada por teclado no es 6, asignamos IPv4
		}

		sockfd = socket(ipversion, SOCK_STREAM, 0);
		if (sockfd == INVALID_SOCKET) {
			printf("CLIENTE> ERROR\r\n");       //Si la entrada es inválida, salimos del programa
			exit(-1);
		}

		else {
			printf_s("CLIENTE> Socket has been created succesfully.");
			printf("CLIENTE> Enter destination IP (press enter for default IP): ");
			gets_s(ipdest, sizeof(ipdest));

			//Esto que viene a continuación corresponde a la SE5:
			//Se trata de hacer que el sistema interprete una dirección enlazada a un dominio:
			ipdestdom = inet_addr(ipdest);
			if (ipdestdom == INADDR_NONE) {   //Si la dirección que pone el usuario no se corresponde con un dominio existente o es inválida...
				struct hostent *host;
				host = gethostbyname(ipdest); //Compruebo si se trata de un dominio con 'gethostbyname'
				if (host != NULL) {          //Si no es nulo, es un dominio existente
					memcpy(&address, host->h_addr_list[0], 4);  //Copio a la memoria los 4 primeros bytes de información
					printf("Address %s\n", inet_ntoa(address)); //los muestro como dirección...
				}
				strcpy_s(ipdest, sizeof(ipdest), inet_ntoa(address)); //...y los copio con un stringcopy como ip destino
			}
			//Fin de la parte del dominio

			//Dirección por defecto según la familia
			//Si se trata de IPv4...
			if (strcmp(ipdest, "") == 0 && ipversion == AF_INET)
				strcpy_s(ipdest, sizeof(ipdest), default_ip4);
			//Si se trata de IPv6...
			if (strcmp(ipdest, "") == 0 && ipversion == AF_INET6)
				strcpy_s(ipdest, sizeof(ipdest), default_ip6);

			if (ipversion == AF_INET) {
				server_in4.sin_family = AF_INET;
				server_in4.sin_port = htons(TCP_SERVICE_PORT);
				//server_in4.sin_addr.s_addr=inet_addr(ipdest);
				inet_pton(ipversion, ipdest, &server_in4.sin_addr.s_addr);
				server_in = (struct sockaddr*)&server_in4;
				address_size = sizeof(server_in4);
			}

			if (ipversion == AF_INET6) {
				memset(&server_in6, 0, sizeof(server_in6));
				server_in6.sin6_family = AF_INET6;
				server_in6.sin6_port = htons(TCP_SERVICE_PORT);
				inet_pton(ipversion, ipdest, &server_in6.sin6_addr);
				server_in = (struct sockaddr*)&server_in6;
				address_size = sizeof(server_in6);
			}

			estado = S_HELO;
			//Hacemos un checkeo para ver que la conexión se ha estableciddo correctamente
			if (connect(sockfd, server_in, address_size) == 0) {
				printf("CLIENTE> CONNECTION SUCESFULLY ESTABLISHED WITH %s:%d\r\n", ipdest, defaultmailPort);
				recibidos = recv(sockfd, buffer_in, 512, 0); //Esta línea la hemos añadido, ya que sin ella permanecía dando el HELO constantemente, con ella el servidor nos envía
															//confirmación de que ya puede transmitir.


				//Inicio de la máquina de estados
				do {
					switch (estado) {
					case S_HELO:
						printf("Welcome to Mail Service!\r\n");// Se recibe el mensaje de bienvenida
						sprintf_s(buffer_out, sizeof(buffer_out), "HELO %s %s", ipdest, CRLF); //EN caso de usuario correcto,250
						estado++;

						break;
						//Siguiente estado: S_MAIL
					case S_MAIL:
						printf("MAIL FROM:");
						gets_s(sender, sizeof(sender));
						if (strlen(sender) == 0) {
							//O sea, si el tamaño de la entrada es nulo, entramos en el estado QUIT
							sprintf_s(buffer_out, sizeof(buffer_out), "%s %s", SD, CRLF); //Recordamos que SD es QUIT

							estado = S_QUIT;
						}
						else {
							//Sprinteamos la entrada y avanzamos al siguiente estado de la máquina de estados
							sprintf_s(buffer_out, sizeof(buffer_out), "%s %s", MA, sender, CRLF);
							estado++; //Avanzamos de estado
						}
						break; //Fin del caso S_MAIL


						//El siguiente estado es S_RCPT:
					case S_RCPT:
						printf("RCPT TO: ");
						gets_s(receiver, sizeof(receiver));

						if (strlen(receiver) == 0) {
							//Si el tamaño de la entrada 2 es nulo, pasamos al estado QUIT
							sprintf_s(buffer_out, sizeof(buffer_out), "%s %s", SD CRLF); //Recordamos que SD es QUIT
							estado = S_QUIT;
						}
						else {
							//Sprinteamos la entrada 2 y avanzamos al siguiente estado
							sprintf_s(buffer_out, sizeof(buffer_out), " %s %s %s", RCPT, receiver, CRLF);
							estado++; //Avance de estado
						}
						break; //Fin del case S_RCPT


					// AQUÍ IBAN USER Y PASS, PERO LOS HE ELIMINADO

						//El siguiente estado es el caso S_DATA:

					case S_DATA:

						/*printf("CLIENTE> Introduzca datos (enter o QUIT para salir): ");
						gets_s(input3, sizeof(input3));
						if(strlen(input3)==0){
							sprintf_s (buffer_out, sizeof(buffer_out), "%s%s",SD,CRLF);             //Toda esta parte comentada queda inservible.
							estado=S_QUIT;
						}
						else {
						printf("DATA: %s %s", input3, CRLF);
					}*/
					//Enviamos al servidor que estamos en el caso S_DATA y avanzamos al siguiente estado.
						sprintf_s(buffer_out, sizeof(buffer_out), "%s%s", DATA, CRLF);
						estado++;
						break;
						//Fin de S_DATA


					//Ahora pasamos al siguiente estado, S_MENS
					case S_MENS:
						printf("Write the message [press ENTER to QUIT]");
						gets(message, sizeof(message));

						do {
							gets(data, sizeof(data));
						} while (strcmp(data, ".") != 0);   //Va cogiendo datos mientras no se meta un punto (.)

						printf("Subject:");
						gets(subject);
						printf("Message content: \r \n");
						printf("Datetime: %s \r\n", salida_hora);
						printf("Subject: %s %s", subject, CRLF);
						printf("Sender: %s %s %s", MA, sender, CRLF);
						printf("Destination: %s %s %s", RCPT, receiver, CRLF);
						printf("Data: %s %s", message, CRLF);

						sprintf_s(buffer_out, sizeof(buffer_out), "Date:%s%s From:%s%s To:%s%s Subject:%s%s Data:%s%s%s%s",
							salida_hora, CRLF, sender, CRLF, receiver, CRLF, subject, CRLF, message, CRLF, data, CRLF);

						estado++;

						printf("SERVER> YOUR DATA HAVE BEEN SUCESFULLY SENT!\r\n");
						break; //Fin del case S_MENS, adaptado para coger datos de todas las entradas


						//Ahora pasamos al caso RESET 
					case S_RSET:
						do {
							printf("WILL YOU CONTINUE TEXTING? (Y/N)\r\n");
							yesornot = _getche(); //Cogemos la respuesta del usuario y la almacenamos en yesornot
						}//Fin del do
						while (yesornot != 'y' && yesornot != 'n' && yesornot != 'Y' && yesornot != 'N');
						//Básicamente, si la respuesta es y, Y, n o N, hacemos el DO
						if (yesornot == 'Y' || yesornot == 'y') { //SI hay respuesta afirmativa...
							sprintf_s(buffer_out, sizeof(buffer_out), "%s%s", RSET, CRLF); //Con esto decimos al servidor que hemos hecho el reset y ahora pasaremos a HELO
							//Enviamos el mensaje
							enviados = send(sockfd, buffer_out, (int)strlen(buffer_out), 0);
							//Volvemos a HELO
							estado = S_HELO;
						}
						else {
							//En caso contrario salimos (QUIT)
							sprintf_s(buffer_out, sizeof(buffer_out), "%s%s", SD, CRLF);
							estado++;
						}
						break;
						//Fin del case RSET 

						//Pasamos al caso S_QUIT
					case S_QUIT:
						break;


					} //Fin de switch


					//PROCEDEMOS AL ENVÍO DEL MENSAJE	

					if (estado != S_HELO) {
						enviados = send(sockfd, buffer_out, (int)strlen(buffer_out), 0);
						if (enviados == SOCKET_ERROR) {
							estado = S_QUIT;
							continue;
						}
					}

					//PROCEDEMOS A LA RECEPCIÓN DEL MENSAJE

					recibidos = recv(sockfd, buffer_in, 512, 0);
					if (recibidos <= 0) {
						DWORD error = GetLastError();
						if (recibidos < 0) {
							printf("CLIENTE> ERROR %d IN DATA RECEPTION\r\n", error);
							estado = S_QUIT;
						}
						else {
							printf("CLIENTE> CONNECTION WITH SERVER CLOSED\r\n");
							estado = S_QUIT;
						}

					}
					else {

						buffer_in[recibidos] = 0x00;
						printf(buffer_in);
						if (estado != S_DATA && strncmp(buffer_in, UU, 2) == 0) {
							int state2 = 0;

							do {

								printf("ENTER '1' TO REWRITE BOTH USERS \n ENTER '2' TO REWRITE ONE USER");
								scanf_("%i", &state2);
								switch (state2) {
								case 1:
									estado = S_MAIL;
									gets_s(sender, sizeof(sender));
									break;

								case 2:
									estado = S_RCPT;
									gets_s(receiver, sizeof(receiver));
									break;

								default:
									printf("VOID OPTION\n");
									break;
								}
								} while (state2 != 1 && state2 != 2);
							}
						}

					}while (estado != S_QUIT);



				}
			else {
				int error_code = GetLastError();
				printf("CLIENTE> FAILED ATTEMPT TO CONNECT WITH %s:%d\r\n", ipdest, TCP_SERVICE_PORT);
			}
			// fin de la conexion de transporte
			closesocket(sockfd);

			}
			printf("-----------------------\r\n\r\nCLIENTE> RE-TRY TO CONNECT? (Y/N)\r\n");
			option = _getche();

		}while (option != 'n' && option != 'N');

		WSACleanup();
		return(0);

	}
