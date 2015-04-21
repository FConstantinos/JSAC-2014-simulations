CC = gcc
CCFLAGS = -O3 `pkg-config --cflags --libs glib-2.0 gsl `
CCFLAGS_EXE = -o
INCLUDE_LIBS =  -L../detalgo/ -L../stochalgo/ -L../rlnc/ -L../backpressure/ -L../backpressureACK/ -L../stability_region -L../delay -ldetalgo -lstochalgo -lrlnc -lbackpressure -lbackpressureACK -lstability_region -ldelay -Wl,-rpath=../detalgo,-rpath=../stochalgo,-rpath=../rlnc,-rpath=../backpressure,-rpath=../backpressureACK,-rpath=../stability_region,-rpath=../delay
C = sim_main.c
EXE = sim

all: $(EXE)

$(EXE):
	$(CC) $(C) $(CCFLAGS) $(INCLUDE_LIBS) $(CCFLAGS_EXE) $(EXE)
clean: 
	rm $(EXE)
