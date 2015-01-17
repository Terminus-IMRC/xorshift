#include <x86intrin.h>
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

static __m512i u;
static __m512i idx_rotatel;
static __mmask16 mask_least;

void xorshift_knc32_init();
uint32_t xorshift_knc32();

void xorshift_knc32_init()
{
	int i;
	uint32_t rns[16] __attribute__((aligned(64)));

	{
		int fd;
		unsigned int seed;
		ssize_t rc;

		if ((fd = open("/dev/urandom", O_RDONLY)) == -1) {
			fprintf(stderr, "%s:%d: error: open: /dev/urandom: %s\n", __FILE__, __LINE__, strerror(errno));
			exit(EXIT_FAILURE);
		}

		if ((rc = read(fd, &seed, sizeof(seed))) == -1) {
			fprintf(stderr, "%s:%d: error: i=%d: read: %s\n", __FILE__, __LINE__, i, strerror(errno));
			exit(EXIT_FAILURE);
		}
		srandom(seed);

		if ((rc = close(fd)) == -1) {
			fprintf(stderr, "%s:%d: error: close: %s\n", __FILE__, __LINE__, strerror(errno));
			exit(EXIT_FAILURE);
		}
	}

	for (i = 0; i < 16; i++)
		rns[i] = random();
	u = _mm512_extload_epi32(rns, _MM_UPCONV_EPI32_NONE, _MM_BROADCAST32_NONE, _MM_HINT_NONE);

	rns[0] = 16 - 1;
	for (i = 1; i < 16; i++)
		rns[i] = i - 1;
	idx_rotatel = _mm512_extload_epi32(rns, _MM_UPCONV_EPI32_NONE, _MM_BROADCAST32_NONE, _MM_HINT_NONE);

	mask_least = _mm512_int2mask(0x01);

	for (i = 0; i < 16*10; i++)
		(void) xorshift_knc32();

	return;
}

uint32_t xorshift_knc32()
{
	__m512i t, t2;
	uint32_t v __attribute__((aligned(64)));

	t2 = _mm512_mask_srli_epi32(u, mask_least, u, 4);
	t2 = _mm512_mask_xor_epi32(u, mask_least, u, t2);
	u = _mm512_permutevar_epi32(idx_rotatel, u);
	t = _mm512_mask_slli_epi32(u, mask_least, u, 5);
	u = _mm512_mask_xor_epi32(u, mask_least, u, t);
	t = _mm512_mask_srli_epi32(u, mask_least, u, 3);
	u = _mm512_mask_xor_epi32(u, mask_least, u, t);
	u = _mm512_mask_xor_epi32(u, mask_least, u, t2);

	_mm512_mask_extstore_epi32(&v, mask_least, u, _MM_DOWNCONV_EPI32_NONE, _MM_HINT_NONE);

	return v;
}

int main()
{
	int i;

	xorshift_knc32_init();

	for (i = 0; i < 32; i++)
		printf("0x%08"PRIx32"\n", xorshift_knc32());
	
	return 0;
}
