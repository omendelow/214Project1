#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <fcntl.h> 
#include <assert.h>


int wrap(unsigned width, int input_fd, int output_fd);


int main(int argc, char* argv[]) {

    if (argc < 2 || argc > 3) {
        perror("Wrong number of arguments!");
        exit(EXIT_FAILURE);
    }
    int width = atoi(argv[1]);
    assert(width > 0);
    // if (width < 1) {
    //     perror("Width must be positive!");
    //     exit(EXIT_FAILURE);
    // }
    if (argc == 2) {
        int error = wrap(width, 0, 1);
        if (error == 1) {
            exit(EXIT_FAILURE);
        }
    }
    else if (argc == 3) {
        int fd = open(argv[2], O_RDONLY);
        if (fd == -1) {
            perror(argv[2]);
            exit(EXIT_FAILURE);
        }
        int error = wrap(width, fd, 1);
        if (error == 1) {
            exit(EXIT_FAILURE);
        }
    }
        
    return EXIT_SUCCESS;
}

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
