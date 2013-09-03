ifeq ($(OS),Windows_NT)
    CCFLAGS += -D WIN32
    ifeq ($(PROCESSOR_ARCHITECTURE),AMD64)
        CCFLAGS += -D AMD64
    endif
    ifeq ($(PROCESSOR_ARCHITECTURE),x86)
        CCFLAGS += -D IA32
    endif
else
    UNAME_S := $(shell uname -s)
    ifeq ($(UNAME_S),Linux)
        INCS = -I/usr/local/include -I/usr/include
		OPTS = -g -D__linux__ 
		CC = g++
		TARGETOBJ = nbserv
    endif
    ifeq ($(UNAME_S),Darwin)
        INCS = -I/usr/local/include -I/usr/include
		OPTS = -g -D__linux__ 
		CC = g++
		TARGETOBJ = nbserv
    endif
    ifeq ($(UNAME_S),NONSTOP_KERNEL)
        INCS = -I/usr/local/oss/include -I/usr/local/include -I/usr/include
		OPTS = -g -Winspect -Woptimize=0 -Wextensions -Wsystype=guardian -D__linux__ -D_XOPEN_SOURCE_EXTENDED=1
		CC = c89
		TARGETOBJ = nbserv
    endif
    UNAME_P := $(shell uname -p)
    ifeq ($(UNAME_P),x86_64)
        CCFLAGS += -D AMD64
    endif
    ifneq ($(filter %86,$(UNAME_P)),)
        CCFLAGS += -D IA32
    endif
    ifneq ($(filter arm%,$(UNAME_P)),)
        CCFLAGS += -D ARM
    endif
endif

nbserv: NonBlockingServer.cpp
	$(CC) -o $@ ${INCS} ${OPTS} $?

bserv: exampled.cpp
	$(CC) -o $@ ${INCS} ${OPTS} $?

client: TestClient.cpp
	$(CC) -o $@ ${INCS} ${OPTS} $?

sel: SelectServer.cpp
	$(CC) -o $@ ${INCS} ${OPTS} $?


clean:
	rm -rf bserv bserv.dSYM/
	rm -rf client client.dSYM/
	rm -rf nbserv nbserv.dSYM/	
