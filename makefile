 all: isa.cpp 
	 g++ isanew.cpp -o isanew -lrt

  clean: 
	$(RM) trace