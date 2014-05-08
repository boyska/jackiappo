#include <libconfig.h>
#include <jack/jack.h>
#include <string.h>

/* Returns to_rule, if the fromrule matches with the port specified by
 * port_name */
config_setting_t* port_matches(config_setting_t *rule, const char* port_name) {
	config_setting_t *fromrule = config_setting_get_member(rule, "from");
	if(fromrule == NULL) {
		fprintf(stderr, "From rule not found!\n");
		fflush(stderr);
		fprintf(stderr, "We are in [%s]\n", config_setting_name(rule));
		fflush(stderr);
		return NULL;
	}

	const char* rule_name = config_setting_get_string(
			config_setting_get_member(fromrule, "name"));
	if(strcmp(rule_name, port_name) == 0) {
		return config_setting_get_member(rule, "to");
	}
	return NULL;
}
