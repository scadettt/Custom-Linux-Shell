// Include the shell header file for necessary constants and function declarations
#include "shell.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h> 
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/select.h>
#include <time.h>
#include <linux/limits.h>
#include <fcntl.h>
#include <errno.h>

#define CONFIG_FILE "custom.ini"

void load_config(ShellConfig *config) {

    strcpy(config->text_size, "normal");
    strcpy(config->color_scheme, "light");
    strcpy(config->prompt_style, "$$");

    FILE *file = fopen(CONFIG_FILE, "r");
    if (file == NULL) {
        perror("Could not open configuration file");
        return;
    }
    char line[256];
    while (fgets(line, sizeof(line), file)) {
        char *key = strtok(line, "=");
        char *value = strtok(NULL, "\n");

        if (strcmp(key, "text_size") == 0) {
            strncpy(config->text_size, value, sizeof(config->text_size));
        } else if (strcmp(key, "color_scheme") == 0) {
            strncpy(config->color_scheme, value, sizeof(config->color_scheme));
        } else if (strcmp(key, "prompt_style") == 0) {
            strncpy(config->prompt_style, value, sizeof(config->prompt_style));
        }
    }
    fclose(file);
}

void save_config(ShellConfig *config) {
    FILE *file = fopen("shell_config.ini", "w");
    if (file == NULL) {
        perror("Could not open configuration file for writing");
        return;
    }
    fprintf(file, "text_size = %s\n", config->text_size);
    fprintf(file, "color_scheme = %s\n", config->color_scheme);
    fprintf(file, "prompt_style = %s\n", config->prompt_style);
    fclose(file);
}

//Function to run .cseshellrc config file
void source_cseshellrc() {
    FILE *file = fopen(".cseshellrc", "r");
    if (file == NULL) {
        perror("Failed to open .cseshellrc");
        return;
    }

    char line[256];
    while (fgets(line, sizeof(line), file)) {
        // Remove newline character
        line[strcspn(line, "\n")] = 0;
        // Skip empty lines
        if (strlen(line) == 0) continue;
        // Check if the line starts with "PATH="
        if (strncmp(line, "PATH=", 5) == 0) {
            // Set the PATH environment variable using system command setenv
            if (setenv("PATH", line + 5, 1) != 0) {
                perror("Failed to set PATH");
            }
        } else if (strncmp(line, "PROJECT_DIR=", 12) == 0) {
            // Set the Project dir environment variable using system command setenv
            if (setenv("PROJECT_DIR", line + 12, 1) != 0) {
                perror("Failed to set PROJECT_DIR");
            }
        } else {
            // Execute other commands that are not related to setting PATH
            if (system(line) == -1) {
                perror("Command execution failed");
            }
        }
    }

    fclose(file);
}

////////////////////////////////////////////////////////////////////////////////////////////
// Function to read a command from the user input
void read_command(char **cmd)
{
  // Define a character array to store the command line input
  char line[MAX_LINE];
  // Initialize count to keep track of the number of characters read
  int count = 0, i = 0;
  // Array to hold pointers to the parsed command arguments
  char *array[MAX_ARGS], *command_token;

  // Infinite loop to read characters until a newline or maximum line length is reached
  for (;;)
  {
    // Read a single character from standard input
    int current_char = fgetc(stdin);
    // Store the character in the line array and increment count
    line[count++] = (char)current_char;
    // If a newline character is encountered, break out of the loop
    if (current_char == '\n')
      break;

    // If the command exceeds the maximum length, print an error and exit
    if (count >= MAX_LINE)
    {
      printf("Command is too long, unable to process\n");
      exit(1);
    }
  }

  // Null-terminate the command line string
  line[count] = '\0';

  // If only the newline character was entered, return without processing
  if (count == 1)
    return;

  // Use strtok to parse the first token (word) of the command
  command_token = strtok(line, " \n");

  // Continue parsing the line into words and store them in the array
  while (command_token != NULL)
  {
    array[i++] = strdup(command_token);  // Duplicate the token and store it
    command_token = strtok(NULL, " \n"); // Get the next token
  }

  // Copy the parsed command and its parameters to the cmd array
  for (int j = 0; j < i; j++)
  {
    cmd[j] = array[j];
  }

  // Null-terminate the cmd array to mark the end of arguments
  cmd[i] = NULL;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Function to display the shell prompt
bool prompt_displayed = false;
void type_prompt(ShellConfig *config)
{
  // Use a static variable to check if this is the first call to the function
  static int first_time = 1;
  if (first_time)
  {
    // Clear the screen on the first call
#ifdef _WIN32
    system("cls"); // Windows command to clear screen
#else
    system("clear"); // UNIX/Linux command to clear screen
#endif
    first_time = 0;
  }

  printf("%s ", config->prompt_style);  // Print the shell prompt
  fflush(stdout); // Flush the output buffer
  prompt_displayed = true;
}



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Helper function to figure out how many builtin commands are supported by the shell
int num_builtin_functions()
    {
        return sizeof(builtin_commands) / sizeof(char *);
    }

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**Task 5**/
// Additional shell feature
// Sustainability
/*Idle Resource Management:
Description: Implement a feature that reduces resource usage when the shell is idle.
Task: Add functionality to put the shell into a low-power mode when not in use, waking up only when a new command is entered.*/

// 1. Idle condition: no user input for >= 10 seconds
// 2. Check Idle state: calculate the difference between current time and last user input time
// 3. Enter Low-power mode: reduce resource usage by 1. pausing background processes to conserve resources; 2. entering sleep state to reduce cpu usage
// 4. Wake up when new command entered: use select() to indicate that stdin has input 


// Declare global variables
time_t last_activity_time = 0;
bool is_idle = false;
bool is_in_low_power_mode = false;
pid_t pid_array[MAX_PIDS];
int pid_count=0;

// Function to store pid of process in array
void add_pid(pid_t pid) {
  if (pid_count < MAX_PIDS) {
    pid_array[pid_count++] = pid;
  }
  else {
    fprintf(stderr, "Max number of PIDs reached. \n");
  }
}


void pause_background_process(pid_t bg_process_pid) {
  if (kill(bg_process_pid, SIGSTOP) == -1) {
        perror("kill");
        exit(EXIT_FAILURE);
    }
    printf("Paused background process with PID: %d\n", bg_process_pid);
}

void resume_background_process(pid_t bg_process_pid) {
  if (kill(bg_process_pid, SIGCONT) == -1) {
        perror("kill");
        // exit(EXIT_FAILURE);
        // return 1;
    }
    printf("Resumed background process with PID: %d\n", bg_process_pid);
}

int essential_process(pid_t bg_process_pid){
    char stat_path[64];
    snprintf(stat_path, sizeof(stat_path), "/proc/%d/stat", bg_process_pid );
    
    FILE *fp = fopen(stat_path, "r");
    if (!fp) {
        // perror("fopen");
        // printf("TESTING: %s\n", stat_path);
        return 1; // Default to essential if we can't read stat file
    }

    char buffer[1024];
    if (fgets(buffer, sizeof(buffer), fp) == NULL) {
        perror("fgets");
        fclose(fp);
        return 1; // Default to essential if we can't read stat file
    }
    fclose(fp);

    // Variables to hold the CPU times
    long utime, stime;
    sscanf(buffer, "%*d %*s %*c %*d %*d %*d %*d %*d %*u %*u %*u %*u %*u %ld %ld", &utime, &stime);

    // Total CPU time used by the process
    long total_cpu_time = utime + stime;

    // CPU threshold in clock ticks (10 seconds worth of ticks)
    long cpu_threshold = 10 * sysconf(_SC_CLK_TCK);

    // Determine if the process is essential based on CPU usage
    return (total_cpu_time > cpu_threshold) ? 1 : 0;
}  

void low_power_mode(){
  if (pid_count > 0) {
    for (int i=0; i<pid_count; i++){
      if (essential_process(pid_array[i]) == 0){
      pause_background_process(pid_array[i]);
  }
  }
  }
  printf("Shell is now entering sleep.\n");
  sleep(1);
}

void resume_normal_mode(){
  if (pid_count > 0){
    for (int i=0; i<pid_count; i++){
      resume_background_process(pid_array[i]); }
} }

// Function to check idle state
void check_idle_state(){
  time_t current_time = time(NULL);
    
  // Calculate time since last activity
  double idle_duration = difftime(current_time, last_activity_time);

  if (idle_duration>IDLE_THRESHOLD){
    if (!is_in_low_power_mode) {
      printf("\nShell is now entering low-power mode.\n");
      fflush(stdout);
      low_power_mode();
      is_in_low_power_mode = true;
    }
      is_idle = true;
  }
  else {
    if (is_in_low_power_mode) {
      // printf("TESTING2");
      printf("Shell is active.\n");
      fflush(stdout);
      resume_normal_mode();
      is_in_low_power_mode = false;
      }
      is_idle = false;
  }
}


//////////////////////////////////////////////////////////////////////////////////////////
// The main function where the shell's execution begins
int main(void) {
  // Define an array to hold the command and its arguments
  char *cmd[MAX_ARGS];
  int child_status;
  pid_t pid;
  char full_path[PATH_MAX];
  char cwd[1024];
  int status;
  fd_set rfds;
  struct timeval tv;
  int retval;

    /**TASK 1**/
// 1. Enters an infinite loop where it calls type_prompt to display the shell prompt and read_command to read and parse a command from the user.
// 2. Skips execution if the command is empty.
// 3. Exits the loop (and the shell) if the command is “exit”.
// 4. For other commands, it attempts to execute them by first forking the shell process using fork() then using execv in the child process to replace its image with the one specified by the command path.
// 5. The parent process (shell) should wait (block) for the child to complete before continuing using waitpid, and inspect child process’ exit status
// 6. Handles process creation errors and command not found errors.
// 7. Cleans up by freeing allocated memory for the command arguments.
// {.error} Your shell should never abruptly terminate, even when we give commands that don’t exist or simply pressing enter multiple times. Your shell should also be able to execute commands as-given, 
// it should not accidentally access garbage input values from uncleared buffers in any way. Failure to do this results in 1% grade penalty.

    
  //Run config file to configure PATH and other possible aliases
  source_cseshellrc();
  ShellConfig config;
  load_config(&config);
  apply_color_scheme(config.color_scheme);
  apply_text_size(config.text_size);
  printf("Text Size: %s\n", config.text_size);
  printf("Color Scheme: %s\n", config.color_scheme);
  printf("Prompt Style: %s\n", config.prompt_style);
    
  for (;;)
  {
        if (!prompt_displayed)
        {
            type_prompt(&config); // Display the prompt once
        }
      
    // Initialize file descriptor set
    FD_ZERO(&rfds);
    FD_SET(STDIN_FILENO, &rfds); // Monitor standard input

    // Set timeout interval to 1 second 
    tv.tv_sec = 30;
    tv.tv_usec = 0;

    // Use select() to wait for input on standard input or timeout
    retval = select(STDIN_FILENO + 1, &rfds, NULL, NULL, &tv);

    // Update last activity time if there is input
    if (retval == -1) {
      perror("select");
      continue;
    } // keyboard press detected
    else if(retval) {
      read_command(cmd); // Read a command from the user
      prompt_displayed = false; // Reset prompt_displayed flag after command input
      // Update last activity time
      last_activity_time = time(NULL);

      // If command is empty, skip execution
      if (cmd[0] == NULL) {
        
        continue;
        // return 0;
      }

      // If the command is "exit", break out of the loop to terminate the shell
      if (strcmp(cmd[0], "exit") == 0) {
        break;
        // return 1;
      }

      // Process customization commands
            if (strncmp(cmd[0], "set-", 4) == 0) {
                process_command(cmd, &config);
                continue;
            }

      // For other built-in commands, 
      // Loop through our command list and check if the commands exist in the builtin command list
      int found_builtin = 0;
      for (int command_index = 0; command_index < num_builtin_functions(); command_index++)
        {
          if (strcmp(cmd[0], builtin_commands[command_index]) == 0) // Assume args[0] contains the first word of the command
          {
            (*builtin_command_func[command_index])(cmd);
            found_builtin = 1;
            continue;
          }
        }

      // For other external commands,
      // We will create new process to run the function with the specific command except for builtin commands.
      // These have to be done by the shell process. 
      if (!found_builtin) {
            pid = fork();
            if (pid < 0) {
              fprintf(stderr, "Fork has failed. Exiting now");
              return 1; // exit error
            }
            else if (pid==0) {
              // // Formulate the full path of the command to be executed
              // if (getcwd(cwd, sizeof(cwd)) != NULL) {
              //   snprintf(full_path, sizeof(full_path), "%s/bin/%s", cwd, cmd[0]);
              // }
              // else {
              //   printf("Failed to get current working directory.");
              //   exit(1);
              // }
              // execv(full_path, cmd);

              // // If execv returns, command execution has failed
              // printf("Command %s not found\n", cmd[0]);
              // continue;

          //     //execute command, able to find it since PATH has been correctly configured
              if (execvp(cmd[0], cmd) == -1) {
              //if execvp returns, command execution has failed
              printf("Command %s not found\n", cmd[0]);
          }
          _exit(1);
            }

            else {
              add_pid(pid);
              printf("pid: %d\n", pid);
              int status, child_exit_status;
              waitpid(pid, &status, WUNTRACED);

              // If child terminates properly,
              if (WIFEXITED(status)) {
                child_exit_status = WEXITSTATUS(status);
                printf("Child has exited with status %d\n", child_exit_status);
              }
              // TODO: Not sure if handling of the abnormal termination is correct
              // For abnormal termination, reap the process to prevent it from becoming zombie
              else {
                if (WIFSIGNALED(status)) {
                  int signal_num = WTERMSIG(status);
                  printf("Child terminated by signal %d\n", signal_num);
                }
                else if (WIFSTOPPED(status)) {
                  int signal_num = WSTOPSIG(status);
                  printf("Child stopped by signal %d\n", signal_num);
                }
                else {
                  printf("Child did not terminate normally and was not terminated by signal \n");
                }
                }
              }
            }

      // Free the allocated memory for the command arguments before exiting
      if (sizeof(cmd) > 0) {
          
      for (int i = 0; cmd[i] != NULL; i++) {
        free(cmd[i]); //cmd is a pointer, this frees the memory but pointer still has value
        cmd[i]=NULL;  //this makes pointer null
        
      } }
      
      memset(cwd, '\0', sizeof(cwd)); // clear the cwd array
      }
    // Check idle state after handling command or timeout
    check_idle_state(); 
    }
  return 0; 
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**Task 2**/
// The shell must be expanded support the following 7 builtin commands: cd, help, exit, usage, env, setenv, unsetenv.

// Change the current working directory of the shell. 
// You can print out your current working directory as part of the shell’s prompt, or you can use the ld command to list the current directory.
int shell_cd(char **args) {
  if (args[1]==NULL) {
    fprintf(stderr, "CSEShell: expected argument to \"cd\"\n");
    return 1;
  }
  else {
    if (chdir(args[1]) != 0) {
      perror("CSEShell: No such file or directory");
      return 1;
    }
  }
  return 0;
}

// Print out all builtin commands in the shell.
int shell_help(char **args) {
  printf("CSEShell Interface\n");
  printf("Usage: command arguments\n");
  printf("The following commands are implemented within the shell:\n");
  for (int command_index = 0; command_index < num_builtin_functions(); command_index++)
  {
    printf("%s\n", builtin_commands[command_index]);
    }
  return 0;
  }

int shell_exit(char **args) {
  return 1;
}

// Print a brief description on how to use each builtin command. 
// Also prints a useful message if the command given is not part of CSEShell’s builtin command.
int shell_usage(char **args) {
  if (args[1] == NULL) {
    printf("Command not given. Type usage <command>.\n");
  }
  else if (strcmp(args[1], "cd") == 0) {
    printf("Type: cd directory_name to change the current working directory of the shell\n");
  }
  else if (strcmp(args[1], "help") == 0) {
    printf("Type: help for supported commands\n");
  }
  else if (strcmp(args[1], "exit") == 0) {
    printf("Type: exit to terminate the shell gracefully\n");
  }
  else if (strcmp(args[1], "usage") == 0) {
    printf("Type: usage cd/help/exit\n"); 
  }
  else if (strcmp(args[1], "env") == 0) {
    printf("Type: env to list all registered env variables\n");
  }
  else if (strcmp(args[1], "setenv") == 0) {
    printf("Type: setenv ENV=VALUE to set a new env variable\n");
  }
  else if (strcmp(args[1], "unsetenv") == 0) {
    printf("Type: unsetenv ENV to remove this env from the list of env variables\n");
  }
  else {
    printf("The command you gave: %s, is not part of CSEShell's builtin command\n", args[0]);
  }
  return 0;
}


// Print all environment variables of this shell. This should inherit your system’s environment variables too.
// Declaration of the external variable 'environ'
// environ is actually defined in the C standard library

extern char **environ; 
int list_env(char **args) {
  char **env = environ; // Pointer to the array of environment strings
  while (*env != NULL) { // Loop until NULL pointer is encountered
    printf("%s\n", *env); // Print the current environment variable
    env++; // Move to the next environment variable
  }
    return 0;
}


// The command setenv KEY=VALUE simply adds to the list of this process’ environment variables. It should not print anything:
int set_env_var(char **args) {
  if (args[1]==NULL) {
    fprintf(stderr, "Usage: setenv KEY=VALUE\n");
    return 1;
  }

  char *key_value = args[1];
  char *equal_sign = strchr(key_value, '=');

  if (equal_sign == NULL || equal_sign == key_value) {
    fprintf(stderr, "Invalid format. Usage: setenv KEY=VALUE\n");
    return 1;
  }

  *equal_sign = '\0'; // Split key_value into key and value
  char *key = key_value;
  char *value = equal_sign + 1;

    // Set the environment variable
    if (setenv(key, value, 1) != 0) {
        perror("setenv");
        return 1;
    }
    return 0;
}


// This command unset KEY should delete any environment variable whose KEY matches any existing environment variables. 
// If you attempt to unset an environment variable that does not exist using unsetenv(), the function is considered to succeed, and it returns 0. 
int unset_env_var(char **args) {
  if (args[1]==NULL) {
    return 0;
  }
  if (unsetenv(args[1])!=0){
    return 0;
  }
  return 0;
}

void set_text_size(ShellConfig *config, const char *size) {
    strncpy(config->text_size, size, sizeof(config->text_size) - 1);
    save_config(config);
    apply_text_size(size);
}

void set_color_scheme(ShellConfig *config, const char *scheme) {
    strncpy(config->color_scheme, scheme, sizeof(config->color_scheme) - 1);
    save_config(config);
    apply_color_scheme(scheme);
}

void set_prompt_style(ShellConfig *config, const char *style) {
    if (strcmp(style, "simple") == 0) {
        strncpy(config->prompt_style, "$", sizeof(config->prompt_style) - 1);
    } else if (strcmp(style, "double") == 0) {
        strncpy(config->prompt_style, "$$", sizeof(config->prompt_style) - 1);
    } else {
        // Allow user-defined text as a style
        strncpy(config->prompt_style, style, sizeof(config->prompt_style) - 1);
    }
    config->prompt_style[sizeof(config->prompt_style) - 1] = '\0'; // Ensure null-termination

    printf("Setting prompt style to: %s\n", config->prompt_style); // Debug print

    save_config(config); // Save the updated configuration
}

void process_command(char *cmd[], ShellConfig *config) {
    if (strcmp(cmd[0], "set-text-size") == 0 && cmd[1] != NULL) {
        strncpy(config->text_size, cmd[1], sizeof(config->text_size) - 1);
        config->text_size[sizeof(config->text_size) - 1] = '\0'; // Ensure null-termination
        apply_text_size(config->text_size);
        printf("Text size set to: %s\n", config->text_size);
    } else if (strcmp(cmd[0], "set-color-scheme") == 0 && cmd[1] != NULL) {
        strncpy(config->color_scheme, cmd[1], sizeof(config->color_scheme) - 1);
        config->color_scheme[sizeof(config->color_scheme) - 1] = '\0'; // Ensure null-termination
        apply_color_scheme(config->color_scheme);
        printf("Color scheme set to: %s\n", config->color_scheme);
    } else if (strcmp(cmd[0], "set-prompt-style") == 0 && cmd[1] != NULL) {
        set_prompt_style(config, cmd[1]); // Pass config and style argument to set_prompt_style
    } else {
        printf("Unknown customization command: %s\n", cmd[0]);
    }
}

void apply_color_scheme(const char *scheme) { 
if (strcmp(scheme, "red") == 0) { 
        printf("\033[31m"); // Red text 
    } else if (strcmp(scheme, "green") == 0) { 
        printf("\033[32m"); // Green text 
    } else if (strcmp(scheme, "blue") == 0) { 
        printf("\033[34m"); // Blue text 
    } else if (strcmp(scheme, "yellow") == 0) { 
        printf("\033[33m"); // Yellow text 
    } else if (strcmp(scheme, "magenta") == 0) { 
        printf("\033[35m"); // Magenta text 
    } else if (strcmp(scheme, "cyan") == 0) { 
        printf("\033[36m"); // Cyan text 
    } else if (strcmp(scheme, "dark") == 0) { 
        printf("\033[90m"); // dark text 
    } else { 
        printf("\033[37m"); // Default text color (light) 
    } 
 
}

void apply_text_size(const char *size) { 
    if (strcmp(size, "bold") == 0) { 
        printf("\033[1m"); // Bold text 
    } else if (strcmp(size, "normal") == 0) { 
        printf("\033[22m"); // Normal intensity (not bold) 
    } else { 
        printf("\033[0m"); // Reset to default (normal intensity, default text size) 
    } 
}

void apply_prompt_style(const char *style) {
    printf("Prompt style: %s\n", style);
}


