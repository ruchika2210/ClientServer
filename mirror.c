
// header files
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include <time.h>
#include <dirent.h>

 

 //defining the port , buffer size and macro

#define _XOPEN_SOURCE 500
#define BUFSIZE 1024
#define MIRROR_PORT 2004    

 
 // Function to create a tar.gz archive


void create_targzf(const char *sourceDir, const char *outputPath, const char *extensions[], int numExtensions) {
    printf("\n I have reached inside create_targzf function!!!\n");
        // Define a buffer to store the command

   char cmd[BUFSIZE];
       // Create the initial part of the command to find files in the source directory

    snprintf(cmd, sizeof(cmd), "find %s -type f ", sourceDir);

 

    // Add extension filters to the command
    //for (int i = 0; i < numExtensions && strcmp(extensions[i], "-u") != 0; ++i) {

    // Append the last extension filter and remove the last "-o" and space

        snprintf(cmd + strlen(cmd), sizeof(cmd) - strlen(cmd), "-name '*.%s' -o ", extensions[0]);
    //}
    cmd[strlen(cmd) - 4] = '\0';  // Remove the last "-o" and space

        // Append the rest of the command to create the tar.gz archive


    snprintf(cmd + strlen(cmd), sizeof(cmd) - strlen(cmd), "| xargs tar -czf %s -C %s",
             outputPath, sourceDir);

 // Execute the tar command
    system(cmd);
}

 



// Function to create a tar.gz archive based on date ranges
 

int createArchive(const char *date1, const char *date2, int unzip) {
    char command[1024];
    char tempFileName[] = "./temp.tar.gz";
    char sourcePathName[]="~";

 
// Construct the tar command based on the given arguments
    snprintf(command, sizeof(command),
             "find %s -type f -newermt %s ! -newermt %s -print0 | tar --null -czf %s --files-from=-",sourcePathName,date1, date2, tempFileName);

 // Execute the tar command
    int result = system(command);

 if (result == 0) {
        printf("Archive created successfully: %s\n", tempFileName);
        if (unzip) {
            char unzipCommand[512];
            snprintf(unzipCommand, sizeof(unzipCommand), "tar xzf %s -C ./", tempFileName);
            system(unzipCommand);
            printf("Archive extracted in the current directory.\n");
        }
        return 0; // Success
    } else {
        printf("Error creating archive.\n");
        return 1; // Error
    }
}

 // Function to search for a file in a directory



/*This function searches for a specified filename in the given directory and its subdirectories. If the file is found, it retrieves its size, modification date, and name, and stores these 
details in the provided output variables. The function uses recursive directory traversal to search through subdirectories as well.*/
void searchFile(const char *dirPath, const char *filename, char *resultName, int *resultSize, int *resultDate) {
        // Open the directory

    DIR *dir = opendir(dirPath);
    if (dir == NULL) {
        perror("Error opening directory");
        return;
    }

 

    struct dirent *entry;
        // Iterate through the directory entries

    while ((entry = readdir(dir)) != NULL) {
                // Check if the entry is a regular file and its name matches the desired filename

        if (entry->d_type == DT_REG && strcmp(entry->d_name, filename) == 0) {
            char filePath[PATH_MAX];
            snprintf(filePath, PATH_MAX, "%s/%s", dirPath, filename);

 

            struct stat st;
        // Get file information (size and modification date)

            if (stat(filePath, &st) == 0) {
                *resultSize = st.st_size;
                *resultDate = st.st_mtime;
                strcpy(resultName, filename);
            }
//close dir
            closedir(dir);
            return;
        }
    }
    //close dir
    closedir(dir);

 

    // If the file is not found in this directory, try subdirectories
    dir = opendir(dirPath);
    // If the file is not found in the current directory, try searching in subdirectories

    if (dir != NULL) {
        while ((entry = readdir(dir)) != NULL) {
                    // Check if the entry is a subdirectory and not "." or ".."

            if (entry->d_type == DT_DIR && strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
                            // Create the path for the subdirectory

                char newPath[PATH_MAX];
                snprintf(newPath, PATH_MAX, "%s/%s", dirPath, entry->d_name);
                            // Recursively search the subdirectory for the file

                searchFile(newPath, filename, resultName, resultSize, resultDate);
                            // If the file is found in the subdirectory, stop searching and return

                if (*resultName != '\0') {
                    closedir(dir);
                    return;
                }
            }
        }
        closedir(dir);
    }
}

 
// Create a tarball (compressed archive) of the specified source directory

void create_tarball_tarex(const char *sourceDir, const char *outputPath) {
        // Create a command buffer to store the tar command

    char cmd[256];
        // Construct the tar command to compress the source directory

    snprintf(cmd, sizeof(cmd), "tar -czf %s -C %s .", outputPath, sourceDir);
        // Execute the constructed tar command to create the tarball

    system(cmd);
}



 // Process files within a specified size range, optionally unzipping them


void processFiles_tarex(long long size1, long long size2, int unzip) {
        // Validate the size range

    if (size1 < 0 || size2 < 0 || size1 > size2) {
        printf("Invalid size range.\n");
        return;
    }

 

    
    // Get user's home directory
    const char *homeDir = getenv("HOME");
    if (!homeDir) {
        printf("Unable to get user's home directory.\n");
        return -1;
    }

 

    // Compress files in the size range
    const char * extract_path="temp.tar.gz";
    create_tarball_tarex(homeDir, extract_path);


    // Unzip the tarball if the 'unzip' flag is set

    if (unzip) {
        printf("Unzipping temp.tar.gz...\n");
        char unzipCmd[256];
        snprintf(unzipCmd, sizeof(unzipCmd), "tar -xzf %s", extract_path);
        int unzipStatus = system(unzipCmd);
        if (unzipStatus == 0) {
            printf("Unzipped successfully.\n");
        } else {
            printf("Error during unzip.\n");
        }
    }

     // Print a message indicating whether the tarball was created and unzipped


    printf("temp.tar.gz created and %s.\n", (unzip ? "unzipped" : "not unzipped"));
}


// Create a tarball containing specific files from a directory
void create_tarball(const char *sourceDir, const char *outputPath, const char *files[], int numFiles) {
    // Prepare the base tar command
    char cmd[1024];
    snprintf(cmd, sizeof(cmd), "tar -czf %s -C %s", outputPath, sourceDir);

    // Append each file to the tar command
    for (int i = 0; i < numFiles; ++i) {
        snprintf(cmd + strlen(cmd), sizeof(cmd) - strlen(cmd), " %s", files[i]);
    }

    // Execute the tar command to create the tarball
    system(cmd);
}


 

// Process specified files and create a tarball
void processFiles(const char *files[]) {
    // Get the user's home directory
    const char *homeDir = getenv("HOME");
    if (!homeDir) {
        printf("Unable to get user's home directory.\n");
        return;
    }

    // Count the number of files to process
    int numFiles = 0;
    for (int i = 0; i < 4 && files[i] != NULL; ++i) {
        numFiles++;
    }

    // Initialize variables for file processing
    int found = 0;
    const char *foundFiles[4] = {NULL, NULL, NULL, NULL};
    int numFoundFiles = 0;

    // Check each specified file's existence
    for (int i = 0; i < numFiles; ++i) {
        char filePath[1024];
        snprintf(filePath, sizeof(filePath), "%s/%s", homeDir, files[i]);

        // Check if the file exists
        struct stat fileInfo;
        if (stat(filePath, &fileInfo) == 0) {
            found = 1;
            foundFiles[numFoundFiles++] = files[i];
        }
    }

    // Create a tarball if files are found
    if (found) {
        create_tarball(homeDir, "temp.tar.gz", foundFiles, numFoundFiles);
        printf("temp.tar.gz created.\n");
    } else {
        printf("No file found.\n");
    }
}


 

 
// Search for a file in the specified directory and its subdirectories
void search_directory(char* path, char* filename, char* command) {
    DIR* dir;   // Directory pointer
    struct dirent* entry;   // Directory entry
    struct stat st;   // File status structure

    // Open the directory
    if ((dir = opendir(path)) == NULL) {
        printf("Unable to open directory %s\n", path);
        return;
    }

    // Traverse the directory tree
    while ((entry = readdir(dir)) != NULL) {
        // Skip "." and ".." directories
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        // Construct the full path to the file or directory
        char full_path[1024];
        snprintf(full_path, sizeof(full_path), "%s/%s", path, entry->d_name);

        // Check if the entry is a file or a directory
        if (lstat(full_path, &st) == 0) {
            if (S_ISDIR(st.st_mode)) {
                // Recurse into the subdirectory
                search_directory(full_path, filename, command);
            } else if (S_ISREG(st.st_mode)) {
                // Check if the file has the desired name
                if (strcmp(entry->d_name, filename) == 0) {
                    time_t c_time = st.st_ctime;
                    sprintf(command, "%s, %ld, %s\n", full_path, st.st_size, ctime(&c_time));
                    closedir(dir);
                    return;
                }
            }
        } else {
            printf("Unable to get information about file %s\n", full_path);
        }
    }

    // Close the directory
    closedir(dir);
}



// Check if the given date is in valid format (YYYY-MM-DD)
int check_valid_date(char* date) {
    int year, month, day;

    // Attempt to parse the date using the specified format
    if (sscanf(date, "%d-%d-%d", &year, &month, &day) != 3) {
        return 0; // Parsing failed, invalid format
    }

    // Check if year, month, and day values are within valid ranges
    if (year < 1 || year > 9999 || month < 1 || month > 12 || day < 1 || day > 31) {
        return 0; // Values out of valid ranges
    }

    return 1; // Date is valid
}
 

 
// Check if the file list contains at least one file
int check_valid_filelist(char* filelist) {
    // Check if the length of the file list string is zero
    if (strlen(filelist) == 0) {
        return 0; // No files in the list, invalid
    }
    return 1; // File list contains at least one file, valid
}


 // Check if the extensions list contains at least one extension
int check_valid_extensions(char* extensions) {
    // Check if the length of the extensions string is zero
    if (strlen(extensions) == 0) {
        return 0; // No extensions in the list, invalid
    }
    return 1; // Extensions list contains at least one extension, valid
}


 

void processclient(int sockfd) {
    // Buffer for receiving client data
    char buff[1024] = {0};
    // Buffer for constructing server responses
    char command[1024] = {0};
    // Another buffer
    char gf[1024]={0};

 
    // Loop to continuously handle client requests

    while (1) {
                // Clear the buffers

        memset(buff, 0, sizeof(buff));
        memset(command, 0, sizeof(command));
        memcpy(gf, buff, sizeof(buff));

 
        // Read client data into the buffer

        int valread = read(sockfd, buff, sizeof(buff));
        buff[valread] = '\0';

 

        // Parse command from the received data
        char* token = strtok(buff, " ");

        if (token == NULL) {
            sprintf(command, "The syntax is Invalid. Please try again.\n");
        } else if (strcmp(token, "filesrch") == 0) {  // execcution of filesrch command
            char* filename = strtok(NULL, " ");
            if (filename == NULL) {
                sprintf(command, "The syntax is Invalid. Please try again.\n");
            } else {
                     char foundFilename[PATH_MAX] = "";
                    int fileSize = -1;
                    int fileDate = -1;

             searchFile(getenv("HOME"), filename, foundFilename, &fileSize, &fileDate);

        if (foundFilename[0] != '\0') {
                        printf("Filename: %s\nSize: %d bytes\nCreation Date: %d\n", foundFilename, fileSize, fileDate);
                    } else {
                        printf("File not found.\n");
                    }

  }
        } else if (strcmp(token, "targzf") == 0) { //executiion of sgetfiles command

            printf("%s","I am in server for targzf\n");
            char* extensions[5];
             int unzip = 0;
             int numExtensions=0 ;
             extensions[0]= strtok(NULL, " ");
            for(int i=1;i<5 && extensions[i] != NULL;i++)
            {

                 extensions[i]= strtok(NULL, " ");
                 printf("%s\n", extensions[i]);
                    numExtensions++;

            }

                printf("I reached create_targzf\n");
                            
                const char *homeDir = getenv("HOME");
            if (!homeDir) {
                printf("Unable to get user's home directory.\n");
                return 1;
            }

            const char *outputPath = "temp.tar.gz";
             printf("I reached create_targzf\n");
            create_targzf(homeDir, outputPath, extensions, numExtensions);

 
        if (unzip) {
                char unzipCmd[BUFSIZE];
                snprintf(unzipCmd, sizeof(unzipCmd), "tar -xzf %s -C ./", outputPath);
                int unzipStatus = system(unzipCmd);
                if (unzipStatus == 0) {
                    //printf("Unzipped %s successfully.\n", outputPath);
                    sprintf(command, "Files retrieved and unzipped successfully.\n");

                } else {
                    //printf("Error during unzip.\n");
                    sprintf(command, "Error during unzip.\n");

                }
            }else{
                    //printf("Files retrieved and not unzipped.\n");
                    sprintf(command, "Files retrieved and not unzipped.\n");


 }

 
} else if (strcmp(token, "getdirf") == 0) {  // execution of dgetfiles command
            char* date1 = strtok(NULL, " ");
            char* date2 = strtok(NULL, " ");
            char* unzip_flag = strtok(NULL, " ");

 

            if (date1 == NULL || date2 == NULL) {
                sprintf(command, "Invalid syntax. Please try again.\n");
            } else {


 

                // Check if the unzip flag is provided
                if (unzip_flag != NULL && strcmp(unzip_flag, "-u") == 0) {
                    // Unzip the file in the current directory
                     createArchive(date1,date2, 1); // Unzip flag set to 1


 

                    sprintf(command, "Files retrieved and unzipped successfully.\n");
                } else {
                    createArchive(date1, date2, 0);
                    sprintf(command, "Files retrieved successfully. Use the -u flag to unzip.\n");
                }
            }
        }else if (strcmp(token, "fgets") == 0) { //execution of getfiles command


                const char *files[4] = {NULL, NULL, NULL, NULL};

 

                for (int i = 1;  i <= 4; ++i) {
                    files[i - 1] =strtok(NULL, " ");
                }

 

                 processFiles(files);

 

 

 

            } else if (strcmp(token, "tarfgetz") == 0) { //execution of gettargz command


 

                int size1=atoi(strtok(NULL, " "));
                int size2=atoi(strtok(NULL, " "));

 

                const char * spc =strtok(NULL, " ");

                if (size1 < 0 || size2 < 0 || size1 > size2) {
                    printf("Invalid size range.\n");
                    return -1;
                }

 

            
            int unzip = 0;

 

            if (spc !=NULL && strcmp(spc, "-u") == 0) {
                unzip = 1;
            }
            //printf("%d\n", unzip);
            processFiles_tarex(size1, size2, unzip);

 

    }else if (strcmp(token, "quit") == 0) {
            sprintf(command, "Goodbye.\n");
            break;
        } else {
            sprintf(command, "The syntax is invalid. Please try again.\n");
        }

 

        // Send response to client
        send(sockfd, command, strlen(command), 0);
    }

 

    close(sockfd);
    exit(0);
}

 

 

int main(int argc, char const *argv[]) {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

 

    // Create socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

 

    // Attach socket to the port 8080
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(MIRROR_PORT);

     // Bind the socket to the specified address


    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

     // Start listening for incoming connections


    if (listen(server_fd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

     // Accept and handle incoming connections


    while (1) {
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
            perror("accept");
            exit(EXIT_FAILURE);
        }

 
        // Fork a child process to handle the client

        printf("New client is connected. Forking child process...\n");


    }

 

    return 0;
}