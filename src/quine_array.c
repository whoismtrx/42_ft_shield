#include <libc.h>

unsigned char	*get_file_data(void)
{
	FILE			*file;
	long			size, rsize;
	unsigned char	*buff;

	file = fopen("/Users/orekabe/Desktop/42_ft_shield/src/main.c", "rb");
	if (!file)
		exit(1);
	fseek(file, 0, SEEK_END);
	size = ftell(file);
	rewind(file);
	buff = (unsigned char *)calloc(size + 1, sizeof(unsigned char));
	if (!buff)
	{
		fclose(file);
		exit(1);
	}
	rsize = fread(buff, 1, size, file);
	if (rsize != size)
	{
		free(buff);
		fclose(file);
		exit(1);
	}
	if (fclose(file) < 0)
	{
		free(buff);
		exit(1);
	}
	return buff;
}

void	RC4_swap(unsigned char *S, int i, int j)
{
	unsigned char	temp;

	temp = S[i];
	S[i] = S[j];
	S[j] = temp;
}

void	RC4_KSA(const char *salt, unsigned char *S)
{
	size_t	i, j, len = strlen(salt);
	for (i = 0; i < 256; i++)
		S[i] = i;
	j = 0;
	for (i = 0; i < 256; i++)
	{
		j = (j + S[i] + salt[i % len]) % 256;
		RC4_swap(S, i, j);
	}
	return;
}

void	RC4_PRGA(unsigned char *S, unsigned char *K, size_t len)
{
	size_t	i = 0, j = 0;

	for (size_t p = 0; p < len; p++)
	{
		i = (i + 1) % 256;
		j = (j + S[i]) % 256;
		RC4_swap(S, i, j);
		K[p] = S[(S[i] + S[j]) % 256];
	}
}

unsigned char	*RC4(const char *salt, const unsigned char *msg, size_t len)
{
	unsigned char	*cypher = (unsigned char *)calloc(len+1, sizeof(unsigned char));
	unsigned char	S[256];
	unsigned char	K[len];

	RC4_KSA(salt, S);
	RC4_PRGA(S, K, len);
	for (size_t i = 0; i < len; i++)
		cypher[i] = msg[i] ^ K[i];
	return cypher;
}

int main()
{
	char	salt[] = "orekabe & aabdou\n";
	unsigned char	*msg = get_file_data();
	printf("%s\n", msg);
	unsigned char *cypher = RC4(salt, msg, strlen((const char *)msg));
	
	printf("Salt: ");
    for (size_t i = 0; i < sizeof(salt); i++) 
        printf("0x%02X ", (unsigned char)salt[i]);
    printf("\nCypher: {");
    for (size_t i = 0; i < strlen((const char *)msg); i++)
	{
		if (i && i % 19 == 0)
			printf(",\n");
		else if (i)
			printf(", ");
        printf("0x%02X", (unsigned char)cypher[i]);
	}
    printf("};\n");
	for (int i = 0; i < strlen((const char *)msg); i++)
		printf("%c", cypher[i]);
	printf("\n%s", cypher);
	free(cypher);
}