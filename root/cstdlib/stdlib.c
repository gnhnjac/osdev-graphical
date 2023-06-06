unsigned int atoi(const char *str)
{

	unsigned int result = 0;
	unsigned int power = 1;

	for(unsigned int i = strlen(str)-1; i >= 0; i--)
	{

		result += (str[i]-'0')*power;

		power *= 10;

	}

	return result;

}

void itoa(int n, char *buf, int base)
{

	char digits[] = {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};

	int neg = 0;
	if (n < 0)
	{
		n = -n;
		neg = 1;
	}
	int temp_n = n;
	int n_of_digits = 0;
	do
	{

		n_of_digits++;
		temp_n /= base;

	} while (temp_n);

	if (neg)
	{
		n_of_digits += 1;
		buf[0] = '-';
	}

	uint32_t i = neg;
	i = 0;
	do
	{	

		int digit = digits[n%base];
		*(buf + n_of_digits - 1 - i) = digit;
		i++;
		n /= base;

	} while(n);

	buf[n_of_digits] = 0;

}

void uitoa(unsigned int n, char *buffer, int base)
{
	char digits[] = {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};

	uint32_t u_n = (uint32_t)n; 
	int temp_n = u_n;
	int n_of_digits = 0;
	do
	{

		n_of_digits++;
		temp_n /= base;

	} while (temp_n);

	uint32_t i = 0;
	do
	{	

		int digit = digits[u_n%base];
		*(buffer + n_of_digits - 1 - i) = digit;
		i++;
		u_n /= base;

	} while(u_n);

	buffer[n_of_digits] = 0;

}

unsigned int abs(int x)
{

	return (x > 0) ? x : -x;

}