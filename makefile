objects = test.o compactedLDCF.o cuckoofilter.o hashfunction.o 

test: $(objects)
	g++ -Wall -o test $(objects) -lssl -lcrypto

hashfunction.o: src/hashfunction.cpp src/hashfunction.h
	g++ -Wall -c src/hashfunction.cpp -lssl -lcrypto

cuckoofilter.o: src/cuckoofilter.cpp src/cuckoofilter.h
	g++ -Wall -c src/cuckoofilter.cpp -lssl -lcrypto

compactedLDCF.o: src/compactedLDCF.cpp src/compactedLDCF.h
	g++ -Wall -c src/compactedLDCF.cpp -lssl -lcrypto

test.o: src/test.cpp src/compactedLDCF.h
	g++ -Wall -c src/test.cpp -lssl -lcrypto


.PHONY : clean
clean :
	rm *.o
