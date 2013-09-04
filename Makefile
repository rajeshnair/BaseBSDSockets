INCS = -I. -I/usr/local/include -I/usr/include -I/G/system/system -I/G/system/ztcpip
OPTS = -g -Winspect -Woptimize=0 -Wextensions -Wsystype=guardian -D__linux__ -D_XOPEN_SOURCE_EXTENDED=1
CC = c89
TARGETOBJ = nbserv

nbserv: NonBlockingServer.cpp
	$(CC) -o $@ ${INCS} ${OPTS} $?

bserv: exampled.cpp
	$(CC) -o $@ ${INCS} ${OPTS} $?

client: TestClient.cpp
	$(CC) -o $@ ${INCS} ${OPTS} $?

sel: SelectServer.cpp
	$(CC) -o $@ ${INCS} ${OPTS} $?

nwsock: NowaitGuardianSockets.cpp
	$(CC) -o $@ ${INCS} ${OPTS} $?


clean:
	rm -rf bserv bserv.dSYM/
	rm -rf client client.dSYM/
	rm -rf nbserv nbserv.dSYM/	
	rm -rf sel sel.dSYM/	
	rm -rf nwsock nwsock.dSYM/	
