CC = g++
CFLAGS = -g -Wall -O3
OBJS = lib.o inst.o netlist.o read_input.o placement.o cluster.o kmeans_cls.o cls_mapping.o finetune.o output.o
BINS = main
INFILE = 3.in

all: $(BINS)
	./$(BINS) $(INFILE)

%.o: %.cpp %.h
	$(CC) $(CFLAGS) -c $< -o $@

$(BINS): main.cpp $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@

run:
	./$(BINS) $(INFILE)
clean:
	rm $(BINS) $(OBJS) *.o
	