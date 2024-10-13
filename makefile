# Makefile

# Default target
all: compile

# Target to run the compile.sh script
compile:
	./compile.sh && ./test.sh


# Clean target (optional)
clean:
	rm -rf *.o my_program
