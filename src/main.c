#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

long long counter = 0;
int num_threads = 1;
int num_iterations = 1;

enum {
	THREADS = 1,
	ITERATIONS,
	LISTS,
	SYNC
};

static struct option long_options[] =
{
	{"threads", required_argument, 0, THREADS},
	{"iterations", required_argument, 0, ITERATIONS},
	{"lists", required_argument, 0, LISTS},
	{"sync", required_argument, 0, SYNC},
	{0, 0, 0, 0}
};

/* getopt_long stores the option index here. */
int option_index = 0;

int main (int argc, char **argv) {

	int c;
	while (1)
	{
		c = getopt_long(argc, argv, "", long_options, &option_index);

		/* Detect the end of the options. */
		if (c == -1)
			break;

		switch (c)
		{
			case THREADS:
				num_threads = strtol(optarg, NULL, 10);
				break;

			case ITERATIONS:
				num_iterations = strtol(optarg, NULL, 10);
				break;

			case LISTS:
				break;

			case SYNC:
				break;

			default:
				/* code for unrecognized options */
				break;
		}
	}

	/* check for nonpositive values */
	if (num_threads < 1)
		num_threads = 1;
	if (num_iterations < 1)
		num_iterations = 1;

	return 0;
}
