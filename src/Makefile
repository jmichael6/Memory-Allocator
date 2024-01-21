BIN = ../bin
TYPES = heap-types

HEAP_TYPE = $(BIN)/footer.o

$(BIN)/a.out: main.c $(HEAP_TYPE)
	gcc -g main.c $(HEAP_TYPE)      -o $(BIN)/a.out

$(BIN)/header.o: $(TYPES)/header.c common.h heap.h
	gcc -g -c $(TYPES)/header.c     -o $(BIN)/header.o

$(BIN)/footer.o: $(TYPES)/footer.c common.h heap.h
	gcc -g -c $(TYPES)/footer.c     -o $(BIN)/footer.o

$(BIN)/free_list.o: $(TYPES)/free_list.c common.h heap.h
	gcc -g -c $(TYPES)/free_list.c  -o $(BIN)/free_list.o 

clean: 
	rm $(BIN)/*.o $(BIN)/a.out

.PHONY: clean 
