#include "config_parse.h"

int main(int argc, char* argv[]) {
	config_t *cfg;
	if(argc <= 1) {
		fprintf(stderr, "Not enough args\n");
		return 1;
	}
	char *fname = argv[1];
	cfg = read_cfg(fname);
	return 0;
}
