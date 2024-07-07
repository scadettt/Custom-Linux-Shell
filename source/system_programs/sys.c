#include "system_program.h"

int get_os_kernel() {
    //variable info
    struct utsname info;

    //execute system command, uname, that will retrieve system info
    if (uname(&info) != 0) {
        perror("uname unable to retrieve info.\n");
        return 1;
    }

    //variable kernel_v array of char
    char kernel_version[256];
    //copy sys info to variable kernel_version
    strncpy(kernel_version, info.release, sizeof(kernel_version)-1);  

    //Ensure null termination
    kernel_version[sizeof(kernel_version)-1] = '\0';

    //tokenise kernel_version array, so that token will point to the first part before "-" as 1 string
    char *token = strtok(kernel_version, "-");

    //copy to first part to kernel_version variable
    if (token != NULL) {
        strncpy(kernel_version, token, sizeof(kernel_version) - 1);
        //Ensure null termination since strncpy overwrites the whole content
        kernel_version[sizeof(kernel_version) - 1] = '\0'; 
    } else {
        //if no token is empty
        strcpy(kernel_version, "No information available.\n");
        //Ensure null termination since strncpy overwrites the whole content
        kernel_version[sizeof(kernel_version)-1] = '\0';
    }
    
    //print out all info
    printf("OS: %s\n", info.sysname);   
    printf("Kernel: %s\n", kernel_version);
    printf("Hostname: %s\n", info.nodename);
    return 0;
}

int get_memtotal() {
    //variables
    FILE *file_content;
    char buffer[256];
    long total_memory = 0;

    // Open the /proc/meminfo file
    file_content = fopen("/proc/meminfo", "r");
    if (file_content == NULL) {
        perror("fopen: ");
        return 1;
    }

    // Read the file line by line and store it to buffer char array
    while (fgets(buffer, sizeof(buffer), file_content)) {
        // sscanf checks if the line fits the format "MemTotal: %ld kB", 
        // if it does, extract its value into total_memory and return number of fields extracted.
        // if statement checks if we managed to extract 1 field, if so, break
        if (sscanf(buffer, "MemTotal: %ld kB", &total_memory) == 1) {
            break;
        }
    }

    // Close the file
    fclose(file_content);

    // Print the total memory size in kilobytes
    if (total_memory != 0) {
        printf("Total Memory: %ld kB\n", total_memory);
    } else {
        printf("Error.\n");
    }
    return 0;
}

int get_CPU() {
    FILE* file_content = fopen("/proc/cpuinfo", "r");
    if (!file_content) {
        perror("fopen");
        return 1;
    }

    char line[256];
    //read line by line
    while (fgets(line, sizeof(line), file_content)) {
        //if model name found
        if (strncmp(line, "model name", 10) == 0) {
            line[strcspn(line, "\n")] = '\0';
            //search for ":" in the line and assign the position that is 2 space ahead to cpu pointer
            char* cpu = strchr(line, ':') + 2;
            printf("CPU Model: %s\n", cpu);
            break;
        }
    }

    fclose(file_content);
    return 0;
}

int get_uptime(){
    FILE *file_content;
    char buffer[128];
    double uptime;

    // Open /proc/uptime file for reading
    file_content = fopen("/proc/uptime", "r");
    if (file_content == NULL) {
        perror("Failed to read /proc/uptime \n");
        return 1;
    }

    // Read uptime value
    if (fgets(buffer, 128, file_content) != NULL) {
        sscanf(buffer, "%lf", &uptime); // Read uptime as double
        printf("Uptime: %.2f seconds\n", uptime);
    } else {
        printf("%s", "Error getting uptime.\n");
    }

    // Close the file
    fclose(file_content);

    return 0;
}

int main() {
    system("clear");
    int temp;
    //function table
    typedef int (*InfoTable)();
    InfoTable info_table[] = {
        get_os_kernel,
        get_memtotal,
        get_CPU,
        get_uptime
    };

    // Run through each function 
    for (int i=0; i < sizeof(info_table)/sizeof(info_table[0]); i++) {
        //check should always be 1 if function executed susccesfully
        int check = info_table[i]();
        if (check==1) {
            printf("Error %d.\n", i);
            return 1;
        }
    }
    return EXIT_SUCCESS;
}