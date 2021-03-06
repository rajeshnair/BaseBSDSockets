#include <iostream>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <string>
#include <cstring>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <map>
#include <list>

#ifdef _GUARDIAN_TARGET
#include </G/system/system/cextdecs(FILE_CLOSE_)>
#endif

#define STAT_REQUEST_INTERVAL 1000

using namespace std;

class NonBlockingServer {
	public:
		NonBlockingServer (int port)
		:m_port(port)
		{
			requestCount =0;
			lastStatTime = time(NULL);
		}
		virtual ~NonBlockingServer ()
		{
		}

		void init()
		{
			listenfd = socket(AF_INET, SOCK_STREAM, 0);
			memset(&serv_addr, '0', sizeof(serv_addr));

			serv_addr.sin_family = AF_INET;
			serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
			serv_addr.sin_port = htons(m_port); 

			bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)); 
			int rc =setNonblocking(listenfd);
			if(rc < 0)
			{
				perror("No Nonblocking");
				exit(1);
			}
			listen(listenfd, 10); 

		}
		void start()
		{
			char sendBuff[1025];
			char recvBuff[1025];
			memset(sendBuff, '0', sizeof(sendBuff)); 
			memset(recvBuff, '0', sizeof(recvBuff)); 
			bool continueServing = true;
			do
			{
				struct sockaddr_in clientaddr;
				socklen_t addrlen;	
				printf("Going to call accept() \n ");
				int connfd = accept(listenfd, (struct sockaddr*)&clientaddr, &addrlen); 
				if(connfd < 0 && (errno != EAGAIN && errno != EWOULDBLOCK ))
				{	
					printf("Failed with return status %d, errno %d",connfd, errno);
					continue;
				}	
				else if(connfd > 0)
				{
					clientConnFdList.push_back(connfd);				
					setNonblocking(connfd);
				}
				printf("accept() returned \n");


				for (std::list<int>::iterator it = clientConnFdList.begin(); it != clientConnFdList.end(); it++)
				{
					int nrcvd = recv(*it, recvBuff, 200, 0 );
					if(nrcvd == -1 && (errno == EAGAIN || errno == EWOULDBLOCK))
						continue;
					string numStr(recvBuff,0,nrcvd);
					requestMap.insert(std::pair<int,string>(*it,numStr));
				}

				std::map<int ,string>::iterator it = requestMap.begin();
				while( it != requestMap.end())
				{
					int localConnFd = it->first;
					string requestString = it->second;
					if(requestString.compare(0,4,"STOP") ==0)
					{
#ifdef _GUARDIAN_TARGET
						FILE_CLOSE_((short)(listenfd));		
#else
						close(listenfd);					
#endif
						
												
						continueServing = false;
						return;
					}
					int resp = romanToInt(requestString);	
					snprintf(sendBuff, sizeof(sendBuff), "%d\r\n", resp);
					ssize_t wrtbytes = send(localConnFd, sendBuff, strlen(sendBuff),0); 
#ifdef _GUARDIAN_TARGET
					FILE_CLOSE_((short)localConnFd);
#else
					close(localConnFd);	
#endif					

					requestMap.erase(it++);
					clientConnFdList.remove(localConnFd);
					printStat();
				}
				

			}while(continueServing);
		}
	private:
		void printStat()
		{
			requestCount++;
			if(requestCount == STAT_REQUEST_INTERVAL )
			{
				time_t now = time(NULL);
				long diffsec = now - lastStatTime;
				float throughput = ((float)requestCount) / ((float)diffsec);
				std::cout <<"Throughput : "<<throughput <<std::endl;

				requestCount = 0;
				lastStatTime = now;
			}
		}
	
		int setNonblocking(int fd)
		{
			int flags;

			/* If they have O_NONBLOCK, use the Posix way to do it */
#if defined(O_NONBLOCK) 
/*#if 0 */
			if (-1 == (flags = fcntl(fd, F_GETFL, 0)))
				flags = 0;
			return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
#else
			/* Otherwise, use the old way of doing it */
			flags = 1;
			return ioctl(fd, FIOBIO, &flags);
#endif
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

		long requestCount;
		time_t lastStatTime;
		

		short int m_port;	
		int listenfd;
		struct sockaddr_in serv_addr; 
		list<int> clientConnFdList;	
		map<int, string> requestMap;
};


int main()
{
	NonBlockingServer server(5000);
	server.init();
	server.start();

}
