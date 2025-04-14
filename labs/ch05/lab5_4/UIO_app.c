#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <sys/mman.h>

#define RP1_RW_OFFSET 0x0000
#define RP1_XOR_OFFSET 0x1000
#define RP1_SET_OFFSET 0x2000
#define RP1_CLR_OFFSET 0x3000

#define RP1_RIO_OUT 0x00
#define RP1_RIO_OE 0x04
#define RP1_RIO_IN 0x08

#define GPIO_FUNCSEL_MASK 0b11111
#define PAD_OUT_DIS_MASK 1 << 7

long read_long_from_file(char *path)
{
	FILE *fp;
	long val;

	if ((fp = fopen(path, "r")) == NULL)
	{
		fprintf(stderr, "failed to open %s, err = %d\n", path, errno);
		return -1;
	}

	fscanf(fp, "0x%lx", &val);
	return val;
}
void *map_resource(int fd, int uio, int map, long *sz)
{
	char path[PATH_MAX];
	long offset, size;
	void *addr;

	sprintf(path, "/sys/class/uio/uio%d/maps/map%d/size", uio, map);
	if ((size = read_long_from_file(path)) < 0)
		return NULL;

	sprintf(path, "/sys/class/uio/uio%d/maps/map%d/offset", uio, map);
	if ((offset = read_long_from_file(path)) < 0)
		return NULL;

	sprintf(path, "/dev/uio%d", uio);
	if ((fd = open(path, O_RDWR | O_SYNC)) < 0)
	{
		fprintf(stderr, "failed to open %s, err = %d\n", path, errno);
		return NULL;
	}

	addr = mmap(0, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, map * getpagesize());
	if (addr == MAP_FAILED)
	{
		fprintf(stderr, "failed to map register %d\n", map);
		close(fd);
		return NULL;
	}

	*sz = size;
	return addr + offset;
}

void initialize_gpio(int pin, void *gpio_base, void *rio_base, void *pads_base)
{
	void *gpio_pin = gpio_base + pin * sizeof(int) * 2 + 4;
	void *rio_pin = rio_base;
	void *pads_pin = pads_base + 4 + pin * sizeof(int);

	/* Initialize sys_rio */
	*(int *)(rio_pin + RP1_CLR_OFFSET + RP1_RIO_OUT) = 1 << pin;
	*(int *)(rio_pin + RP1_SET_OFFSET + RP1_RIO_OE) = 1 << pin;

	/* Setup muxing */
	int ctrl = *(int *)gpio_pin;
	ctrl = (ctrl & (~GPIO_FUNCSEL_MASK)) | 0b00101;
	*(int *)(gpio_pin) = ctrl;

	/* Clear output disabled bit */
	int pad = *(int *)pads_pin;
	pad = pad & (~PAD_OUT_DIS_MASK);
	*(int *)pads_pin = pad;
}

int main()
{
	void *gpio_base, *rio_base, *pads_base;
	long gpio_sz, rio_sz, pads_sz;
	int fd, pin = 22; // 27;
	char sendstring[128];

	printf("Starting led example\n");

	fd = open("/dev/uio0", O_RDWR | O_SYNC);
	if (fd < 0)
	{
		perror("Failed to open the device\n");
		exit(EXIT_FAILURE);
	}
	printf("opened /dev/uio0\n");

	gpio_base = map_resource(fd, 0, 0, &gpio_sz);
	if (!gpio_base)
	{
		fprintf(stderr, "failed to map gpio registers\n");
		exit(EXIT_FAILURE);
	}
	printf("mappged gpio_base = 0x%p, gpio_sz = %ld\n", gpio_base, gpio_sz);

	rio_base = map_resource(fd, 0, 1, &rio_sz);
	if (!rio_base)
	{
		fprintf(stderr, "failed to map rio registers\n");
		exit(EXIT_FAILURE);
	}
	printf("mappged rio_base = 0x%p, rio_sz = %ld\n", rio_base, rio_sz);

	pads_base = map_resource(fd, 0, 2, &pads_sz);
	if (!rio_base)
	{
		fprintf(stderr, "failed to map rio registers\n");
		exit(EXIT_FAILURE);
	}
	printf("mappged pads_base = 0x%p, pads_sz = %ld\n", pads_base, pads_sz);

	printf("mapped all registers\n");

	initialize_gpio(pin, gpio_base, rio_base, pads_base);

	printf("initialized gpio pin %d\n", pin);

	do
	{
		printf("Enter led value: on, off, or exit:\n");
		scanf("%[^\n]%*c", sendstring);

		if (strcmp("on", sendstring) == 0)
		{
			*(int *)(rio_base + RP1_SET_OFFSET + RP1_RIO_OUT) = 1 << pin;
		}
		else if (strcmp("off", sendstring) == 0)
		{
			*(int *)(rio_base + RP1_CLR_OFFSET + RP1_RIO_OUT) = 1 << pin;
		}
		else if (strcmp("exit", sendstring) == 0)
		{
			printf("Exit application\n");
		}
		else
		{
			printf("Bad value\n");
			return -EINVAL;
		}
	} while (strncmp(sendstring, "exit", strlen(sendstring)));

	printf("Application terminated\n");
	return 0;
}
