//header files
#include <sys/socket.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <unistd.h>

 

//defining the Port
#define PORT 2000
#define mirror_port 2004

 

 
// Function to check if the given date string is in valid format (YYYY-MM-DD)

int check_valid_date(char* date) {
        // Extract year, month, and day using sscanf

    int year, month, day;
            // If sscanf doesn't parse all three integers, return 0 (invalid format)

    if (sscanf(date, "%d-%d-%d", &year, &month, &day) != 3) {
        return 0;
    }
        // Check if the parsed values fall within valid ranges

    if (year < 1 || year > 9999 || month < 1 || month > 12 || day < 1 || day > 31) {
        return 0;
    }
        // If all checks pass, return 1 (valid date)

    return 1;
}

 

// Function to check if the given file list string is valid
int check_valid_filelist(char* filelist) {
    // Check if the length of the file list string is zero (no files specified)
    if (strlen(filelist) == 0) {
        // If the file list is empty, return 0 (invalid file list)
        return 0;
    }

    // If the file list is not empty, return 1 (valid file list)
    return 1;
}


 

int check_valid_extensions(char* extensions) {
    // Check at least one extension is present in the extensions list
    if (strlen(extensions) == 0) {
        return 0;
    }
    return 1;
}

 

 

// Main function to establish a connection to the server and interact with it
int main(int argc, char const *argv[]) {
    int sockfd;
    struct sockaddr_in serv_addr, mirror_addr; // Server and mirror server address structures

    char buff[1024] = {0}; // Buffer to store user input
    char command[1024] = {0}; // Buffer to store formatted command
    char gf[2048]={0}; // Buffer to store user input (used for mirror redirection)
    int valid_syntax; // Flag to indicate valid syntax

    // Create socket for communication
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n Socket creation error \n");
        return -1;
    }

    // Initialize server address structure
    memset(&serv_addr, '0', sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // Convert server IP address from text to binary form
    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        printf("\n the address is invalid / Address not supported \n");
        return -1;
    }

    // Connect to the server
    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("\nConnection has Failed \n");
        return -1;
    }

    printf("Connected to server.\n");
    printf("Enter a command (or 'quit' to exit):\n");

    // Loop for user interaction
    while (1) {
        valid_syntax = 1;
        memset(buff, 0, sizeof(buff));
        memset(command, 0, sizeof(command));
        fgets(buff, sizeof(buff), stdin); // Read user input
        memcpy(gf, buff, sizeof(buff));
        buff[strcspn(buff, "\n")] = 0; // Remove newline character

        // Parse command
        char* token = strtok(buff, " ");
        if (token == NULL) {
            valid_syntax = 0;
        } else if (strcmp(token, "filesrch") == 0) { // Handle 'filesrch' command
            char* fname = strtok(NULL, " ");
            if (fname == NULL) {
                valid_syntax = 0;
            } else {
                sprintf(command, "filesrch %s", fname);
            }
        } else if (strcmp(token, "getdirf") == 0) {  // Handle 'getdirf' command
            char* size1 = strtok(NULL, " ");
            char* size2 = strtok(NULL, " ");
            if (size1 == NULL || size2 == NULL || atoi(size1) < 0 || atoi(size2) < 0) {
                valid_syntax = 0;
            } else {
                char* unzip = strtok(NULL, " ");
                if (unzip != NULL && strcmp(unzip, "-u") == 0) {
                    sprintf(command, "getdirf %s %s -u", size1, size2);
                } else {
                    sprintf(command, "getdirf %s %s", size1, size2);
                }
            }
        } else if (strcmp(token, "targzf") == 0) {  // Handle 'targzf' command
            char *arguments[100];
            int i = 0;

            // Extract command line arguments and store them in the arguments array
            while ((token = strtok(NULL, " ")) != NULL) {
                arguments[i] = token;
                i++;
            }
            
            snprintf(command, sizeof(command), "targzf");
            for (int j = 0; j < i; j++) {
                snprintf(command + strlen(command), sizeof(command) - strlen(command), " %s", arguments[j]);
            }
        } else if (strcmp(token, "fgets") == 0) {  // Handle 'fgets' command
            gf[strcspn(gf, "\n")]='\0';
            sprintf(command, gf);
            printf("%s", command);
        } else if (strcmp(token, "tarfgetz") == 0) { // Handle 'tarfgetz' command
            gf[strcspn(gf, "\n")]='\0';
            sprintf(command, gf);
            printf("%s", command);
        } else if (strcmp(token, "quit") == 0) {
            sprintf(command, "quit");
        } else {
            valid_syntax = 0;
        }

        // Check for valid syntax
        if (!valid_syntax) {
            printf("Invalid syntax. Please try again.\n");
            continue;
        }

        // Send command to server
        send(sockfd, command, strlen(command), 0);

        // Handle response from server
        char resp[1024]={0};
        int valueread=read(sockfd, resp, sizeof(resp)); // Redirect to mirror
        printf("%s\n", resp);
        resp[strcspn(resp, "\n")] = '\0';
        
        // Check if the response indicates a mirror server redirection
        if (strcmp(resp, "2004") == 0) {
            close(sockfd);   // Close the current server connection

            // Create a new socket for the mirror server
            sockfd = socket(AF_INET, SOCK_STREAM, 0);
            if (sockfd == -1) {
                perror("socket");
                exit(EXIT_FAILURE);
            }

            // Initialize mirror server address structure
            memset(&mirror_addr, '\0', sizeof(mirror_addr));
            mirror_addr.sin_family = AF_INET;
            mirror_addr.sin_port = htons(mirror_port);
            mirror_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

            // Connect to the mirror server
            if (connect(sockfd, (struct sockaddr *)&mirror_addr, sizeof(mirror_addr)) == -1) {
                perror("connect");
                exit(EXIT_FAILURE);
            }
        } else {
            printf("%s\n", resp);
        }

        // Check if the user wants to quit
        if (strcmp(command, "quit") == 0) {
            break;
        }

        printf("Enter a command (or 'quit' to exit):\n");
    }

    close(sockfd);
    printf("Connection is closed.\n");

    return 0;
}
