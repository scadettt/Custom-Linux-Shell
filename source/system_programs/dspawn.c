#include "system_program.h"
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>

char output_file_path[PATH_MAX];

static int daemon_work() {
    // put your full PROJECT_DIR path here  
    strcpy(output_file_path, "/home/sadet/Downloads/Testerfile/programming-assignment-1-2024-c05_team/dspawnlog.txt"); 

    int num = 0;
    FILE *fptr;
    char *cwd;
    char buffer[1024];

    // write PID of daemon in the beginning
    fptr = fopen(output_file_path, "a");
    if (fptr == NULL)
    {
        return EXIT_FAILURE;
    }

    fprintf(fptr, "Daemon process running with PID: %d, PPID: %d, opening logfile with FD %d\n", getpid(), getppid(), fileno(fptr));

    // then write cwd
    cwd = getcwd(buffer, sizeof(buffer));
    if (cwd == NULL)
    {
        perror("getcwd() error");
        return 1;
    }

    fprintf(fptr, "Current working directory: %s\n", cwd);
    fclose(fptr);

    while (1)
    {

        // use appropriate location if you are using MacOS or Linux
        fptr = fopen(output_file_path, "a");

        if (fptr == NULL)
        {
            return EXIT_FAILURE;
        }

        fprintf(fptr, "PID %d Daemon writing line %d to the file.  \n", getpid(), num);
        num++;

        fclose(fptr);

        sleep(10);

        if (num == 10) // we just let this process terminate after 10 counts
            break;
    }

    return EXIT_SUCCESS;
}

int create_daemon(){
    //In the parent process, setsid has no effect on the session id, 
    // since the manual states that setsid only sets the process to be the session 
    //and process group leader if it is called by a process that is not a process group leader.

    char cwd[1024];
    int fd0, fd1, fd2;
    pid_t pid, pid2;

    pid = fork();
    
    if (pid < 0) {
          fprintf(stderr, "Fork has failed. Exiting now");
          return 1; // exit error
    } else if (pid == 0){
        //to make child process the session leader, will lose controlling TTY (terminal)
        //a process is a session leader when its pid==pgid
        //Note process group leader and session leader are 2 diff things
        // child tries setsid
        if (setsid() < 0) {
            perror("setsid() failed");
            return 1; // Exit error
        }
        
        //ignore SIGCHLD amd SIGHUP to prevent daemon from being killed if the session leader terminates
        signal(SIGHUP, SIG_IGN);
        signal(SIGCHLD, SIG_IGN);
        
        //fork again
        pid2 = fork();

        if (pid2 <0) {
            fprintf(stderr, "Fork has failed. Exiting now");
            return 1; // exit error
        } else if (pid2 ==0){
            //child process w pid2

            //set all new files created by it to have 0777 permission(world-RW & executable),
            //means that file it creates can be globally readable, writeable, and executable by any other processes.
            umask(0);
            
            //change working directory to root
            if (chdir("/") < 0) {
                perror("chdir() failed");
                return 1; // Exit error
            }

            //Close all open file descriptors 
            int x;
            for (x = sysconf(_SC_OPEN_MAX); x>=0; x--) { 
                close (x);
            }
            
            //Attach file descriptors 0, 1, and 2 to /dev/null.
            fd0 = open("/dev/null", O_RDWR);
            fd1 = dup(0);
            fd2 = dup(0);

        } else {
            //parent process w pid2
            _exit(0);
        }

    } else{
        _exit(0);
    }
    return 0; 
}

int main(int argc, char **args) {
    printf("testing func\n");
    create_daemon();

    return daemon_work();
} 