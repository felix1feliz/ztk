#include<stdio.h>
#include<string.h>

// Command Implementation Declarations
void help(char **args);

// Flag Setter Declarations
void flagUse(char **args);

struct Command {
	char *name;
	char *description;
	size_t argc;
	void (*implementation)(char **args);
};

const struct Command COMMANDS[] = {
	{
		.name = "help",
		.description = "help | Prints usage instructions",
		.argc = 0,
		.implementation = &help,
	}
};

struct Flag {
	// Little Name
	char *lname;
	// Big Name
	char * bname;
	char *description;
	size_t argc;
	void (*setter)(char **args);
	int used;
};

struct Flag FLAGS[] = {
	{
		.lname = "-u",
		.bname = "--use",
		.description = "-u --use <DIRECTORY> | Use specified directory instead of default location",
		.argc = 1,
		.setter = &flagUse
	}
};

/////////////
// OPTIONS //
/////////////

struct Options {
	char *zettels_dir;
};

struct Options options = {
	.zettels_dir = ""
};

/////////////////////////////
// COMMAND IMPLEMENTATIONS //
/////////////////////////////

void help(char **args) {
	printf("Usage: ztk <COMMAND> [OPTIONS]\n\n");
	printf("Commands:\n");
	for(size_t i = 0; i < sizeof(COMMANDS) / sizeof(COMMANDS[0]); ++i) {
		printf(" %s\n", COMMANDS[i].description);
	}
	printf("\nFlags:\n");
	for(size_t i = 0; i < sizeof(FLAGS) / sizeof(FLAGS[0]); ++i) {
		printf(" %s\n", FLAGS[i].description);
	}
}

//////////////////
// FLAG SETTERS //
//////////////////

void flagUse(char **args) {
	options.zettels_dir = args[1];
}

int main(int argc, char **argv) {
	// Command Commands Index
	size_t cindex = -1;
	// Command Args Index
	size_t aindex = -1;

	for(size_t i = 1; i < argc; ++i) {
		int valid_arg = 0;
		for(size_t j = 0; j < sizeof(COMMANDS) / sizeof(COMMANDS[0]); ++j) {
			if(strcmp(argv[i], COMMANDS[j].name) == 0) {
				if(cindex != -1) {
					printf("Error: More than one command specified\n");
					return 1;
				}

				if(COMMANDS[j].argc > argc - (i + 1)) {
					printf("Error: Command '%s' requires %zu positional argument(s)\n", COMMANDS[j].name, COMMANDS[j].argc);
					return 1;
				}

				cindex = j;
				aindex = i;
				i += COMMANDS[j].argc;
				valid_arg = 1;
				break;
			}
		}

		for(size_t j = 0; j < sizeof(FLAGS) / sizeof(FLAGS[0]); ++j) {
			if(
				strcmp(argv[i], FLAGS[j].lname) == 0 ||
				strcmp(argv[i], FLAGS[j].bname) == 0
			) {
				if(FLAGS[j].used) {
					printf("Error: Flag '%s' must not be repeated", FLAGS[j].bname);
					return 1;
				}

				if(FLAGS[j].argc > argc - (i + 1)) {
					printf("Error: Flag '%s' requires %zu positional argument(s)\n", FLAGS[j].bname, FLAGS[j].argc);
					return 1;
				}

				(*(FLAGS[j].setter))(&(argv[i]));
				FLAGS[j].used = 1;
				i += FLAGS[j].argc;
				valid_arg = 1;
				break;
			}
		}

		if(!valid_arg) {
			printf("Error: Unrecognized command '%s'\n", argv[i]);
			return 1;
		}
	}

	if(cindex == -1) {
		printf("Error: No command specified\n");
		help(NULL);
		return 1;
	}

	(*(COMMANDS[cindex].implementation))(&(argv[aindex]));

	return 0;
}
