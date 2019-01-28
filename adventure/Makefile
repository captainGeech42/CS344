CC=gcc
C_FLAGS=-g -Wall -DDEBUG
ONID=neadwora

all:
	@echo "specify a target"
	@exit 1

buildrooms: $(ONID).buildrooms.c
	$(CC) $(C_FLAGS) -o $(ONID).$@ $(ONID).$@.c
	./$(ONID).$@

adventure: $(ONID).adventure.c
	$(CC) $(C_FLAGS)  -o $(ONID).$@ $(ONID).$@.c
	./$(ONID).$@

clean:
	@$(RM) -r $(ONID).buildrooms $(ONID).adventure *.rooms.*
