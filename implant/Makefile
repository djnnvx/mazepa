########################
#### VARIABLES #########
########################

NAME		 	= implant
CC			 	= clang
RM			 	= rm -rf

INCLUDES 	 	= -I./includes/
LINK_FLAG    	= -lxkbcommon
CFLAGS 		 	= -Wall -Wextra -O2 -fPIC -Weverything
CFLAGS 			+= -Wno-padded -Wno-gnu-zero-variadic-macro-arguments -Wno-unused-command-line-argument
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

all: directories $(NAME)

$(OBJECT_DIR):
	@mkdir -p $@

directories: | $(OBJECT_DIR)

$(OBJECT_DIR)/%$(OBJPATTERN) : %$(PATTERN)
	@$(CC) -o $@ -c $< $(CFLAGS) $(INCLUDES) $(LINK_FLAG)
	@echo "Compiling $@"

$(NAME): $(OBJ)
	@$(CC) -o $(NAME) $^ $(CFLAGS) $(INCLUDES) $(LINK_FLAG)
	@echo "[*** COMPILATION SUCCESSFUL ***]"
	@strip --strip-all $(NAME)
	@echo "[*** BINARY STRIP DONE ***]"

clean:
	@$(RM) objects

fclean: clean
	@$(RM) $(NAME)
	@echo "[*** CLEAN ***]"

re: fclean all

.PHONY: all clean fclean re

