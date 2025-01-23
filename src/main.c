#include "../include/ft_shield.h"

void	ft_error(char *str)
{
	write(2, str, strlen(str));
	exit(1);
}

bool	is_bin(void)
{
	char	path[PATH_MAX], rpath[PATH_MAX];
	ssize_t	len;
	
	len = readlink("/proc/self/exe", path, sizeof(path)-1);
	if (len == -1)
		ft_error("readlink");
	path[len] = 0;
	if (!realpath(path, rpath))
		ft_error("realpath");
	if (strcmp("/usr/bin", dirname(rpath)))
		return false;
	return true;
}

int	main()
{
	if (getegid())
		ft_error(SUDO);
	printf("%s", LOGINS);
	if (is_bin())
	{
		write(1, "I'm in /bin\n", 12);
	}
	else
	{
		write(1, "I'm not in /bin\n", 16);
	}
}