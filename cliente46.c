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
#include <string.h>

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
	char inputdataempty[1024], inputtest[500];
	int recibidos = 0, enviados = 0;
	int estado = S_HELO;
	char option;
	int ipversion = AF_INET;//IPv4 por defecto
	char ipdest[256]; //Esto es la IP de destino
	char ipdestdom; //Esto será la ipdest del dominio
	char default_ip4[16] = "127.0.0.1";
	char default_ip6[64] = "::1";
	char state2 = "" ;
	int mensajenuevo;
	//Variables para el loop:
	char yesornot = "";
	char punto = "";
	char state3 = "";
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
				server_in4.sin_port = htons(defaultmailPort); //Aquí he cambiado el puerto
				//server_in4.sin_addr.s_addr=inet_addr(ipdest);
				inet_pton(ipversion, ipdest, &server_in4.sin_addr.s_addr);
				server_in = (struct sockaddr*)&server_in4;
				address_size = sizeof(server_in4);
			}

			if (ipversion == AF_INET6) {
				memset(&server_in6, 0, sizeof(server_in6));
				server_in6.sin6_family = AF_INET6;
				server_in6.sin6_port = htons(defaultmailPort); //Aquí he cambiado el puerto
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

				//Voy a implementar un bucle para salir:
				if (recibidos <= 0) {
					DWORD error = GetLastError();
					if (recibidos < 0) {
						printf("CLIENTE> Data reception error %d\r\n", error); //Así pondrá el error ocurrido con su código
						estado = S_QUIT;
					} //Cierro primer if
				}//Cierro segundo if


				//Inicio de la máquina de estados
				do {
					switch (estado) {
					case S_HELO:
						printf("Welcome to Mail Service!\r\n");// Se recibe el mensaje de bienvenida
						sprintf_s(buffer_out, sizeof(buffer_out), "%s %s %s", HE, ipdest, CRLF); //EN caso de usuario correcto,250
						//estado++;  AQUIIIIII

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
							sprintf_s(buffer_out, sizeof(buffer_out), "%s %s %s", MA, sender, CRLF);
							//estado++; //Avanzamos de estado AQUIIIIIIII
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
							//estado++; //Avance de estado AQUIIII
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
						//Vamos a checkear que el usuario quiere enviar contenido el el mensaje
						/*do {
							printf("Would you like to send data in this message? (y/n, Y/N)\r\n");
							yesornot = _getche(); //Cogemos el dato (Scanf_s no funciona, resuelto en tutorias el martes 12/12/17)
						} while (yesornot != 's' && yesornot != 'n' && yesornot != 'S' && yesornot != 'N');
						//Mientras la opción no sea correcta se repite el proceso
						if (yesornot == 'Y' || yesornot == 'y') {

							//En caso afirmativo, pasamos a meter los datos en DATA y seguimos avanzando estados*/
							sprintf_s(buffer_out, sizeof(buffer_out), "%s %s", DATA, CRLF);
							/*int error_code1 = GetLastError();
							if (error_code1 = 500 || error_code1 = 501 || error_code1 = 502 || error_code1 = 503 || error_code1 = 504 || error_code1 = 505 || error_code1 = 506 || error_code1 = 507 || error_code1 = 508 || error_code1 = 509 || error_code1 = 5010 || ) {
								estado = S_DATA;
							}*/
							//estado++;             
						//}
						/*else //En caso negativo
						{

							//Con memset copiamos esos datos y los mandamos a una entrada vacia creada en un carácter, así se borran.
							memset(inputdataempty, 0, sizeof(inputdataempty));
							sprintf_s(buffer_out, sizeof(buffer_out), "%s %s", RSET, CRLF);
							estado = S_HELO;

						}*/
						break;


					//Ahora pasamos al siguiente estado, S_MENS
					case S_MENS:
						printf("Write the message [press ENTER to QUIT]");
						gets(message, sizeof(message));

						do {
							gets(data, sizeof(data));
							strcat(message, "\r\n");
							strcat(message, data); //De esta forma con strcat juntamos el mensaje con los datos añadidos anteriormente, strcat sirve para concatenar strings
						} while (strcmp(data, ".") != 0);   //Va cogiendo datos mientras no se meta un punto (.)

						printf("Subject:");
						gets(subject);
						printf("Message content: \r \n");
						printf("Datetime: %s \r\n", salida_hora);
						printf("Subject: %s %s", subject, CRLF);
						printf("Sender: %s %s %s", MA, sender, CRLF);
						printf("Destination: %s %s %s", RCPT, receiver, CRLF);
						printf("Data: %s %s", message, CRLF);

						sprintf_s(buffer_out, sizeof(buffer_out), "Date:%s%sSubject:%s%sTo:%s%sFrom:%s%s%s%s%s",
							salida_hora, CRLF, subject, CRLF, receiver, CRLF, sender,CRLF, CRLF, message, CRLF);

					

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
							sprintf_s(buffer_out, sizeof(buffer_out), "%s %s", RSET, CRLF); //Con esto decimos al servidor que hemos hecho el reset y ahora pasaremos a HELO
							//Enviamos el mensaje
							enviados = send(sockfd, buffer_out, (int)strlen(buffer_out), 0);
							//Volvemos a HELO
							estado = S_HELO;
						}
						else {
							//En caso contrario salimos (QUIT)
							sprintf_s(buffer_out, sizeof(buffer_out), "%s %s", SD, CRLF);
							estado++;
						}
						break;
						//Fin del case RSET 

						//Pasamos al caso S_QUIT
					case S_QUIT:
						break;


					} //Fin de switch


					//PROCEDEMOS AL ENVÍO DEL MENSAJE	


					
						enviados = send(sockfd, buffer_out, (int)strlen(buffer_out), 0);
						if (enviados == SOCKET_ERROR) {
							estado = S_QUIT;
							continue;
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
						
						switch (estado) {

						case S_HELO:
						case S_MAIL:
							if (strncmp(buffer_in, "2", 1) == 0)
							{
								estado++;
							}
							break;
						case S_RCPT:
							if (strncmp(buffer_in, "2", 1) == 0)
							{ 
								printf("Do you want to enter another receiver? (s = yes, n=no\r\n");
								state2 = _getche();
						
								switch (state2) {
								case 's': 
									estado = S_RCPT;
									memset(inputdataempty, 0, sizeof(inputdataempty));
									gets_s(receiver, sizeof(receiver)); //Se vuelve a coger el rcp
									break;

								case 'n': 
									if (strncmp(buffer_in, "2", 1) == 0) {
										estado = S_RSET;
									}
								

								}

/*

								do {

									printf("ENTER '1' TO REWRITE BOTH USERS \n ENTER '2' TO REWRITE ONE USER\r\n");
									scanf("%i", &state2); //Aquí he arreglado esto 12/12/2017
														  //getchar("%i", &state2);
									switch (state2) {
									case 1:
										printf("WARNING > If you proceed to enter both user you accept to send a new message\r\n");
										estado = S_RSET;
										memset(inputdataempty, 0, sizeof(inputdataempty));
										gets_s(sender, sizeof(sender)); //Se vuelve a coger el FROM
										break;

									case 2:
										estado = S_RCPT;
										gets_s(receiver, sizeof(receiver)); //En este caso cogemos solo el receptor
										break;

									default:
										printf("VOID OPTION\n");
										break;
									}
								} while (state2 != 1 && state2 != 2);
								*/






								estado++ ;
							}
							break;  ///////////////jhhjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjj
						case S_DATA:
							if (strncmp(buffer_in, "3", 1) == 0) {
								estado++;
							}
							
							break;

						case S_MENS:
							if (strncmp(buffer_in, "2", 1) == 0) {
								printf("Do you want to send another message? (1 yes, 2 no) \r\n");
								scanf("%i", &mensajenuevo);
								if (mensajenuevo == 1) {
									estado = S_MAIL;
								}
								else if (mensajenuevo == 2) {
									estado = S_QUIT;
								}


								
							}

							break;


						}

						
						
						
						
						
						
						
						
						
						if (strncmp(buffer_in, UU, 2) == 0) {
							

							do {

								printf("ENTER '1' TO REWRITE BOTH USERS \n ENTER '2' TO REWRITE ONE USER\r\n");
								scanf("%i", &state2); //Aquí he arreglado esto 12/12/2017
								//getchar("%i", &state2);
								switch (state2) {
								case 1:
									printf("WARNING > If you proceed to enter both user you accept to send a new message\r\n");
									estado = S_RSET;  
									memset(inputdataempty, 0, sizeof(inputdataempty));
									gets_s(sender, sizeof(sender)); //Se vuelve a coger el FROM
									break;

								case 2:
									estado = S_RCPT;
									gets_s(receiver, sizeof(receiver)); //En este caso cogemos solo el receptor
									break;

								default:
									printf("VOID OPTION\n");
									break;
								}
								} while (state2 != 1 && state2 != 2);
							} 
						//En caso de desear más destinatarios se implementa a continuación:
					/*	else if (estado == S_DATA) {
							//char state3; //Declaro un tercer estado para la situación que estamos implementando
							do {
								strcpy(inputdataempty, receiver); // copiamos el contenido del destinatario y concatenamos
								strcpy(inputdataempty, " "); //idem en vacío
								printf("Would you like to enter another receiver?--> s = YES, n = NO\r\n");
								state3 = _getche(); //solucionado 12/12/17 tutorías 
								
								switch (state3) {
								case 's': 
									estado = S_RCPT;
									gets_s(inputtest, sizeof(inputtest)); //Aquí ira el destinatario nuevo
									break;

								case 'S':
									estado = S_RCPT;
									gets_s(inputtest, sizeof(inputtest)); //Aquí ira el destinatario nuevo
									break;

								case 'n': 
									estado++;
									gets_s(inputtest, sizeof(inputtest)); //Aquí irá el destinatario nuevo
									break;
								case 'N':
									estado++;
									gets_s(inputtest, sizeof(inputtest)); //Aquí irá el destinatario nuevo
									break;

								default:
									printf("WARNING> Void Option");
									break;
								}
							} while (state3 != 's' && state3 != 'n'&& state3 != 'S' && state3 != 'N');       

							//estado++;
						} */
						}

					}while (estado != S_QUIT);



				}
			else {
				int error_code = GetLastError();
				printf("CLIENTE> FAILED ATTEMPT TO CONNECT WITH %s:%d\r\n", ipdest, defaultmailPort); //Aquí he cambiado puerto
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
