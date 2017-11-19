 all: isa.cpp 
	 g++ isanew.cpp -o isanew -lrt -pthread

  clean: 
	$(RM) trace