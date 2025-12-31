#ifndef PARSER_H
#define PARSER_H

enum LEAD_STATE {
	LEAD_FAIL,
	LEAD_DONE,
	LEAD_WAIT
};

int load_config(char config_path[]);
void reload_config(char config_path[]);
int exec(char c);

#endif
