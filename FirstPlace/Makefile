TARGET = cadb0027

# Testcase
TC1  = ../Testcase/testcase1_0812.txt
TC2  = ../Testcase/testcase2_0812.txt
TC3  = ../Testcase/testcase3.txt
TC4  = ../Testcase/hiddencase01.txt
TC5  = ../Testcase/hiddencase02.txt
TC6  = ../Testcase/hiddencase03.txt
TC7  = ../Testcase/hiddencase04.txt
OUTFILE = output.txt

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

# analyze
analyze1:
	./analyzer $(TC1) $(OUTFILE)

analyze2:
	./analyzer $(TC2) $(OUTFILE)

analyze3:
	./analyzer $(TC3) $(OUTFILE)

analyze4:
	./analyzer $(TC4) $(OUTFILE)

analyze5:
	./analyzer $(TC5) $(OUTFILE)

analyze6:
	./analyzer $(TC6) $(OUTFILE)

analyze7:
	./analyzer $(TC7) $(OUTFILE)


	