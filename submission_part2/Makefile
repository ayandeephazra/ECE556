SYSTEM     = x86-64_sles10_4.1
LIBFORMAT  = static_pic

# ---------------------------------------------------------------------         
# Compiler selection                                                            
# ---------------------------------------------------------------------         

CCC = g++

# ---------------------------------------------------------------------         
# Compiler options                                                              
# ---------------------------------------------------------------------         

CCOPT = -m64 -O -fPIC -fexceptions -DNDEBUG -DIL_STD -g -Wall

# ---------------------------------------------------------------------         
# Link options and libraries                                                    
# ---------------------------------------------------------------------         

CCFLAGS = $(CCOPT) 
CCLNFLAGS = -lm -pthread 

#------------------------------------------------------------                   
#  make all      : to compile.                                     
#  make execute  : to compile and execute.                         
#------------------------------------------------------------    

ROUTE.exe: main.o ece556.o quicksort_dec.o
	/bin/rm -f ROUTE.exe
	$(CCC) $(LINKFLAGS) $(CCFLAGS) main.o ece556.o quicksort_dec.o $(CCLNFLAGS) -o ROUTE.exe

main.o: main.cpp ece556.h 
	/bin/rm -f main.o
	$(CCC) $(CCFLAGS) main.cpp -c

ece556.o: ece556.cpp ece556.h 
	/bin/rm -f ece556.o
	$(CCC) $(CCFLAGS) ece556.cpp -c

quicksort_dec.o: quicksort_dec.cpp quicksort_dec.h
	/bin/rm -f quicksort_dec.o
	$(CCC) $(CCFLAGS) quicksort_dec.cpp -c

clean:
	/bin/rm -f *~ *.o ROUTE.exe 
