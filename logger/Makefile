########################
#### VARIABLES #########
########################

NAME		 	= k3yl0gg3r
CC			 	= gcc
RM			 	= rm -rf

INCLUDES 	 	= -I./include/
LINK_FLAG    	=
CFLAGS 		 	= -Wall -Wextra -O2 -static -s -fPIC
PATTERN 	 	= .c
OBJPATTERN  	= .o
SRC_DIR 		= ./sources
OBJECT_DIR 		= ./objects

vpath %$(PATTERN) $(SRC_DIR)

MAIN		 	= main.c
SRC 		 	= get_keyboard.c socket.c logger.c random_functions.c

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
