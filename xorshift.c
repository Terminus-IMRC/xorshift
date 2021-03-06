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

static uint32_t a[16];

void xorshift_init();
uint8_t xorshift32();

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

		if ((rc = read(fd, &seed, sizeof(seed))) == -1) {
			fprintf(stderr, "%s:%d: error: read: %s\n", __FILE__, __LINE__, strerror(errno));
			exit(EXIT_FAILURE);
		}
		srandom(seed);

		if ((rc = close(fd)) == -1) {
			fprintf(stderr, "%s:%d: error: close: %s\n", __FILE__, __LINE__, strerror(errno));
			exit(EXIT_FAILURE);
		}
	}

	for (i = 0; i < 16; i++)
		a[i] = random();

	for (i = 0; i < 100; i++)
		(void) xorshift32();

	return;
}

uint8_t xorshift32()
{
	int i;
	uint32_t t;

	t = a[0] ^ (a[0] << 5);
	for (i = 0; i < 16 - 1; i++)
		a[i] = a[i + 1];
	a[16 - 1] = (a[16 - 2] ^ (a[16 - 2] >> 4)) ^ (t ^ (t >> 3));

	return a[16 - 1];
}

int main()
{
	int i, j;
	uint32_t r;
	struct timeval start, end;

	xorshift_init();

	gettimeofday(&start, NULL);
	for (i = 0; i < M; i++)
		for (j = 0; j < N; j++)
			r = (r + xorshift32()) / 2;
	gettimeofday(&end, NULL);

	printf("%f\n", (end.tv_sec + end.tv_usec * 1e-6) - (start.tv_sec + start.tv_usec * 1e-6));

	{
		int fd;
		ssize_t rc;

		if ((fd = open("/dev/null", O_WRONLY)) == -1) {
			fprintf(stderr, "%s:%d: error: open: /dev/null: %s\n", __FILE__, __LINE__, strerror(errno));
			exit(EXIT_FAILURE);
		}

		if ((rc = write(fd, &r, sizeof(r))) == -1) {
			fprintf(stderr, "%s:%d: error: write: %s\n", __FILE__, __LINE__, strerror(errno));
			exit(EXIT_FAILURE);
		}

		if ((rc = close(fd)) == -1) {
			fprintf(stderr, "%s:%d: error: close: %s\n", __FILE__, __LINE__, strerror(errno));
			exit(EXIT_FAILURE);
		}
	}

	return 0;
}
