#include "include/utility.h"

char line[128];

void print_status()
{

    // TODO: write the status of each executable file to autograder.out. Your output should align with expected.out
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
    //THIS CURRENTLY RUNS INDEPENDENTLY OF BATCH SIZE WHICH NEEDS TO BE IMPLEMENTED STILL
    for (int i = 0; i < total_lines; i++)
    { // loop through each executable
        for (int j = 0; j < number_of_parameters; j++)
        { // loop through each parameter for each executable
            char str[64];
            sprintf(str, "%d", parameters[j]); // assign parameter to string so its passable using execl
            int pid = fork();
            
            if (pid == 0){//Child Process
                printf("%s",str);
                int process = execl(executable_array[i], executable_array[i], str, NULL); //Think this may be wrong
            }
            else{ //Parent Scenario
                wait(NULL);
            }
            
            
        }
    }

    // TODO: Write the status of each executable file from "submissions.txt" to autograder.out. For testing purposes, you can compare the results with the provided expected.out file
    print_status();

    return 0;
}
