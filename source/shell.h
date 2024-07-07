#include <limits.h> // For PATH_MAX
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>
#include <sys/select.h>
#include <stdbool.h>

#define MAX_LINE 1024
#define BIN_PATH "./bin/"
#define MAX_PIDS 1024
#define IDLE_THRESHOLD 10
#define MAX_ARGS 100

typedef struct {
    char text_size[10];
    char color_scheme[10];
    char prompt_style[10];
} ShellConfig;

const char *builtin_commands[] = {
    "cd",    // Changes the current directory of the shell to the specified path. If no path is given, it defaults to the user's home directory.
    "help",  //  List all builtin commands in the shell
    "exit",  // Exits the shell
    "usage", // Provides a brief usage guide for the shell and its built-in command
    "env", // Lists all the environment variables currently set in the shell
    "setenv", // Sets or modifies an environment variable for this shell session
    "unsetenv" // Removes an environment variable from the shell
    };

/*
Handler of each shell builtin function
*/
int shell_cd(char **args);
int shell_help(char **args);
int shell_exit(char **args);
int shell_usage(char **args);
int list_env(char **args);
int set_env_var(char **args);
int unset_env_var(char **args);

void load_config(ShellConfig *config);
void save_config(ShellConfig *config);
void read_command(char **cmd);
void type_prompt(ShellConfig *config);
int  num_builtin_functions();
void add_pid(pid_t pid);
void pause_background_process(pid_t bg_process_pid);
void resume_background_process(pid_t bg_process_pid);
int  essential_process(pid_t bg_process_pid);
void low_power_mode();
void resume_normal_mode();
void check_idle_state();
void set_text_size(ShellConfig *config, const char *size);
void set_color_scheme(ShellConfig *config, const char *scheme);
void set_prompt_style(ShellConfig *config,const char *style);
void process_command(char *cmd[], ShellConfig *config);
void apply_color_scheme(const char *scheme);
void apply_text_size(const char *size);
void apply_prompt_style(const char *style);

/*** This is array of functions, with argument char ***/
int (*builtin_command_func[])(char **) = {
    &shell_cd,     // builtin_command_func[0]: cd 
    &shell_help,   // builtin_command_func[1]: help
    &shell_exit,   // builtin_command_func[2]: exit
    &shell_usage,  // builtin_command_func[3]: usage
    &list_env,     // builtin_command_func[4]: env
    &set_env_var,  // builtin_command_func[5]: setenv
    &unset_env_var // builtin_command_func[6]: unsetenv
};
