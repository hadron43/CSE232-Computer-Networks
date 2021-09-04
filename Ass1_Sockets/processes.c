#include <dirent.h>
#include <string.h>

#define MAX_BUFFER_SIZE 1024
#define MAX_NO_PROCESSES 100000
#define PROCESS_NAME_LENGTH 128

void check_error(int return_val, char *str) {
    if(return_val < 0) {
        perror(str);
        exit(return_val);
    }
}

struct process {
    int pid;
    char name[PROCESS_NAME_LENGTH];
    int usage;
};

int is_higher(struct process *p1, struct process *p2) {
    // returns 1 if p1 has more usage than p2, 0 otherwise
    return p1 -> usage > p2 -> usage;
}

void print_process_struct(struct process *p) {
    printf("%d %s %d\n", p -> pid, p -> name, p -> usage);
}

void get_process_name(FILE *file, char *buff) {
    char pc = '.';
    char c = fgetc(file);
    int i = 0;
    while(!(c == ' ' && pc == ')')) {
        buff[i++] = c;
        pc = c;
        c = fgetc(file);
    }
    // fgetc(file);
    buff[i] = '\0';
}

struct process process_stat_file(char *filepath) {
    FILE* file = fopen(filepath, "r");
    char buff[PROCESS_NAME_LENGTH];
    struct process p;
    int index = 1;

    if(file == NULL) {
        perror(strcat("error while opening file ", filepath));
        p.pid = -1;     // to indicate error
        return p;
    }

    p.usage = 0;
    while(feof(file) == 0) {
        if(index == 2)
            get_process_name(file, buff);
        else
            fscanf(file, "%[^ ]%*c", buff);

        switch(index) {
            case 1:
                p.pid = atoi(buff);
                break;
            case 2:
                strcpy(p.name, buff);
                break;
            case 14:
            case 15:
                p.usage += atoi(buff);
                break;
        }

        index ++;
    }
    fclose(file);
    return p;
}

int is_valid_process_directory(char *d_name) {
    // returns 1, if true; 0 otherwise
    int ans = 1;
    int len = strlen(d_name);
    for(int i = 0; i < len; ++i) {
        if(d_name[i] >= '9' || d_name[i] <= '0') {
            ans = 0;
            break;
        }
    }
    return ans;
}

void get_list_of_processes(int *no_of_processes, int *list_of_process_ids) {
    *no_of_processes = 0;

    DIR *d;
    struct dirent *dir;
    d = opendir("/proc/");

    if (d) {
        while ((dir = readdir(d)) != NULL) {
            if(dir -> d_type != 4 || is_valid_process_directory(dir -> d_name) == 0)
                continue;
            list_of_process_ids[(*no_of_processes) ++] = atoi(dir -> d_name);
        }
        closedir(d);
    }
}

void swap_process_in_list(struct process *process_list, int i, int j) {
    struct process temp = process_list[i];
    process_list[i] = process_list[j];
    process_list[j] = temp;
}

int partition_process_list(struct process *process_list, int low, int high) {
    struct process *pivot = process_list + high;
    int i = low - 1;
    for(int j = low; j <= high - 1; j++) {
        if(is_higher(process_list + j, pivot)) {
            i++;
            swap_process_in_list(process_list, i, j);
        }
    }
    swap_process_in_list(process_list, i+1, high);
    return (i+1);
}

void sort_process_list(struct process *process_list, int low, int high) {
    if(low >= high)
        return;

    int pi = partition_process_list(process_list, low, high);
    sort_process_list(process_list, low, pi - 1);
    sort_process_list(process_list, pi + 1, high);
}

void write_processes_to_file(struct process *process_list, int no_of_processes,
 char *filepath, int N) {
    FILE *fptr;
    struct process *p;

    fptr = fopen(filepath, "w");
    if(fptr == NULL) {
        perror("error while creating data file");
        return;
    }

    for(int i = 0; i < N && i < no_of_processes; ++i) {
        p = process_list + i;
        // print_process_struct(p);
        fprintf(fptr, "%d %s %d\n", p -> pid, p -> name, p -> usage);
    }
    fclose(fptr);
}

void create_data_file(char *data_file_path, int N) {
    int list_of_process_ids[MAX_NO_PROCESSES];
    int no_of_processes;
    struct process *process_list;
    char filepath[512] = "";

    get_list_of_processes(&no_of_processes, list_of_process_ids);
    process_list = (struct process *) malloc(sizeof(struct process) * no_of_processes);

    for(int i = 0; i < no_of_processes; ++i) {
        sprintf(filepath, "/proc/%d/stat", list_of_process_ids[i]);
        process_list[i]  = process_stat_file(filepath);
    }

    sort_process_list(process_list, 0, no_of_processes - 1);
    write_processes_to_file(process_list, no_of_processes, data_file_path, N);

    free(process_list);
}

void send_end(int sd) {
    char *msg = "<end>";
    write(sd, msg, sizeof(msg));
}

int check_end(char *msg) {
    return strcmp(msg, "<end>") == 0;
}