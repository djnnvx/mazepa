########################
#### VARIABLES #########
########################

NAME		 	= implant
CC			 	= clang
RM			 	= rm -rf

INCLUDES 	 	= -I./includes/
LINK_FLAG    	= -lxkbcommon


CFLAGS 			= -Wno-unused-command-line-argument -fPIC

ifeq ($(MAKECMDGOALS), debug)
	CFLAGS 			+= -g -g3 -DDebug
else
	CFLAGS 		 	+= -fPIC -Wall -Wextra -O2 -Weverything -Wno-padded -Wno-gnu-zero-variadic-macro-arguments
endif


PATTERN 	 	= .c
OBJPATTERN  	= .o
SRC_DIR 		= ./sources
OBJECT_DIR 		= ./objects

vpath %$(PATTERN) $(SRC_DIR)

MAIN		 	= main.c
SRC 		 	= opt.c utils.c keyboard.c logger.c

########################
#### COMPILATION #######
########################

COMBINED		= $(SRC) $(MAIN)
OBJ 	 		= $(patsubst %$(PATTERN), $(OBJECT_DIR)/%$(OBJPATTERN), $(COMBINED))


.PHONY: all
all: directories $(NAME) ## generate PROD binary (you might have to run make fclean before)

.PHONY: debug
debug: directories $(NAME) ## generate DEBUG binary (you might have to run make fclean before)


$(OBJECT_DIR):
	@mkdir -pv $@

directories: | $(OBJECT_DIR)

$(OBJECT_DIR)/%$(OBJPATTERN) : %$(PATTERN)
	@$(CC) -o $@ -c $< $(CFLAGS) $(PROD_FLAGS) $(INCLUDES) $(LINK_FLAG)
	@echo "Compiling $@"

$(NAME): $(OBJ)
	@$(CC) -o $(NAME) $^ $(CFLAGS) $(PROD_FLAGS) $(INCLUDES) $(LINK_FLAG)
	@echo "[*** COMPILATION SUCCESSFUL ***]"
	@strip --strip-all $(NAME)
	@echo "[*** BINARY STRIP DONE ***]"

.PHONY: clean
clean:  ## removes object files
	@$(RM) objects

.PHONY: fclean
fclean: clean  ## removes all compilation artifacts
	@$(RM) $(NAME)
	@echo "[*** CLEAN ***]"

.PHONY: re  ## cleans everything and rebuild PROD binary
re: fclean all


