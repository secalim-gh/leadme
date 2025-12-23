void client(void) {
	int s = socket(AF_UNIX, SOCK_STREAM, 0);
	struct sockaddr_un a = {.sun_family = AF_UNIX, .sun_path = SOCK_PATH};

	if (connect(s, (struct sockaddr*)&a, sizeof(a)) < 0) {
		fprintf(stderr, "Connection failed: Server does not exist\n");
		return;
	}

	char buffer[8] = {0};
	read(s, buffer, sizeof(buffer) - 1);

	if (strcmp(buffer, "QUIT") == 0) {
		fprintf(stderr, "There's already a client running. Exiting.\n");
		close(s);
		return;
	}

	printf("Connected. Server said: %s\n", buffer);
	close(s);
	return;
}
