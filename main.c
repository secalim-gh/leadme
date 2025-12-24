#include "parser.h"
#include "server.c"
#include "client.c"

int main(int argc, char **argv) {
	
	if (argc >= 2) {
		if (!strcmp(argv[1], "-s")) {
			server();
		} else {
			fprintf(stderr, "Invalid option. Use -s to launch server.\n");
			exit(EXIT_FAILURE);
		}
	} else {
		client();
	}

	return 0;
}

