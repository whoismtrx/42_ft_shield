#include "../inc/ft_shield.h"

void    ft_error(char *str)
{
    write(2, str, strlen(str));
    exit(1);
}

int main()
{
    if (getegid())
        ft_error(SUDO);
    printf("%s", LOGINS);

}