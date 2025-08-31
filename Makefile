NAME = ft_ping

SRC = srcs/main.c \
	  srcs/utils.c \
	  srcs/icmp.c \
	  srcs/parsing.c \
	  srcs/ping.c

FLAGS = -Wall -Wextra -Werror

CC= gcc

.c.o:
	gcc $(FLAGS) -c $< -o ${<:.c=.o}

OBJ=$(SRC:.c=.o)
OBJ_LIB=$(libft/*.o)

$(NAME): $(OBJ)
	$(CC) $(OBJ) -o $(NAME) -lm

all: $(NAME)

clean:
	/bin/rm -f $(OBJ)

fclean: clean
	/bin/rm -f $(NAME)

re: fclean all

.PHONY: all bonus clean fclean re
