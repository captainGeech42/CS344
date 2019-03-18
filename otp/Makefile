CC=gcc
C_FLAGS=-std=gnu99 -Wall -Werror -MMD
DEBUG_FLAGS=-DDEBUG -ggdb

EXES=keygen otp_enc otp_enc_d otp_dec otp_dec_d
ZIP=neadwora.otp.zip

.PHONY: all
all: $(EXES)

.SILENT: debug
.PHONY: debug
debug: 
	$(eval C_FLAGS += $(DEBUG_FLAGS))
	echo > /dev/null 
	# ^^ suppress "make: nothing to be done" warning

keygen: keygen.c keygen.h
	$(CC) $(C_FLAGS) -o $@ $<

otp_enc_d: otp_enc_d.c otp_enc_d.h constants.h
	$(CC) $(C_FLAGS) -o $@ $<

otp_enc: otp_enc.c otp_enc.h constants.h
	$(CC) $(C_FLAGS) -o $@ $<

otp_dec_d: otp_dec_d.c otp_dec_d.h constants.h
	$(CC) $(C_FLAGS) -o $@ $<

otp_dec: otp_dec.c otp_dec.h constants.h
	$(CC) $(C_FLAGS) -o $@ $<

-include *.d

.PHONY: submit
.SILENT: submit
submit:
	zip -q $(ZIP) *.c *.h Makefile compileall
	echo "submit $(ZIP)"

.PHONY: clean
.SILENT: clean
clean:
	$(RM) -r $(EXES) *.o *.d core.* *peda*