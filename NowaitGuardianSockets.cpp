/*
* AF_INET Server Stub Routine
* *****************************************************************
* * *
* * Copyright (c) Hewlett-Packard Company, 2003 *
* * *
* * The software contained on this media is proprietary to *
* * and embodies the confidential technology of Hewlett *
* * Packard Corporation. Possession, use, duplication or *
* * dissemination of the software and media is authorized only *
* * pursuant to a valid written license from Hewlett Packard *
* * Corporation. *
* * *
* * RESTRICTED RIGHTS LEGEND Use, duplication, or disclosure *
* * by the U.S. Government is subject to restrictions as set *
* * forth in Subparagraph (c)(1)(ii) of DFARS 252.227-7013, *
* * or in FAR 52.227-19, as applicable. *
* * *
* *****************************************************************
*/
/* This is the same as the IPV4 sample server, but using nowaited I/O calls */
#include <systype.h>
#include <socket.h>
#include <errno.h>
#include <in.h>
#include <netdb.h>
#include <string.h>
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <inet.h>
#include <tal.h>
#include <ctype.h>
#include <cextdecs.h>
#define SERVER_PORT 7639
#define CLIENT_PORT 7739
#define MAXBUFSIZE 4096
long tagBack;
short completedSocket;
short dcount;

short IOCheck ( long TOVal ) { 
	/* use a single AWAITIOX() check for all I/O in this pgm 
	   return value is FE; 
	   sets global tagBack & socket that completed;
	   don't care about buf addr but do want count */
	short error;
	_cc_status CC;
	completedSocket = -1;
	CC = AWAITIOX( &completedSocket,,&dcount,&tagBack,TOVal );
	/* ignoring possible _status_gt condition */
	if( _status_lt( CC ) ) {
		FILE_GETINFO_( completedSocket,&error ); 
		return error;
	}
	else return 0;
}
int main (int argc,char **argv ) {
	int s;
	char databuf[MAXBUFSIZE];
	int new_s;
	u_short port;
	struct hostent *hp;
	const char *ap;
	short fe;
	long tag = 44; /* for nowait I/O ID */
	long tag2 = 45; /* " " */ 
	long acceptWait = -1;/* how long to wait for connections */ 
	long timeout = 500; /* read t/o of 5 secs */ 

	/* Declares sockaddr_in structures. The use of this type of
	   structure implies communication using the IPv4 protocol. */
	struct sockaddr_in serveraddr;
	struct sockaddr_in clientaddr;
	int clientaddrlen;
	char response[MAXBUFSIZE] = " This is the server's response";

	/* Create an AF_INET socket. 
	   FLAGS argument does not indicate open nowait (octal 200) ,
	   but does indicate 2 outstanding I/Os max.
	   SETMODE 30 included in the call */
	if ((s = socket_nw(AF_INET, SOCK_STREAM, 0, 2, 0)) < 0) {
		perror("socket");
		exit (0);
	}

	/* Clear the server address and set up server variables. The socket
	   address is a 32-bit Internet address and a 16-bit port number on
	   which it is listening.*/
	bzero((char *) &serveraddr, sizeof(struct sockaddr_in));
	serveraddr.sin_family = AF_INET;
	/* Set the server address to the IPv4 wild card address
	   INADDR_ANY. This signifies any attached network interface on
	   the system. */
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serveraddr.sin_port = htons(SERVER_PORT);
	/* Bind the server's address to the AF_INET socket. */
	if (bind_nw(s, (struct sockaddr *)&serveraddr, sizeof(serveraddr), tag)<0){
		perror("bind");
		exit(2);
	}
	if( fe = IOCheck( -1 ) ) {
		printf( "AWAITIO error %d from bind_nw\n",fe ); 
		exit(2);
	}
	while (1) {
		/* Accept a connection on this socket. The accept call places the
		   client's address in the sockaddr_in structure named clientaddr.*/
		clientaddrlen = sizeof(clientaddr);
		printf("Before accept\n");
		if( accept_nw(s, (struct sockaddr *)&clientaddr, &clientaddrlen, tag) <0) {
			perror("accept");
			exit(3);
		}
		printf("after accept\n");
		if( fe = IOCheck(acceptWait) ) { /* initially, wait -1; 
											maybe change afterwards? */
			if( fe == 40 ) {
				printf( "Timed out after %ld secs wtg Client connect.Terminating.\n",acceptWait/100 );
				FILE_CLOSE_((short)s);
				exit(0);
			} else {
				printf( "AWAITIO error %d from accept_nw\n",fe ); 
				exit(3);
			}
		}
		/* Need a newsocket for the data transfer 
		   Resembles the earlier call */
		if ((new_s = socket_nw(AF_INET, SOCK_STREAM,0,2,0)) < 0) {
			perror ("Socket 2 create failed.");
			exit (4);
		}
		/* Make the connection */
		if ( accept_nw2(new_s, (struct sockaddr *)&clientaddr, tag2) < 0) {
			perror ("2nd Accept failed.");
			exit (5);
		} 
		if( fe = IOCheck(-1) ) { 
			printf( "AWAITIO error %d, tag %ld from 2nd accept_nw\n",fe,tagBack );
			exit(4);
		} 
		/* Receive data from the client. 
		   recv_nw() - awaitio() should be in a loop until a logical record
		   has been received. In this example, we expect the short messages
		   to be completed in a single recv_nw() */
		if( recv_nw(new_s, databuf, sizeof(databuf), 0, tag2) < 0 ) {
			if( errno == ESHUTDOWN || errno == ETIMEDOUT || errno ==
					ECONNRESET ) {
				FILE_CLOSE_((short)new_s);
				continue;
			} else {
				perror( "recv_nw error" );
				exit( 6 );
			}
		}
		if( fe = IOCheck(timeout) ) { 
			if( fe == 40 ) { /* abandon and start over */
				FILE_CLOSE_((short)new_s);
				continue;
			} else {
				printf( "AWAITIO error %d from recv_nw\n",fe ); 
				exit(6);
			}
		}
		databuf[dcount] = '\0'; /* dcount set by IOCheck */
		/* Retrieve the client name using the address in the sockaddr_in
		   structure named clientaddr. A call to gethostbyaddr expects an
		   IPv4 address as input. */
		hp = gethostbyaddr((char *)&clientaddr.sin_addr.s_addr,
				sizeof(clientaddr.sin_addr.s_addr), AF_INET);
		/* Convert the client's 32-bit IPv4 address to a dot-formatted
		   Internet address text string. A call to inet_ntoa expects an
		   IPv4 address as input. */
		ap = inet_ntoa(clientaddr.sin_addr);
		port = ntohs(clientaddr.sin_port);
		printf("Request received from");
		if (hp != NULL) printf(" %s", hp->h_name);
		if (ap != NULL) printf(" (%s)", ap);
		printf(" port %d\n\"%s\"\n", port, databuf);
		/* Send a response to the client. */
		if (send_nw2(new_s, response, (int)strlen(response), 0, tag2) < 0) {
			perror("send_nw2");
			FILE_CLOSE_((short)new_s);
			continue;
		}
		if( fe = IOCheck( -1 ) ) { 
			FILE_CLOSE_((short)new_s);
			continue;
		}
	} /* while */
}
