CC = g++
CFLAGS = -std=c++11 -O3
OBJS = lib.o inst.o netlist.o placement.o cluster.o kmeans_cls.o read_input.o cls_mapping.o finetune.o output.o modifycls.o
BINS = main
TC1  = sampleCase
TC2  = testcase1_0614.txt
INFILE = ./Testcase/$(TC1)
OUTFILE = output.txt

all: $(BINS)
	./$(BINS) $(INFILE) $(OUTFILE)

%.o: %.cpp 
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.cpp %.h
	$(CC) $(CFLAGS) -c $< -o $@

$(BINS): main.cpp $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@

clean:
	rm $(BINS) $(OBJS) *.o
	