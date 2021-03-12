#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <ctype.h>
#include <fcntl.h> 
#include <assert.h>

int wrap(unsigned width, int input_fd, int output_fd) {

    int error = 0; //variable to check if wordLength > width
    int bytes;
    int size = 128*sizeof(char); //buffer size
    char *buffer = malloc(size);
    char *word = NULL; //stores partial word from the end of buffer
    int start = 0;  //starting index of a word
    int end = 0;    //ending index of a word
    int wordLength = 0;
    int characters = 0; //number of characters in a line
    int newLine = 0; //variable to indicate if a newline needs to be inserted
    while ((bytes = read(input_fd, buffer, size)) > 0) {
        start = 0;
        end = 0;
        //second case of new line: last element of buffer was \n
        if (newLine == 2) {
            if (buffer[start] == '\n') {
                characters = 0;
                write(output_fd, "\n", 1);
                write(output_fd, "\n", 1);
                newLine = 0;
            }
        }
        
        //if we stashed partial word and the first element of buffer is WS
        if (isspace(buffer[start]) && word != NULL) {
            if (wordLength > width) {
                write(output_fd, "\n", 1);
                write(output_fd, word, wordLength);
                write(output_fd, "\n", 1);
                error = 1;
            }
            else if (characters + wordLength <= width) {
                characters += wordLength;
                write(output_fd, word, wordLength);
                characters++;
                write(output_fd, " ", 1);
            }
            else { //characters + wordLength > width so put word on newline
                characters = 0;
                write(output_fd, "\n", 1);
                characters += wordLength;
                write(output_fd, word, wordLength);
                characters++;
                write(output_fd, " ", 1);
            }
            free(word);
            word = NULL;
            wordLength = 0;
            start = end + 1;
            end = start;
        }
        //if we stashed partial word and the first element of buffer is not WS
        else if (!isspace(buffer[start]) && word != NULL) {
            while (!isspace(buffer[end])) {
                end++;
                if (end == bytes) {
                    break;
                }
                //check for 2 newlines in a row
                if (end < bytes-1) {
                    if (buffer[end] == '\n' && buffer[end+1] == '\n') {
                        newLine = 1;
                    }
                }
                else {
                    if (buffer[end] == '\n') {
                        newLine = 2;
                    }
                }
            }
            int temp = end - start; //length of the rest of the word
            char* restOfWord = malloc((temp+1)*sizeof(char));
            restOfWord[temp] = '\0';
            memcpy(restOfWord, &buffer[start], temp);
            wordLength += temp;
            char* p = realloc(word, (wordLength+2)*sizeof(char));
            word = p;
            strcat(word, restOfWord); //concatenate partial word with rest of word
            free(restOfWord);
            if (end < bytes && isspace(buffer[end])) {
                if (wordLength > width) {
                    write(output_fd, "\n", 1);
                    write(output_fd, word, wordLength);
                    write(output_fd, "\n", 1);
                    error = 1;
                }
                else if (characters + wordLength <= width) {
                    characters += wordLength;
                    write(output_fd, word, wordLength);
                    characters++;
                    write(output_fd, " ", 1);
                }
                else {
                    characters = 0;
                    write(output_fd, "\n", 1);
                    characters += wordLength;
                    write(output_fd, word, wordLength);
                    characters++;
                    write(output_fd, " ", 1);
                }
                free(word);
                word = NULL;
                start = end + 1;
                end = start;
            }
            else if (end == bytes) {
                continue;
            }
        }

        // if (buffer[0] == '\n' && buffer[1] == '\n') {
        //     characters = 0;
        //     write(output_fd, "\n", 1);
        //     write(output_fd, "\n", 1);
        //     newLine = 0;
        // }
        
        while (end != bytes) {
            while (!isspace(buffer[end])) {
                end++;
                if (end == bytes) {
                    break;
                }
                //check for 2 newlines in a row
                if (end < bytes-1) {
                    if (buffer[end] == '\n' && buffer[end+1] == '\n') {
                        newLine = 1;
                    }
                }
                else {
                    if (buffer[end] == '\n') {
                        newLine = 2;
                    }
                }
            }
            
            wordLength = end - start;
            
            if (wordLength > width) {
                write(output_fd, "\n", 1);
                write(output_fd, &buffer[start], wordLength);
                // write(output_fd, "\n", 1);
                error = 1;
            }
            //end pointer reached end of buffer so store partial word
            else if (end == size) {
                word = malloc((wordLength+1)*sizeof(char));
                memcpy(word, &buffer[start], wordLength);
                word[wordLength] = '\0';
                break;
            }
            else if (start != end) {
                if (characters + wordLength <= width) {
                    characters += wordLength;
                    write(output_fd, &buffer[start], wordLength);
                    characters++;
                    write(output_fd, " ", 1);
                }
                else {
                    characters = 0;
                    write(output_fd, "\n", 1);
                    characters += wordLength;
                    write(output_fd, &buffer[start], wordLength);
                    characters++;
                    write(output_fd, " ", 1);
                }
            }
            //print 2 new lines for case 1
            if (newLine == 1) {
                characters = 0;
                write(output_fd, "\n", 1);
                write(output_fd, "\n", 1);
                newLine = 0;
            }
            start = end + 1;
            end = start;
            if (end == size) {
                break;
            }
        }

    }
    
    free(buffer);
    if (error == 1) return 1;
    return 0;
}

int is_directory(char *name) {
	struct stat data;
	int err = stat(name, &data);
	// should confirm err == 0
	if (err) {
		perror("Error.");
		exit(EXIT_FAILURE);
	}
	if (S_ISDIR(data.st_mode)) {
		// S_ISDIR macro is true if the st_mode says the file is a directory
		// S_ISREG macro is true if the st_mode says the file is a regular file
		return 1;
	}
	return 0;
}

int process_de(char* dir_name, char* de, unsigned page_width) {
	// page width and filename given
	char file_name[100] = "";
	snprintf(file_name, sizeof(file_name), "%s/%s", dir_name, de);
	int fd = open(file_name, O_RDONLY);
	if (fd < 0) {
		perror("Error: File does not exist.");
		exit(EXIT_FAILURE);
	}
	char de_wrap[100] = "";
	snprintf(de_wrap, sizeof(de_wrap), "%s/wrap.%s", dir_name, de);
	int fd_wrap = open(de_wrap, O_WRONLY|O_TRUNC|O_CREAT, S_IRWXU);
	wrap(page_width, fd, fd_wrap);
	close(fd_wrap);
	close(fd);
	return EXIT_SUCCESS;
}

int process_directory(char* dir_name, unsigned page_width) {
	DIR* directory_p = opendir(dir_name);
	struct dirent* directory_entry_p;
	while ((directory_entry_p = readdir(directory_p))) {
		//puts directory_entry -> directory_name
		char* de= directory_entry_p->d_name;
		//check for . and ..
		if (!((strcmp(de, ".") == 0) || (strcmp(de, "..") == 0))) {
			//check that file does not begin with "wrap"
			if (strlen(de) > 3) {
				char sub[4] = "";
				strncpy(sub, de, 4);
				if (!(strcmp(sub,"wrap") == 0)) {
					process_de(dir_name, de, page_width);
				}
			}
			else {
				process_de(dir_name, de, page_width);
			}
		}
	}
	if (closedir(directory_p) == -1) {
		perror("Error: Unable to close directory.");
		exit(EXIT_FAILURE);
	}
	return EXIT_SUCCESS;
}

int process_file(char* file_name, unsigned page_width) {
	// page width and filename given
	int fd = open(file_name, O_RDONLY);
	if (fd < 0) {
		perror("Error: File does not exist.");
		exit(EXIT_FAILURE);
	}
	wrap(page_width, fd, 1);
	close(fd);
	return EXIT_SUCCESS;
}

int process_standard_input(unsigned page_width) {
	//page width was given but no filename- read from standard input
	return wrap(page_width, 0, 1);
}

int check_input(int argc, char** argv) {
	unsigned page_width = atoi(argv[1]);
	// check page width is valid
	if (page_width <= 0) {
			perror("Error: Page width invalid.");
			exit(EXIT_FAILURE);
	}
	if (argc < 3) {
		return process_standard_input(page_width);
	}
	else {
		char* name = argv[2];
		// check if directory or file
		if (is_directory(name) == 1) {
			return process_directory(name, page_width);
		}
		else {
			return process_file(name, page_width);
		}
	}
}

int process_arguments(int argc, char** argv) {
	// Check number of arguments passed
	if (argc < 2){
		// no arguments were passed
		perror("Error: No arguments passed.");
		exit(EXIT_FAILURE);
	}
	return check_input(argc, argv);
}

int main(int argc, char **argv) {
	return process_arguments(argc, argv);
}

