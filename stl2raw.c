#include <stdio.h>
#include <string.h>
#include <libgen.h>

FILE *fp_out;

long expand_state(long abs_count, unsigned state, 
		long until_count)
{
	while(abs_count < until_count){
		fwrite(&state, sizeof(state), 1, fp_out);
		++abs_count;
	}
	return abs_count;
}

long fill_buffer(long abs_count, unsigned state)
{
	while((abs_count&0x0fffff) != 0){
		fwrite(&state, sizeof(state), 1, fp_out);
		++abs_count;
	}
	return abs_count;
}
int main(int argc, char* argv[])
{
	char aline[128];
	int delta_times = 0;
	long abs_count = 0;
	unsigned old_state = 0;
	FILE *fp_log = 0;

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
	while(fgets(aline, 128, stdin)){
		unsigned count, state;
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
		if (sscanf(pline, "%u,%x", &count, &state) == 2){
			abs_count = expand_state(
				abs_count, old_state, 
				delta_times? abs_count+count: count);
			old_state = state;
		}else{
			fprintf(stderr, "scan failed\n");
			return -1;
		}
	}
	fill_buffer(abs_count, old_state);
}
