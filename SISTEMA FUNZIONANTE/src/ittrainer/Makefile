FLAGS= -g -Wall -pthread
TARGET= ittrainer
LIBS=libserver.c libsocket.c errlib.c  rules.c
CTAGS=ctags-exuberant -R .
BIN=../../bin

all: main.c $(LIBS)
	test -d $(BIN)||mkdir $(BIN)
	gcc -o $(BIN)/$(TARGET) main.c $(LIBS) $(FLAGS) 
	$(CTAGS)

clean:
	rm -rf $(BIN)/$(TARGET)
