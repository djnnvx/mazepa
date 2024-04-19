########################
#### VARIABLES #########
########################

NAME		 	= server
CC			 	= clang
RM			 	= rm -rf

INCLUDES 	 	= -I./includes/
LINK_FLAG    	= -lxkbcommon


CFLAGS 			= -Wno-unused-command-line-argument -Wno-cast-qual -fPIC
CFLAGS 			+= -Wno-tautological-value-range-compare

ifeq ($(MAKECMDGOALS), debug)
	CFLAGS 			+= -g3 -DDEBUG -Wno-format-security
else
	CFLAGS 		 	+= -Wall -Wextra -O3 -Weverything -Wpedantic -Wl,-s
	CFLAGS 			+= -Wno-padded -Wno-gnu-zero-variadic-macro-arguments -Wno-declaration-after-statement
endif


PATTERN 	 	= .c
OBJPATTERN  	= .o
SRC_DIR 		= ./sources
OBJECT_DIR 		= ./objects

vpath %$(PATTERN) $(SRC_DIR)

MAIN		 	= main.c
SRC 		 	= net.c lexxer.c csv.c loop.c keys.c

########################
#### COMPILATION #######
########################

COMBINED		= $(SRC) $(MAIN)
OBJ 	 		= $(patsubst %$(PATTERN), $(OBJECT_DIR)/%$(OBJPATTERN), $(COMBINED))


.PHONY: all
all: directories $(NAME) ## generate PROD binary (you might have to run make fclean before)


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
	@$(RM) $(NAME)
	@echo "[*** CLEAN ***]"

.PHONY: re  ## cleans everything and rebuild PROD binary
re: fclean all


