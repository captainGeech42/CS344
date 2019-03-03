CC=gcc
C_FLAGS=-ggdb -std=gnu99 -Wall -Werror

EXE=smallsh
ZIP=neadwora.smallsh.zip

SRC_FILES=$(wildcard *.c)
OBJ_FILES=$(patsubst %.c, %.o, $(SRC_FILES))

.PHONY: all
all: $(EXE)

.PHONY: debug
debug: C_FLAGS += -DDEBUG
debug: $(EXE) run;

.PHONY: run
run: $(EXE)
	./$(EXE)

$(EXE): $(OBJ_FILES)
	$(CC) $(C_FLAGS) -o $@ $^

%.o: %.c
	$(CC) $(C_FLAGS) -c -MMD -o $@ $<

-include *.d

.PHONY: submit
.SILENT: submit
submit:
	zip -q $(ZIP) *.c *.h Makefile README
	echo "submit $(ZIP)"

.PHONY: clean
.SILENT: clean
clean:
	$(RM) -r $(EXE) *.o *.d core.* *peda*
