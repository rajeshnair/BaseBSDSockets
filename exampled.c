/**
* @file exampled.c
* @brief 
* @author Rajesh Nair <rajenair@paypal.com>
* @version 1.0
* @date 2013-04-15

To compile:	gcc -o exampled examped.c
To run:		./exampled
To test daemon:	ps -ef|grep exampled (or ps -aux on BSD systems)
To test log:	tail -f /tmp/exampled.log
To test signal:	kill -HUP `cat /tmp/exampled.lock`
To terminate:	kill `cat /tmp/exampled.lock`
*/

#include <stdio.h>
#include <fcntl.h>
#include <signal.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <string>
#include <map>
#include <sys/types.h>
#include <time.h> 

#ifdef _GUARDIAN_TARGET
#include </G/system/system/cextdecs(FILE_CLOSE_)>
#endif

#define RUNNING_DIR	"/tmp"
#define LOCK_FILE	"exampled.lock"
#define LOG_FILE	"exampled.log"
using namespace std;

void log_message(char *filename, char* message)
{
    FILE *logfile;
    logfile=fopen(filename,"a");
    if(!logfile) return;
    fprintf(logfile,"%s\n",message);
    fclose(logfile);
}

void signal_handler( int sig)
{
	switch(sig) {
	case SIGHUP:
		log_message(LOG_FILE,"hangup signal catched");
		break;
	case SIGTERM:
		log_message(LOG_FILE,"terminate signal catched");
		exit(0);
		break;
	}
}


int romanToInt(string s) {
    map<char, int> roman;
    roman['M'] = 1000;
    roman['D'] = 500;
    roman['C'] = 100;
    roman['L'] = 50;
    roman['X'] = 10;
    roman['V'] = 5;
    roman['I'] = 1;

    int res = 0;
    for (int i = 0; i < s.size() - 1; ++i)
    {
        if (roman[s[i]] < roman[s[i+1]])
            res -= roman[s[i]];
        else
            res += roman[s[i]];
    }
    res += roman[s[s.size()-1]];
    return res;
}


void startServer()
{
    int listenfd = 0, connfd = 0;
    struct sockaddr_in serv_addr; 

    char sendBuff[1025];
	char recvBuff[1025];
    time_t ticks; 

    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    memset(&serv_addr, '0', sizeof(serv_addr));
    memset(sendBuff, '0', sizeof(sendBuff)); 

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(5000); 

    bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)); 
    listen(listenfd, 10); 

    do
    {
        connfd = accept(listenfd, (struct sockaddr*)NULL, NULL); 
		if(connfd < 0)
			printf("Failed with return status %d, errno %d",connfd, errno);
		
		int nrcvd = recv(connfd, recvBuff, 4, 0 );
		
		string numStr(recvBuff,0,nrcvd);
		int resp = romanToInt(numStr);	
		snprintf(sendBuff, sizeof(sendBuff), "%d\r\n", resp);
        send(connfd, sendBuff, strlen(sendBuff),0); 
		
#ifdef _GUARDIAN_TARGET
		FILE_CLOSE_((short)connfd);
#else		
        close(connfd);
#endif		
    }while(true);
}

main()
{
//	daemonize();
    startServer();
}

/* EOF */
