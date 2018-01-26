#include <stdio.h>
#include <limits.h>
#include <memory.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <math.h>

int HASH_TABLE_SIZE = 0;

unsigned int* num_ent;

int get_hash_dc(int vrid, int fd)
{
	return ((( vrid + fd ) * ( vrid + fd + 1 ) / 2 ) + vrid ) % HASH_TABLE_SIZE;
}

int get_hash_pqa(int vrid, int fd)
{
	return ( vrid * 31 + fd * 37 ) % HASH_TABLE_SIZE;
}

int main(int argc, char **argv)
{
	double sum, sum_squares, mean, var;
	int i, j;
	int min, max;
	int fd_step = 2;
	int (*get_hash)(int, int) = get_hash_dc;
	int num_fd = 0;
	int num_vrid = 255;
	int opt;
	int begin_fd = 12;

	while ((opt = getopt(argc, argv, ":h:b:f:v:s:qH")) != -1) {
		switch(opt) {
		case 'h':
			HASH_TABLE_SIZE = atoi(optarg);
			if (!num_fd)
				num_fd = HASH_TABLE_SIZE;
			break;
		case 'b':
			begin_fd = atoi(optarg);
			break;
		case 'f':
			num_fd = atoi(optarg);
			break;
		case 'v':
			num_vrid = atoi(optarg);
			break;
		case 's':
			fd_step = atoi(optarg);
			break;
		case 'q':
			get_hash = get_hash_pqa;
			if (!HASH_TABLE_SIZE)
				HASH_TABLE_SIZE = 1151;
			break;
		case 'H':
			printf ("-h hash_size -f num_fd -v num_vrid -s fd_step -q primes_algo\n");
			exit(0);
			break;
		case '?':
			printf ("Unknown option -%c\n", optopt);
			exit(1);
			break;
		case ':':
			printf("Missing option argument for -%c\n", optopt);
			exit(1);
			break;
		}
	}

	if (!HASH_TABLE_SIZE)
		HASH_TABLE_SIZE = 1024;
	if (!num_fd)
		num_fd = 1024;

	num_ent = malloc(HASH_TABLE_SIZE * sizeof(num_ent[0]));
	memset(num_ent, 0, HASH_TABLE_SIZE * sizeof(num_ent[0]));

	for (i = begin_fd; i < begin_fd + num_fd * fd_step; i += fd_step) {
		for (j = 1; j <= num_vrid; j++)
			num_ent[get_hash(j, i)]++;
	}

	sum = sum_squares = 0;
	min = INT_MAX;
	max = 0;
	for (i = 0; i < HASH_TABLE_SIZE; i++) {
		sum += num_ent[i];
		sum_squares += num_ent[i] * num_ent[i];
		if (num_ent[i] < min)
			min = num_ent[i];
		if (num_ent[i] > max)
			max = num_ent[i];
	}

	/* Calculate mean and variance */
	mean = sum / HASH_TABLE_SIZE;
	var = sum_squares / HASH_TABLE_SIZE - mean * mean;

	printf ("mean %f, st dev %f, min %d, max %d\n", mean, sqrt(var), min, max);
}
