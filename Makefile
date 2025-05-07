CC = g++
CFLAGS = -O3 -Iinc -I /usr/local/include/boost/

# Directories
SRCDIR = src
INCDIR = inc
OBJDIR = obj

# Source files and target
SRC = $(wildcard $(SRCDIR)/*.cpp)
OBJ = $(patsubst $(SRCDIR)/%.cpp, $(OBJDIR)/%.o, $(SRC))
TARGET = main

# Testcase
TC1  = ./Testcase/testcase1_0812.txt
TC2  = ./Testcase/testcase2_0812.txt
TC3  = ./Testcase/testcase3.txt
TC4  = ./Testcase/hiddencase01.txt
TC5  = ./Testcase/hiddencase02.txt
TC6  = ./Testcase/hiddencase03.txt
TC7  = ./Testcase/hiddencase04.txt
OUTFILE = output.txt


all: $(TARGET)
	./$(TARGET) $(TC2) $(OUTFILE)

# run
run1:
	./$(TARGET) $(TC1) $(OUTFILE)

run2:
	./$(TARGET) $(TC2) $(OUTFILE)

run3:
	./$(TARGET) $(TC3) $(OUTFILE)

run4:
	./$(TARGET) $(TC4) $(OUTFILE)

run5:
	./$(TARGET) $(TC5) $(OUTFILE)

run6:
	./$(TARGET) $(TC6) $(OUTFILE)

run7:
	./$(TARGET) $(TC7) $(OUTFILE)

# check
check1:
	./checker $(TC1) $(OUTFILE)

check2:
	./checker $(TC2) $(OUTFILE)

check3:
	./checker $(TC3) $(OUTFILE)

check4:
	./checker $(TC4) $(OUTFILE)

check5:
	./checker $(TC5) $(OUTFILE)

check6:
	./checker $(TC6) $(OUTFILE)

check7:
	./checker $(TC7) $(OUTFILE)
	
$(TARGET): $(OBJ) 
	$(CC) $(OBJ) -o $(TARGET)

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJDIR) $(TARGET)

	