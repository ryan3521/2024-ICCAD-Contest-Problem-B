CC = g++
CFLAGS = -std=c++11 -O3
OBJS = lib.o inst.o netlist.o placement.o read_input.o output.o banking.o costeva.o draw.o
BINS = main
TC1  = sampleCase
TC2  = testcase1_0614.txt
TC3  = testcase1_0812.txt
TC4  = testcase2_0812.txt
INFILE = ./Testcase/$(TC4)
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
	