#include "parser.h"
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <sys/fcntl.h>
#include <stdio.h>

#define ALL_KEYS 256

typedef struct command {
	char c;
	char *command;
	struct command *lnode;
	struct command *rnode;
} Command;

static Command *G_STATE = NULL;
static Command *map[ALL_KEYS] = { 0 };

int load_config(char config_path[]) {
  FILE *f;
  if (NULL == ( f = fopen(config_path, "r"))) {
    perror("Can't open config file.\n");
    exit(1);
  }

  // Get File Size in Advance
  fseek(f, 0, SEEK_END);
  size_t file_size = ftell(f);
  fseek(f, 0, SEEK_SET);

  char *file_data;
  if (NULL == (file_data = malloc(file_size + 1))) {
    perror("Can't allocate for file size.");
    exit(2);
  }

  if ( file_size != fread(file_data, sizeof(char), file_size, f) ) {
    perror("Can't read all file.");
    exit(3);
  }

  fclose(f);

  char *key = strtok(file_data, "=");
  char *cmd = strtok(NULL, "\n");
  while (key && cmd) {
    if (cmd) {
      if (NULL != map[key[0]]) free(map[key[0]]);
      map[key[0]] = malloc(strlen(cmd) + 2);
      strcpy(map[key[0]], cmd);
      strcat(map[key[0]], " &");
    }
    key = strtok(NULL, "=");
    cmd = strtok(NULL, "\n");
  }

  free(file_data);
  return 0;
}

int exec(char c) {
  if (c > 0) {
		Command *command = map[c];
		if 
    system(map[c]);
    return 1;
  } else {
    perror("Key not mapped or invalid.");
    return -1;
  }
}
