all: build exec reloc
.PHONY: all

build:
	@mkdir -p build

clean:
	@rm -rf build minimal_relocatable.o minimal_executable.elf linked

exec: build exec.c
	@echo "Building and executing a minimal executable:"
	@gcc -g exec.c -o ./build/exec
	@./build/exec minimal_executable.elf
	@echo "Run minimal_executable.elf to test."

reloc: build reloc.c
	@echo "Building and linking a minimal relocatable:"
	@gcc -g reloc.c -o ./build/reloc
	@./build/reloc minimal_relocatable.o
	@gcc main.c -c -o ./build/main.o
	@gcc ./build/main.o minimal_relocatable.o -o linked
	@echo "Run linked to test."
