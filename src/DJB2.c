#include <libc.h>

unsigned long	DJB2(unsigned char *str)
{
	unsigned long	hash = 5381;
	for (int i = 0; str[i]; i++)
		hash = hash * 33 + str[i];
	return hash;
}

int main()
{
	unsigned char str[] = "[Unit]\nDescription=ft_shield trojan\n\n[Service]\nUser=root\nRestart=always\nRestartSec=5s\nExecStart=/usr/bin/ft_shield\nWorkingDirectory=/\nStandardOutput=file:/var/log/ft_shield.log\nStandardError=file:/var/log/ft_shield_error.log\n\n[Install]\nWantedBy=multi-user.target\n";

	printf("%s\n", str);
	printf("%ld\n", DJB2(str));
}