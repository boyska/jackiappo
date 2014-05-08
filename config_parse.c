#include <libconfig.h>
#include <stdlib.h>

#include "config_parse.h"

static int is_valid_conf(config_t* cfg);
static int is_valid_portrule(config_setting_t*);


int is_valid_portrule(config_setting_t *portrule) {
	if(portrule == NULL) {
		fprintf(stderr, "connrule children should contain 'from' and 'to' groups'\n");
		return 0;
	}
	if(!config_setting_is_group(portrule)) {
		fprintf(stderr, "'%s' should be a group'\n",
				config_setting_name(portrule));
		return 0;
	}
	/* TODO: should contain 'name', which is a string */
	return 1;
}

int is_valid_conf(config_t* cfg) {
	config_setting_t *newports, *connrule, *portrule;
	int new_i;
	newports = config_lookup(cfg, "newports");
	if(newports == NULL) {
		fprintf(stderr, "No 'newports' variable'\n");
		return 0;
	}
	if(!config_setting_is_list(newports)) {
		fprintf(stderr, "'newports' should be a list'\n");
		return 0;
	}
	for(new_i=0; new_i < config_setting_length(newports); new_i++) {
		connrule = config_setting_get_elem(newports, new_i);
		if(!config_setting_is_group(connrule)) {
			fprintf(stderr, "'newports' children should be groups'\n");
			return 0;
		}
		portrule = config_setting_get_member(connrule, "from");
		if(!is_valid_portrule(portrule)) {
			return 0;
		}
		portrule = config_setting_get_member(connrule, "to");
		if(!is_valid_portrule(portrule)) {
			return 0;
		}
		/*TODO:  and contains from and to, which are groups, too*/
	}
	return 1;
}
config_t* read_cfg(char* filename) {
	config_t *cfg;
	cfg = malloc(sizeof(config_t));
	config_init(cfg);
	if(!config_read_file(cfg, filename))
	{
		fprintf(stderr, "%s:%d - %s\n", config_error_file(cfg),
				config_error_line(cfg), config_error_text(cfg));
		config_destroy(cfg);
		free(cfg);
		return NULL;
	}

	if(!is_valid_conf(cfg)) {
		config_destroy(cfg);
		free(cfg);
		return NULL;
	}
	return cfg;
}
