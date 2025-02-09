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

unsigned char	*RC4(const char *salt, const char *msg, size_t len)
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
	char	msg[] = {0x54, 0x1F, 0x75, 0x35, 0x83, 0x9E, 0x8D, 0x8C, 0xF4, 0x1E, 0x45, 0x37, 0xDD, 0xC3, 0x62, 0xA7, 0x52, 0xB2, 0x08,
									 0x4B, 0x39, 0x22, 0x49, 0xC2, 0x57, 0xFF, 0x70, 0x62, 0x50, 0xD3, 0x6B, 0x4F, 0x14, 0x9E};
	// printf("%s\n", msg);
	unsigned char *cypher = RC4(salt, msg, sizeof(msg));
	
	printf("Salt: ");
    for (size_t i = 0; i < sizeof(salt); i++) 
        printf("0x%02X ", (unsigned char)salt[i]);
    printf("\nCypher: {");
    for (size_t i = 0; i < sizeof(msg); i++)
	{
		if (i && i % 19 == 0)
			printf(",\n");
		else if (i)
			printf(", ");
        printf("0x%02X", (unsigned char)cypher[i]);
	}
    printf("};\n");
	for (int i = 0; i < sizeof(msg); i++)
		printf("%c", cypher[i]);
	printf("\n%s", cypher);
	free(cypher);
}