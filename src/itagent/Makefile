FLAGS= -g -Wall -lpthread -lm
TARGET= itagent
LIBS=blackboard.c libserver.c libsocket.c errlib.c sensor.c localization.c support_localization.c trajectory.c sender.c rules.c ai.c
CTAGS=ctags-exuberant -R .
BIN=../../bin

all: main.c $(LIBS)
	test -d $(BIN)||mkdir $(BIN)
	gcc -o $(BIN)/$(TARGET) main.c $(LIBS) $(FLAGS)
	$(CTAGS)

clean:
	rm -rf $(BIN)/$(TARGET)	
