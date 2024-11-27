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
OUTFILE = output.txt

all: $(TARGET)
	./$(TARGET) $(TC1) $(OUTFILE)

$(TARGET): $(OBJ) 
	$(CC) $(OBJ) -o $(TARGET)

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp 
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJDIR) $(TARGET)

	