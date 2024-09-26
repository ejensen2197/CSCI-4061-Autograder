#include "include/utility.h"

char line[128];
char str[64];

void print_status(int **status_codes, char **executable_array, int num_of_sols, int num_of_params,int* parameters)
{
    FILE *fptr = fopen("autograder.out", "w");
    if (fptr == NULL)
    {
        printf("autograder.out was unable to be opened");
    }
    for (int i = 0; i < num_of_sols; i++)
    {
        char *executable_name = strrchr(executable_array[i], '/');
        executable_name++;
        fprintf(fptr, "%s", executable_name);
        for (int j = 0; j < num_of_params; j++){
            if (status_codes[i][j+1] == 0)
            {
                fprintf(fptr, " %d(correct)", parameters[j]);
            }
            else if (status_codes[i][j+1] == 1)
            {
                fprintf(fptr, " %d(incorrect)", parameters[j]);
            }
            else
            {
                fprintf(fptr, " %d(crash)", parameters[j]);
            }
        }
        fprintf(fptr,"\n");
    }
    fclose(fptr);
}


void free_executables(char **executable_array, int size)
{
    for (int i = 0; i < size; i++)
    {
        free(executable_array[i]);
    }
}


void free_status(int **status_codes, int size)
{
    for (int i = 0; i < size; i++)
    {
        if(status_codes[i] != NULL)
        {
            free(status_codes[i]);
        }
    }
    free(status_codes);
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


int get_total_lines(){
    int total_lines = 0;
    FILE *fptr = fopen("submissions.txt", "r");
    //Fail can't open fail case
    if (fptr == NULL)
    {
        printf("submissions.txt was unable to be opened");
        exit(0);
    }
    // Gets number of lines in submissions.txt to INIT executable array
    while (fgets(line, sizeof(line), fptr) != NULL)
    { 
        total_lines += 1;
    }

    fclose(fptr);
    return total_lines;
}


int get_number_of_parameters(int argc){

    // Needs to subtract two because one argument is file name and another is the batch size
    int number_of_parameters = (argc - 2); 

    return number_of_parameters;
}


int* initialize_and_populate_parameter_array(int number_of_parameters, char *argv[]) {
    //Create parameteres array
    int* parameters = malloc(number_of_parameters * sizeof(int));
    //Malloc fail case
    if (parameters == NULL) {
        perror("Memory allocation failed");
        exit(0);
    }

    // Populate the array with command-line arguments converted to integers
    for (int i = 0; i < number_of_parameters; i++) {
        // Convert String into Int as argv stores args as strings
        parameters[i] = atoi(argv[2 + i]); 
    }
    
    return parameters;
}



int** create_status_codes_array(int number_of_parameters,int num_of_sols){

    //Create the status codes array 
    int **status_codes = malloc((num_of_sols + 1) * sizeof(int *));
    //Malloc fail case
    if (status_codes == NULL) 
    {
        perror("Failed to allocate memory for status_codes");
        exit(0);
    }

    // loop through and allocate memory to store pid, and parameters in each element
    for (int i = 0; i < num_of_sols; i++) 
    {
        //Makes room for params plus a PID for matching
        status_codes[i] = malloc((number_of_parameters + 1) * sizeof(int));
        if (status_codes[i] == NULL) 
        {
            perror("Failed to allocate memory for status_codes[i]");
            exit(EXIT_FAILURE);
        }
    }
    //return status codes array
    return status_codes;
}



//Initializes the executable array with the executables from submissions.txt
int initialize_executable_array(char** executable_array, int total_lines){
    //Init curr line and num of sols to track pos in array 
    int curr_line = 0;
    int num_of_sols = 0;
    FILE *fptr1 = fopen("submissions.txt", "r");
    //Fail open fail case
    if (fptr1 == NULL)
    {
        printf("submissions.txt was unable to be opened");
        exit(0);
    }
     // Gets each line of submissions.txt. This includes the Newline after each file which will be removed at bottom of func
    while (fgets(line, sizeof(line), fptr1) != NULL)
    {           
        // Need to malloc this mem as there was an issue with buffer being overwritten by last entry.                                              
        executable_array[curr_line] = malloc(strlen(line) + 1);

         // Malloc fail case
        if (executable_array[curr_line] == NULL)
        {
            printf("Memory allocation failed\n");
            fclose(fptr1);
            return 1;
        }
        // Malloc success case and inserts each line into array
        strcpy(executable_array[curr_line], line);
        //Increase curr line and num of sols vars
        curr_line++;
        num_of_sols++;
    }
    // Removes the "\n at the end of each executable in the list"
    for (int i = 0; i < total_lines; i++)
    {
        int length = strlen(executable_array[i]);

        // Replaces the last character of the string with the end string symbol '\0' by moving pointer
        executable_array[i][length - 1] = '\0'; 
    }

    fclose(fptr1);
    return num_of_sols;
}



void run_executables(int **status_codes, int* parameters, char** executable_array, int number_of_parameters, int total_lines, int batch_size, int status){
    for (int i = 0; i < number_of_parameters; i++) 
    {
        //Reset these 2 vars after each "run" of executables. 
        int done_executables = 0; 
        int current_executable = 0;

        while (done_executables < total_lines) 
        {
            // Fork a batch of processes

            // Track how many were forked in the current batch
            int batch_count = 0; 
            // Track the number of finished children and later compare to the batch count
            int num_finished = 0; 
            // Ensure we don't exceed total lines
            for (int b = 0; b < batch_size && current_executable < total_lines; b++)
            {
                // Format the parameter into string so can be passed into exec
                sprintf(str, "%d", parameters[i]);
                //Create fork up to batch size
                pid_t pid = fork();

                //Fork fail case
                if (pid < 0) 
                {
                    perror("Fork failed");
                    exit(1);
                }

                // store the pid in the first element of the array to associate the status codes of each pid
                status_codes[current_executable][0] = pid;

                if (pid == 0) // Child process
                {
                    //Run each exec with the last part of the file path
                    char *executable_name = strrchr(executable_array[current_executable], '/');
                    executable_name++;
                    //Runs exec with each exe 
                    execl(executable_array[current_executable], executable_name, str, NULL);
                }
                // Move to the next executable
                current_executable++; 
                // Increment the number of processes in the current batch
                batch_count++;
            }
            // Wait for all in the current batch to finish
            while (num_finished < batch_count) 
            {
                pid_t result_pid = waitpid(-1, &status, 0); 
                if (result_pid > 0) 
                {
                    if (WIFEXITED(status)) 
                    {
                        int answer = WEXITSTATUS(status);
                        if (answer == 0 || answer == 1) 
                        {
                            update_status_codes(status_codes, result_pid, i + 1, answer);
                        }
                    } 
                    else
                    {
                        update_status_codes(status_codes, result_pid, i + 1, 3);
                    }
                    // Successfully handled one process
                    num_finished++; 
                    // Add one to done executables
                    done_executables++; 
                }
            }
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

    //Get total number of lines from submissions.txt
    int total_lines = get_total_lines();

    //Create executable array with space correlating to amount of exe that will need to be run.
    char *executable_array[total_lines];

    //Populates the executable array
    int num_of_sols = initialize_executable_array(executable_array,total_lines);

    //Gets number of Parameters using argc from main
    int number_of_parameters = get_number_of_parameters(argc); 

    //Inits and populates the paramters array
    int* parameters = initialize_and_populate_parameter_array(number_of_parameters, argv);
   
    //Status Var to hold 0 1 or 2
    int status = -1; 

    //Inits and populates the status codes 2D array. Uses number of parameters and number of solutions for size to ensure dynamic possibility
    int **status_codes = create_status_codes_array(number_of_parameters,num_of_sols);
   
   //Runs the executables and updates the status codes array 
    run_executables(status_codes, parameters, executable_array, number_of_parameters,total_lines,batch_size,status);

    // Write the status codes out into the autograder.out file
    print_status(status_codes, executable_array, total_lines, number_of_parameters, parameters);

    // Free the executable array we used
    free_executables(executable_array, num_of_sols);

    //Free status array
    free_status(status_codes, num_of_sols);

    return 0;
}
