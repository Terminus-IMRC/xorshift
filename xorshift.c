#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <inttypes.h>
#include <errno.h>

static uint8_t a, b, c, d;

void xorshift_init();
uint8_t xorshift8();

void xorshift_init()
{
	int i;

	{
		int fd;
		unsigned int seed;
		ssize_t rc;

		if ((fd = open("/dev/urandom", O_RDONLY)) == -1) {
			fprintf(stderr, "%s:%d: error: open: /dev/urandom: %s\n", __FILE__, __LINE__, strerror(errno));
			exit(EXIT_FAILURE);
		}

		for (i = 0; i < 100; i++) {
			if ((rc = read(fd, &seed, sizeof(seed))) == -1) {
				fprintf(stderr, "%s:%d: error: i=%d: read: %s\n", __FILE__, __LINE__, i, strerror(errno));
				exit(EXIT_FAILURE);
			}
			srandom(seed);
		}

		if ((rc = close(fd)) == -1) {
			fprintf(stderr, "%s:%d: error: close: %s\n", __FILE__, __LINE__, strerror(errno));
			exit(EXIT_FAILURE);
		}
	}

	a = random() % 0xff;
	b = random() % 0xff;
	c = random() % 0xff;
	d = random() % 0xff;

	for (i = 0; i < 100; i++)
		(void) xorshift8();

	return;
}

uint8_t xorshift8()
{
	uint8_t t;

	t = a ^ (a << 3);
	a = b;
	b = c;
	c = d;
	d = (d ^ (d >> 4)) ^ (t ^ (t >> 2));

	return d;
}

int main()
{
	int i;

	xorshift_init();

	for (i = 0; i < 10; i++)
		printf("%"PRIu8"\n", xorshift8());
	
	return 0;
}
