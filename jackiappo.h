#include <libconfig.h>

struct passaround {
	char *to_connect;
	jack_client_t *client;
	config_t *cfg;
};
static void event_loop();
static void worker();
static void show_version (void);
static void show_usage (void);

