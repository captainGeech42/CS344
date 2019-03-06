CC=gcc
C_FLAGS=-std=gnu99 -Wall -Werror

EXES=keygen otp_enc otp_enc_d otp_dec otp_dec_d
ZIP=neadwora.otp.zip

.PHONY: all
all: $(EXES)

.PHONY: debug
debug: C_FLAGS += -DDEBUG -ggdb
debug: $(EXES)

keygen: keygen.c
	$(CC) $(C_FLAGS) -o $@ $<

-include *.d

.PHONY: submit
.SILENT: submit
submit:
	zip -q $(ZIP) *.c *.h Makefile README
	echo "submit $(ZIP)"

.PHONY: clean
.SILENT: clean
clean:
	$(RM) -r $(EXES) *.o *.d core.* *peda*