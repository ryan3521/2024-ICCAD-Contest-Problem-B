CC = g++
CFLAGS = -std=c++11 -O3 -Iinc

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

$(TARGET): $(OBJ) 
	$(CC) $(OBJ) -o $(TARGET)

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp 
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJDIR) $(TARGET)

	