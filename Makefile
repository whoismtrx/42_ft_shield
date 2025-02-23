CC = cc

CFLAGS = -fsanitize=address,undefined -Wall -Wextra -Werror

NAME = ft_shield

OBJD = ./objs

SRC = main.c

OBJ = $(addprefix $(OBJD)/, $(notdir $(SRC:.c=.o)))

all: $(NAME)

$(NAME): $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $@
	strip -s $(NAME)

$(OBJD)/%.o: %.c
	mkdir -p $(OBJD)
	$(CC) -c $(CFLAGS) $(HEADER) $< -o $@

clean:
	rm -rf $(OBJD)

fclean: clean
	rm -rf $(NAME)
	
re: fclean all

up:
	@./setup.sh

ssh:
	vagrant ssh

stop:
	vagrant halt

destroy:
	vagrant destroy -f

status:
	vagrant status

reload:
	vagrant reload --provision