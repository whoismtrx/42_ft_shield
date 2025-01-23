CC = cc

CFLAGS = -Wall -Wextra -Werror

NAME = ft_shield

HEADER = -I./include

SRCD = ./src

OBJD = ./objs

SRC = $(SRCD)/main.c

OBJ = $(addprefix $(OBJD)/, $(notdir $(SRC:.c=.o)))

all: $(NAME)

$(NAME): $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $@

$(OBJD)/%.o: $(SRCD)/%.c
	mkdir -p $(OBJD)
	$(CC) -c $(CFLAGS) $(HEADER) $< -o $@

clean:
	rm -rf $(OBJD)

fclean: clean
	rm -rf $(NAME)
	
re: fclean all