CC     = gcc
CFLAGS = -Wall

PARTS  = part1 part2 

all: $(PARTS)

%: %.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f $(PARTS)

# clean:
# 	rm -rf $(PARTS) *.dSYM

.PHONY: all clean