#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libgen.h>

FILE *fp_out;

long expand_state(long abs_count, unsigned* state, int nstate,
		long until_count)
{
	while(abs_count < until_count){
		fwrite(state, sizeof(state), nstate, fp_out);
		++abs_count;
	}
	return abs_count;
}

long fill_buffer(long abs_count, unsigned* state, int nstate)
{
	while((abs_count&0x0fffff) != 0){
		fwrite(state, sizeof(state), nstate, fp_out);
		++abs_count;
	}
	return abs_count;
}
#define NSTATE 1
#define MAXSTATE	6	/* 6 x DIO432 is max payload */

int main(int argc, char* argv[])
{
	char aline[128];
	int delta_times = 0;
	long abs_count = 0;
	unsigned old_state[MAXSTATE] = {};
	FILE *fp_log = 0;
	int nstate = 0;
	int nstate0 = 0;
	int nl = 0;
	int fill = 1;

	if (getenv("STL2RAW_NOFILL")){
		fill = 0;
	}


	if (argc > 1){
		fp_out = fopen(argv[1], "w");
		if (fp_out == 0){
			perror("failed to open outfile");
			return -1;
		}
		snprintf(aline, 128, "/tmp/%s", basename(argv[1]));
		fp_log = fopen(aline, "w");
	}else{
		fp_out = stdout;
	}
	while(fgets(aline, 128, stdin) && ++nl){
		unsigned count;
		unsigned state[MAXSTATE];
		char* pline = aline;

		if (fp_log) {
			fputs(aline, fp_log);
		}
		if (aline[0] == '#' || strlen(aline) < 2){
			continue;
		}else if (aline[0] == '+'){
			pline = aline + 1;
			delta_times = 1;	/* better make them all delta */
		}
		if ((nstate = sscanf(pline, "%u,%x,%x,%x,%x,%x,%x", &count,
				state+0, state+1, state+2,
				state+3, state+4, state+5) - 1) >= 1){
			if (nstate0){
				if (nstate != nstate0){
					fprintf(stderr, "ERROR: line:%d state count change %d=>%d."
							"Please apply state columns consistently\n",
							nl, nstate0, nstate);
					return -1;
				}
			}else{
				nstate0 = nstate;
			}
			abs_count = expand_state(
				abs_count, old_state, nstate,
				delta_times? abs_count+count: count);
			memcpy(old_state, state, sizeof(unsigned)*nstate);
		}else{
			fprintf(stderr, "scan failed\n");
			return -1;
		}
	}
	if (fill){
		fill_buffer(abs_count, old_state, nstate);
	}
}
