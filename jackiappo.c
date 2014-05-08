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
#include "config_parse.h"

#ifndef CLIENT_NAME
#define CLIENT_NAME "jackiappo"
#endif
#define SHOW_ALIASES 0
#define SHOW_CON 0

struct passaround *globals;

/* Yes, there are so much globals. Me suck. So what? */
char *my_name;

void do_port_action(const char *port_name) {
	/* This function do what's needed */
	int errorcode;
	unsigned int rules_i;
	char *from_varname, *to_varname;
	const char* from_port, *to_port;
	for(rules_i = 0;
			rules_i <
			config_setting_length(config_lookup(globals->cfg, "newports"));
			rules_i++) {
		asprintf(&from_varname, "newports.[%u].from.name", rules_i);
		from_port = config_setting_get_string(config_lookup(globals->cfg, from_varname));
		free(from_varname);

		if(strcmp(from_port, port_name) == 0) {
			/* Yay! connect */
			asprintf(&to_varname, "newports.[%u].to.name", rules_i);
			to_port = config_setting_get_string(config_lookup(globals->cfg, to_varname));
			free(to_varname);

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

void on_client_reg(const char *name, int registering, void *arg) {
	if(!registering) {
		fprintf(stdout, "client %s disconnecting\n", name);
		return;
	} else {
		fprintf(stdout, "client %s connecting\n", name);
	}
	fflush(stdout);
}
void on_port_reg(jack_port_id_t port_id, int registering, void *arg) {
	/* On Jack Port Registered: show_port */
	const char *name;
	jack_port_t *port;
	if(!registering) {
		return;
	} else {
		port = jack_port_by_id(globals->client, port_id);
		name = jack_port_name(port);
		globals->to_connect = calloc(strlen(name)+1, sizeof(char));
		strncpy(globals->to_connect, name, strlen(name)+1);
		fprintf(stderr, "To connect: [%s]\n", globals->to_connect);
		fflush(stderr);
	}
}

void show_ports() {
	const char **ports;
	int i;
	ports = jack_get_ports (globals->client, NULL, NULL, 0);
	if (!ports)
		return;

	for (i = 0; ports && ports[i]; ++i) {

		printf ("%s\n", ports[i]);

	} /* end for on ports */
	if (ports)
		jack_free (ports);
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
			config_fname = (char *) malloc (sizeof (char) * strlen(optarg));
			strcpy (config_fname, optarg);
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

	/* Open a client connection to the JACK server.  Starting a
	 * new server only to list its ports seems pointless, so we
	 * specify JackNoStartServer. */

	globals->client = jack_client_open (CLIENT_NAME, options, &status, server_name);
	if (globals->client == NULL) {
		if (status & JackServerFailed) {
			fprintf (stderr, "JACK server not running\n");
		} else {
			fprintf (stderr, "jack_client_open() failed, "
				 "status = 0x%2.0x\n", status);
		}
		return 1;
	}

	show_ports();
	globals->to_connect = NULL;
	int errorcode;
	errorcode = jack_set_port_registration_callback(globals->client, on_port_reg, NULL);
	if(errorcode != 0) {
		fprintf(stderr, "Error on port registration CB: %d\n", errorcode);
		return 1;
	}
	errorcode = jack_activate(globals->client);
	if(errorcode != 0) {
		fprintf(stderr, "Error activating: %d\n", errorcode);
		return 1;
	}


	event_loop();
	jack_client_close(globals->client);
	free(globals->cfg);
	exit (0);
}

static void event_loop() {
	struct timespec sleep_spec;
	sleep_spec.tv_sec = 0;
	sleep_spec.tv_nsec = 1000*1000*10; /* 10 milliseconds = 0.01 second */
	for(;;) {
		worker();
		nanosleep(&sleep_spec, NULL);
		continue;
	}
}

static void worker() {
	if(globals->to_connect == NULL)
		return;
	fprintf(stderr, "Going to connect: [%s]\n", globals->to_connect);
	fflush(stderr);
	do_port_action(globals->to_connect);
	free(globals->to_connect);
	globals->to_connect = NULL;
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
