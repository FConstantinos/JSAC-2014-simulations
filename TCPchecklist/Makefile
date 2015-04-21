CC = gcc
NAME = TCPchecklist
C = $(NAME).c
OBJ = $(NAME).o
SO = lib$(NAME).so
CCFLAGS_PIC = -O3 -c -fpic `pkg-config --cflags --libs glib-2.0 `
CCFLAGS_SHARED = -O3 -shared $(OBJ)
CCFLAGS_OUT = -o $(SO) 
INCLUDE_LIBS = 

all: $(OBJ) $(SO)

$(OBJ) $(SO):
	$(CC) $(C) $(CCFLAGS_PIC) 
	$(CC) $(CCFLAGS_SHARED) $(INCLUDE_LIBS) $(CCFLAGS_OUT)

clean: 
	rm *.o *.so
