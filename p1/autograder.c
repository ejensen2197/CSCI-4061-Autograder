#include "include/utility.h"

char line[128];

void print_status(int **status_codes, char **executable_array, int num_of_sols, int num_of_params)
{
    // TODO: write the status of each executable file to autograder.out. Your output should align with expected.out
    FILE *fptr = fopen("autograder.out", "w");
    if (fptr == NULL)
    {
        printf("autograder.out was unable to be opened");
    }

    for (int i = 0; i < num_of_sols; i++){
        char *executable_name = strrchr(executable_array[i], '/');
        executable_name++;
        fprintf(fptr, "%s", executable_name);
        printf("%s", executable_name);
        for (int j = 1; j <= num_of_params; j++){
            printf(" %d\n", status_codes[i][j]);
            if (status_codes[i][j] == 0){
                fprintf(fptr, " %d(correct)", status_codes[i][j]);
            }
            else if (status_codes[i][j] == 1){
                fprintf(fptr, " %d(incorrect)", status_codes[i][j]);
            }
            else{
                fprintf(fptr, " %d(crash)", status_codes[i][j]);
            }
            fprintf(fptr, "\n");
        }
    }
    fclose(fptr);
}

// create a function to populate the status_codes array
void update_status_codes(int **status_codes, pid_t pid, int index, int status)
{
    // Iterate through array trying to match pid
    for (int i = 0; i < MAX_EXE; i++)
    {
        // first element is always the pid
        if (status_codes[i][0] == pid) 
        {
            // updating this element with a 1,2, or 3
            // ex) [[pid,1,2,2],[pid,1,1,2]]
            status_codes[i][index] = status;
            break;
        }
    }
}

int main(int argc, char *argv[])
{

    // ANY FUTHER ARGUMENTS AFTER 1 (which is batch size) ARE PARAMETERS THAT NEED TO BE CHECKED
    if (argc < 2)
    {
        printf("Usage: %s <batch> <p1> <p2> ... <pn>\n", argv[0]);
        return 1;
    }

    // Convert the first command-line argument to an integer to determine the batch size
    int batch_size = atoi(argv[1]);

    // write the file paths from the "solutions" directory into the submissions.txt file
    write_filepath_to_submissions("solutions", "submissions.txt");

    // COMPLETE: read the executable filename from submissions.txt

    // INIT file pointer and open file. Prints error statement if submissions.txt fails for some reason
    FILE *fptr = fopen("submissions.txt", "r");
    if (fptr == NULL)
    {
        printf("submissions.txt was unable to be opened");
        return 1;
    }
    int total_lines = 0;

    while (fgets(line, sizeof(line), fptr) != NULL)
    { // Gets number of lines in submissions.txt to INIT executable array
        total_lines += 1;
    }

    fclose(fptr);

    FILE *fptr1 = fopen("submissions.txt", "r");
    if (fptr1 == NULL)
    {
        printf("submissions.txt was unable to be opened");
        return 1;
    }
    char *executable_array[total_lines]; // INIT exectuable list for tracking

    int curr_line = 0;

    while (fgets(line, sizeof(line), fptr1) != NULL)
    {                                                           // Gets each line of submissions.txt. This includes the Newline after each file
        executable_array[curr_line] = malloc(strlen(line) + 1); // Need to malloc this mem as there was an issue with buffer being overwritten by last entry.
        if (executable_array[curr_line] == NULL)
        { // Malloc fail case
            printf("Memory allocation failed\n");
            fclose(fptr1);
            return 1;
        }
        strcpy(executable_array[curr_line], line); // Malloc success case and inserts each line into array
        curr_line++;
    }
    // Removes the "\n at the end of each executable in the list"
    for (int i = 0; i < total_lines; i++)
    {
        int length = strlen(executable_array[i]);
        executable_array[i][length - 1] = '\0'; // Replaces the last character of the string with the end string symbol '\0'
    }

    fclose(fptr1);

    // TODO: For each parameter, run all executables in batch size chunks
    int number_of_parameters = (argc - 2); // Needs to subtract two because one argument is file name and another is the batch size
    int parameters[number_of_parameters];  // Init a parameter array

    for (int i = 0; i < number_of_parameters; i++)
    {
        parameters[i] = atoi(argv[2 + i]); // Convert String into Int bc argv stores args as strings
    }
    
   
    
    int status;

    // organize in order of submissions.txt file 
    // int status_codes[MAX_EXE][number_of_parameters];
    
    // 2D array using double pointers so it can be a dynamic size
    int **status_codes = malloc((MAX_EXE + 1) * sizeof(int *));
    if (status_codes == NULL) 
    {
        perror("Failed to allocate memory for status_codes");
        exit(EXIT_FAILURE);
    }

    // loop through and allocate memory to store pid, and parameters in each element
    for (int i = 0; i < MAX_EXE; i++) 
    {
        status_codes[i] = malloc((number_of_parameters +1) * sizeof(int));
        if (status_codes[i] == NULL) 
        {
            perror("Failed to allocate memory for status_codes[i]");
            exit(EXIT_FAILURE);
        }
    }
     int done_executables = 0;

    for (int i = 0; i < number_of_parameters; i++) 
    {
        int current_executable = 0;
        while (done_executables < total_lines) 
        {
            for (int b = 0; b<batch_size; b++)
            {
                char str[64];
                sprintf(str, "%d", parameters[i]);
                pid_t pid = fork();

                // store the pid in the first element of the array to associate the status codes of each pid
                // status_codes[current_executable][0] = pid;
                // malloc enough space to store pid and number of parameters in each space
                
                status_codes[current_executable][0] = pid; 

                // have each child run one process
                if (pid == 0)
                {
                    execl(executable_array[current_executable], executable_array[current_executable], str , NULL);
                }
                current_executable++;
            }
            // meant to be number of finished executables in the current batch
            int num_finished = 0; 
            
            // runs while the current batch of executables is running
            while (num_finished < batch_size) 
            {
                pid_t result = waitpid(-1,&status, WNOHANG);
                if (result > 0) // an executable has finished   
                {
                    done_executables += 1;
                    if (WIFEXITED(status)) 
                    {
                        int answer = WEXITSTATUS(status);
                        // update the array to reflect the answer associated with the pid
                        if (answer == 0 || answer == 1){
                            update_status_codes(status_codes,result, i + 1, answer);
                        }
                        
                    }
                    else if (WIFSIGNALED(status)) 
                    {
                        update_status_codes(status_codes,result, i + 1, 3);
                        
                    }
                    // else{
                    //      update_status_codes(status_codes,result, i + 1, 2);
                    // }
                    num_finished++;
                }
                else if (result == 0)
                {
                    done_executables += 1;
                    sleep(3);
                }    
            }
        }
    }

    //THIS CURRENTLY RUNS INDEPENDENTLY OF BATCH SIZE WHICH NEEDS TO BE IMPLEMENTED STILL
    // for (int i = 0; i < total_lines; i++)
    // { // loop through each executable
    //     for (int j = 0; j < number_of_parameters; j++)
    //     { // loop through each parameter for each executable
    //         char str[64];
    //         sprintf(str, "%d", parameters[j]); // assign parameter to string so its passable using execl
    //         int pid = fork();
            
    //         if (pid == 0){//Child Process
    //             printf("%s",str);
    //             int process = execl(executable_array[i], executable_array[i], str, NULL); //Think this may be wrong

    //         }
    //         else{ //Parent Scenario
    //             wait(NULL);
    //         }
            
            
    //     }
    // }

    // TODO: Write the status of each executable file from "submissions.txt" to autograder.out. For testing purposes, you can compare the results with the provided expected.out file
    print_status(status_codes, executable_array, total_lines, number_of_parameters);

    return 0;
}
