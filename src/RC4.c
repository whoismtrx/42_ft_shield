#include <libc.h>

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

unsigned char	*RC4(const char *salt, const char *msg)
{
	size_t			len = strlen(msg);
	unsigned char	*cypher = (unsigned char *)malloc(len * sizeof(unsigned char *));
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
	char	msg[] = "%s";

	printf("%s\n", msg);
	unsigned char *cypher = RC4(salt, msg);
	
	printf("Salt: ");
    for (size_t i = 0; i < strlen(salt); i++) 
        printf("0x%02X ", (unsigned char)salt[i]);
    printf("\nCypher: {");
    for (size_t i = 0; i < strlen(msg); i++)
	{
		if (i && i % 19 == 0)
			printf(",\n");
		else if (i)
			printf(", ");
        printf("0x%02X", (unsigned char)cypher[i]);
	}
    printf("};\n");
	free(cypher);
}