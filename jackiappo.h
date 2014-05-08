#include <libconfig.h>

struct passaround {
	jack_client_t *client;
	config_t *cfg;
};
static void event_loop();
static void worker();
static void show_version (void);
static void show_usage (void);

/* These are work descriptions passed from callback thread back to main loop */
enum work_type { WORK_NEWPORT };
struct work_newport {
	char *to_connect;
};
union work_args {
	struct work_newport newport;
};
typedef struct {
	enum work_type type;
	union work_args args;
} work;
