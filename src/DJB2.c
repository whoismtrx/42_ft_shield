#include <libc.h>

unsigned long	DJB2(unsigned char *str)
{
	unsigned long	hash = 5831;
	for (int i = 0; str[i]; i++)
		hash = hash * 33 + str[i];
	return hash;
}

int main()
{
	unsigned char str[] = "TooFatToKidnappMTRX";

	printf("%s\n", str);
	printf("%ld\n", DJB2(str));
}