#include "../include/ft_shield.h"

void	ft_error(char *str)
{
	write(2, str, strlen(str));
	exit(1);
}

void	handle_signal(int sig)
{
	if (sig == SIGTERM)
		exit(0);
}

void	setup_signals(void)
{
	struct sigaction sa;
	sa.sa_handler = handle_signal;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	if (sigaction(SIGTERM, &sa, NULL) < 0)
		ft_error("sigaction");
	return;
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

void	create_daemon(void)
{
	int		fd;
	pid_t	pid = fork();

	if (pid < 0)
		ft_error("fork");
	if (pid)
		exit(0);
	if (setsid() < 0)
		ft_error("setsid");
	if (chdir("/") < 0)
		ft_error("chdir");
	umask(0);
	fd = open("/dev/null", O_RDWR);
	if (fd < 0)
		ft_error("open");
	if (dup2(fd, STDIN_FILENO) < 0)
		ft_error("dup stdin");
	if (dup2(fd, STDOUT_FILENO) < 0)
		ft_error("dup stdout");
	if (dup2(fd, STDERR_FILENO) < 0)
		ft_error("dup stderr");
	if (close(fd) < 0)
		ft_error("close fd");
	return ;
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
		create_daemon();
		setup_signals();
	}
}