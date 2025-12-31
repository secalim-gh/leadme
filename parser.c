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
	struct command *child;
	struct command *brother;
} Command;

static Command *G_STATE = NULL;
static Command *map[ALL_KEYS] = { 0 };

Command* get_cmd(Command *list, char c) {
	if (NULL == list) return NULL;
	for (Command *cmd = list; cmd != NULL; cmd = cmd->brother) {
		if (cmd->c == c) return cmd;
	}
	return NULL;
}

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
			if (!map[*key]) {
				map[*key] = malloc(sizeof(Command));
				map[*key]->c = *key;
				map[*key]->brother = NULL;
				map[*key]->child = NULL;
				map[*key]->command = NULL;
			}
			Command *current = map[*key++];

			for(; *key != '\0'; key++) {
				Command *exists = get_cmd(current->child, *key);						
				if (exists) {
					current = exists;
					continue;
				} else {
					Command *tmp = current->child;
					current->child = malloc(sizeof(Command));
					current->child->c = *key;
					current->child->brother = tmp;
					current->child->child = NULL;
					current->child->command = NULL;
					current = current->child;
				}
			}
			current->command = malloc(strlen(cmd) + 3);
      strcpy(current->command, cmd);
      strcat(current->command, " &\0");
    }
    key = strtok(NULL, "=");
    cmd = strtok(NULL, "\n");
  }

  free(file_data);
  return 0;
}

static void free_tree(Command *node) {
	if (!node) return;
	free_tree(node->child);
	free_tree(node->brother);

	free(node->command);
	free(node);
}

void reload_config(char config_path[]) {
	for (int i = 0; i < ALL_KEYS; i++) {
		if (map[i]) {
			free_tree(map[i]);
			map[i] = NULL;
		}
	}

	G_STATE = NULL;

	load_config(config_path);
	printf("Reloaded config\n");
}

int exec(char c) {
  if (c > 0) {
		Command *command = NULL == G_STATE ? map[c] : get_cmd(G_STATE, c);
		if (command && command->command) {
			printf("Found command: %s\n", command->command);
			system(command->command);
			G_STATE = NULL;
			return LEAD_DONE;
		}
		if (command != NULL) {
			G_STATE = command->child;
			return LEAD_WAIT;
		}
		return LEAD_FAIL;
  } else {
		return LEAD_FAIL;
	}
}
