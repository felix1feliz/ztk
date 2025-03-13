#include<stdlib.h>
#include<stdio.h>
#include<stdint.h>
#include<string.h>
#include<unistd.h>
#include<fcntl.h>
#include<sys/stat.h>
#include<sys/types.h>
#include<errno.h>

void die(int ecode);

/////////////
// OPTIONS //
/////////////

struct Options {
    char *zettels_dir;
    char *text_editor;
};

struct Options options = {0};

int getOptionsFD() {
    char options_file_path_size = sizeof(getenv("HOME")) + sizeof("/.config/ztk/options") + 1;
    char options_file_path[options_file_path_size];
    memset(options_file_path, 0, options_file_path_size);
    strcpy(options_file_path, getenv("HOME"));
    strcat(options_file_path, "/.config");
    int made = mkdir(options_file_path, S_IRWXU);
    if(made != 0 && errno != EEXIST) {
        printf("Error: Unable to create '~/.config' folder\n");
        exit(1);
    }
    strcat(options_file_path, "/ztk");
    made = mkdir(options_file_path, S_IRWXU);
    if(made != 0 && errno != EEXIST) {
        printf("Unable to create '~/.config/ztk' folder\n");
        exit(1);
    }
    strcat(options_file_path, "/options");

    int fd;
    if((fd = open(options_file_path, O_RDONLY | O_CREAT, S_IRUSR | S_IWUSR)) == -1) {
        printf("Error: Unable to create '~/.config/ztk/options' file\n");
        exit(1);
    }
    
    return fd;
}

void initOptions() {
    options.zettels_dir = NULL;
    options.text_editor = malloc(3);
    memset(options.text_editor, 0, 3);
    strcpy(options.text_editor, "vi");
}

void overwriteOptionsFile() {
    char options_file_path_size = sizeof(getenv("HOME")) + sizeof("/.config/ztk/options") + 1;
    char options_file_path[options_file_path_size];
    memset(options_file_path, 0, options_file_path_size);
    strcpy(options_file_path, getenv("HOME"));
    strcat(options_file_path, "/.config/ztk/options");
    int fd = open(options_file_path, O_WRONLY | O_TRUNC, S_IRUSR | S_IWUSR);
    write(fd, "\0\0", 2);
}

//
// OPTIONS FILE SPECIFICATION
//
// uint16_t zettel_dir_size;
// char zettel_dir[zettel_dir_size];
//
void getSavedOptions() {
    // Option File Descriptor
    int ofd = getOptionsFD();

    initOptions();

    uint16_t dir_size;
    if(read(ofd, &dir_size, 2) == 0) {
        printf("Error: Options file is corrupted or didn't exist\n\033[35mOverwriting options file\033[0m\n");
        close(ofd);
        overwriteOptionsFile();
        return;
    }

    if(dir_size == 0) {
        close(ofd);
        return;
    }

    options.zettels_dir = malloc(dir_size + 1);
    memset(options.zettels_dir, 0, dir_size + 1);
    if(read(ofd, options.zettels_dir, dir_size) == 0) {
        printf("Error: Options file is corrupted\n\033[35mOverwriting options file\033[0m\n");
        overwriteOptionsFile();
        free(options.zettels_dir);
        options.zettels_dir = NULL;
    }
    close(ofd);
}

///////////
// FLAGS //
///////////

// Flag Setter Declarations
void flagUse(char **args);

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

// Flags List
struct Flag FLAGS[] = {
    {
        .lname = "-u",
        .bname = "--use",
        .description = "-u --use <DIRECTORY> | Use specified directory instead of default location",
        .argc = 1,
        .setter = &flagUse
    }
};

// Flag Setters

void flagUse(char **args) {
    free(options.zettels_dir);
    options.zettels_dir = malloc(strlen(args[1]) + 1);
    memset(options.zettels_dir, 0, strlen(args[1]) + 1);
    strcpy(options.zettels_dir, args[1]);
}

//////////////
// COMMANDS //
//////////////

// Command Implementation Declarations
void help(char **args);
void commandUse(char **args);

struct Command {
    char *name;
    char *description;
    size_t argc;
    void (*implementation)(char **args);
};

// Command List
const struct Command COMMANDS[] = {
    {
        .name = "help",
        .description = "help | Prints usage instructions",
        .argc = 0,
        .implementation = &help,
    },
    {
        .name = "use",
        .description = "use <DIRECTORY> | Sets directory as zettels' location",
        .argc = 1,
        .implementation = &commandUse
    }
};

// Command Implementations

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

void commandUse(char **args) {
    if(strlen(args[1]) + strlen(getenv("HOME")) > UINT16_MAX) {
        printf("Error: dirname is too big");
        die(1);
    }
    char options_file_path_size = sizeof(getenv("HOME")) + sizeof("/.config/ztk/options") + 1;
    char options_file_path[options_file_path_size];
    memset(options_file_path, 0, options_file_path_size);
    strcpy(options_file_path, getenv("HOME"));
    strcat(options_file_path, "/.config/ztk/options");
    int fd = open(options_file_path, O_WRONLY | O_TRUNC, S_IRUSR | S_IWUSR);
    uint16_t dirpath_size = strlen(args[1]);
    write(fd, &dirpath_size, 2);
    write(fd, args[1], dirpath_size);
    close(fd);
}

//////////
// MAIN //
//////////

void die(int ecode) {
    if(options.zettels_dir != NULL) free(options.zettels_dir);
    free(options.text_editor);
    exit(ecode);
}

int main(int argc, char **argv) {
    getSavedOptions();

    // Command Commands Index
    size_t cindex = -1;
    // Command Args Index
    size_t aindex = -1;

    for(size_t i = 1; i < argc; ++i) {
        int valid_arg = 0;
        for(size_t j = 0; j < sizeof(COMMANDS) / sizeof(COMMANDS[0]); ++j) {
            if(strcmp(argv[i], COMMANDS[j].name) != 0)
                continue;

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
        for(size_t j = 0; j < sizeof(FLAGS) / sizeof(FLAGS[0]); ++j) {
            if(
                strcmp(argv[i], FLAGS[j].lname) != 0 &&
                strcmp(argv[i], FLAGS[j].bname) != 0
            ) continue;

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

    die(0);
    
    return 0;
}
