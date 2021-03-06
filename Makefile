
GCC = gcc
SRCDIR = src
FLAGS = -lm -pthread -o 
SRCS = $(wildcard $(SRCDIR)/*.c)
FLAGSDEBUG = -g
FLAGSDEBUG += $(FLAGS)

main: $(SRCS)
	$(GCC) $^ $(FLAGS) $@

debug: $(SRCS)
	$(GCC) $^ $(FLAGSDEBUG) main_d

clean:
	@ echo "Removing bin!"
	@ rm main*