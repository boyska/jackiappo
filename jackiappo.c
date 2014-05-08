/*
 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#define _POSIX_C_SOURCE	199309L
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include <time.h>
#include <string.h>
#include <getopt.h>
#include <jack/jack.h>
#include "jackiappo.h"
#include "pipe.h"
#include "config_parse.h"
#include "rules.h"


#define CLIENT_NAME "jackiappo"
#define SHOW_ALIASES 0
#define SHOW_CON 0
#define SLEEP_MICROSECONDS 10000

/* Yes, there are so much globals. Me suck. So what? */
struct passaround *globals;

char *my_name;


void on_port_reg(jack_port_id_t port_id, int registering, void* producer) {
	/* This function will be called in the callback thread */
	/* Uses globals->client */
	const char *name;
	jack_port_t *port;
	work work;
	if(!registering) {
		return;
	} else {
		port = jack_port_by_id(globals->client, port_id);
		name = jack_port_name(port);
		work.type = WORK_NEWPORT;
		work.args.newport.to_connect = calloc(strlen(name)+1, sizeof(char));
		strncpy(work.args.newport.to_connect, name, strlen(name)+1);
		pipe_push((pipe_producer_t*)producer, &work, 1);
	}
}


int
main (int argc, char *argv[])
{
	jack_status_t status;
	jack_options_t options = JackNoStartServer;
	int c;
	int option_index;
	char *server_name = NULL;
	char *config_fname = NULL;
	pipe_t *pipe;
	pipe_producer_t* pipe_producer;
	pipe_consumer_t* pipe_consumer;

	struct option long_options[] = {
	    { "server", 1, 0, 's' },
	    { "config", 1, 0, 'c' },
		{ "help", 0, 0, 'h' },
		{ "version", 0, 0, 'v' },
		{ 0, 0, 0, 0 }
	};

	my_name = strrchr(argv[0], '/');
	if (my_name == 0) {
		my_name = argv[0];
	} else {
		my_name ++;
	}

	globals = malloc(sizeof(struct passaround));
	while ((c = getopt_long (argc, argv, "s:c:hv", long_options, &option_index)) >= 0) {
		switch (c) {
		case 's':
			server_name = (char *) malloc (sizeof (char) * strlen(optarg));
			strcpy (server_name, optarg);
			options |= JackServerName;
			break;
		case 'c':
			config_fname = (char *)malloc (sizeof (char) * strlen(optarg));
			strncpy (config_fname, optarg, strlen(optarg));
			break;
		case 'h':
			show_usage ();
			return 1;
			break;
		case 'v':
			show_version ();
			return 1;
			break;
		default:
			show_usage ();
			return 1;
			break;
		}
	}
	if(config_fname == NULL) {
		fprintf(stderr, "Option --config is mandatory\n");
		return 1;
	}
	globals->cfg = read_cfg(config_fname);
	free(config_fname);

	/* Open a client connection to the JACK server.  Starting a
	 * new server only to list its ports seems pointless, so we
	 * specify JackNoStartServer. */

	globals->client = jack_client_open(CLIENT_NAME, options, &status, server_name);
	free(server_name);
	if (globals->client == NULL) {
		if (status & JackServerFailed) {
			fprintf (stderr, "JACK server not running\n");
		} else {
			fprintf (stderr, "jack_client_open() failed, "
				 "status = 0x%2.0x\n", status);
		}
		return 1;
	}

	pipe = pipe_new(sizeof(work), 0);
	pipe_producer = pipe_producer_new(pipe);
	pipe_consumer = pipe_consumer_new(pipe);
	pipe_free(pipe);
	int errorcode;
	errorcode = jack_set_port_registration_callback(globals->client,
			on_port_reg, (void*)pipe_producer);
	if(errorcode != 0) {
		fprintf(stderr, "Error on port registration CB: %d\n", errorcode);
		return 1;
	}
	errorcode = jack_activate(globals->client);
	if(errorcode != 0) {
		fprintf(stderr, "Error activating: %d\n", errorcode);
		return 1;
	}


	event_loop(pipe_consumer);
	jack_client_close(globals->client);
	free(globals->cfg);
	pipe_producer_free(pipe_producer);
	return 0;
}

static void event_loop(pipe_consumer_t* pipe_consumer) {
	struct timespec sleep_spec;
	work work;
	size_t popresult;

	sleep_spec.tv_sec = 0;
	sleep_spec.tv_nsec = 1000 * SLEEP_MICROSECONDS; /* 10 milliseconds = 0.01 second */
	for(;;) {
		popresult = pipe_pop(pipe_consumer, &work, (size_t)1);
		if(popresult != 1) {
		fprintf(stderr, "Warning: pop returned %zu elements",
				popresult);
		}
		worker(work);
		nanosleep(&sleep_spec, NULL);
	}
	pipe_consumer_free(pipe_consumer);
}

static void worker(work work) {
	if(work.type == WORK_NEWPORT) {
		fprintf(stderr, "Considering port [%s]\n", work.args.newport.to_connect);
		fflush(stderr);
		do_port_action(work.args.newport.to_connect);
		free(work.args.newport.to_connect);
	} else {
		fprintf(stderr, "Warning: invalid work type received: %X",
				work.type);
		fflush(stderr);
	}
}

void do_port_action(const char *port_name) {
	/* This function do what's needed */
	int errorcode;
	unsigned int rules_i;
	const char* to_port;
	config_setting_t *base, *to_rule, *rule;
	
	base = config_lookup(globals->cfg, "newports");
	for(rules_i = 0; rules_i < config_setting_length(base); rules_i++) {
		//TODO: get rules_i-th child of base
		rule = config_setting_get_elem(base, rules_i);
		to_rule = port_matches(rule, port_name);

		if(to_rule != NULL) {
			/* Yay! connect */
			rule = config_setting_get_member(to_rule, "name");
			to_port = config_setting_get_string(rule);

			fprintf(stderr, "Connecting %s and %s\n", port_name, to_port);
			fflush(stderr);
			errorcode = jack_connect(globals->client, port_name, to_port);
			if(errorcode) {
				fprintf(stderr, "Error connecting: %d\n",
						errorcode);
				fflush(stderr);
			}
		}
	}
}

static void show_version (void)
{
}

static void show_usage (void) {
	show_version ();
	fprintf (stderr, "\nUsage: %s [options] [filter string]\n", my_name);
	fprintf (stderr, "List active Jack ports, and optionally display extra information.\n");
	fprintf (stderr, "Optionally filter ports which match ALL strings provided after any options.\n\n");
	fprintf (stderr, "Display options:\n");
	fprintf (stderr, "        -s, --server <name>   Connect to the jack server named <name>\n");
	fprintf (stderr, "        -h, --help            Display this help message\n");
	fprintf (stderr, "        --version             Output version information and exit\n\n");
	fprintf (stderr, "For more information see http://jackaudio.org/\n");
}
