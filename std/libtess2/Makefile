SRCDIR = Source
SRC = $(wildcard $(SRCDIR)/*.c)
INC = Include
OBJDIR = Build
OBJ=${patsubst $(SRCDIR)%.c,$(OBJDIR)%.o,${SRC}}
OUT = libtess2

# Fallback to gcc if clang not available
CC = clang
clang = $(shell which clang 2> /dev/null)
ifeq (, $(clang))
	CC = gcc
endif

$(OUT): $(OBJ)

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	@$(CC) -c $< -o $@ -mmacosx-version-min=11.0 -I $(INC)

the_build_folder := $(shell mkdir $(OBJDIR))

clean:
	@rm -f $(OUT) $(OBJ)
