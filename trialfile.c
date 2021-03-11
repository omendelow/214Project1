#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <fcntl.h>

int process_directory(char* dir_name, int* page_width, char* buf) {
	printf("hello!\n");
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

int process_file(char* file_name, int* page_width, char* buf) {
	// page width and filename given
	int fd = open(file_name, O_RDONLY);
	if (fd < 0) {
		perror("Error: File does not exist.");
		exit(EXIT_FAILURE);
	}
	size_t bytes_read;
	bytes_read = read(fd, buf, 256);
	if (bytes_read < 0) {
		perror("Error: error reading file.");
		exit(EXIT_FAILURE);
	}
	while (bytes_read > 0) {
		//run wrap algorithm on buffer
		write(1,buf,strlen(buf));
		memset(&buf[0], 0, sizeof(buf)); //clear buf
		bytes_read = read(fd, buf, 256);
	}
	close(fd);
	return 0;
}

int process_standard_input(int* page_width, char* buf) {
	//page width was given but no filename- read from standard input
	int bytes_read;
	bytes_read = read(0,buf,256);
	
	if (bytes_read < 0) {
		perror("Error: error reading file.");
		exit(EXIT_FAILURE);
	}
	while (bytes_read > 0) {
		//run wrap algorithm on buffer
		write(1,buf,strlen(buf));
		printf("\n");
		memset(&buf[0], 0, sizeof(buf)); //clear buf
		if (bytes_read < 256) {bytes_read = -1;}
		else {bytes_read = read(0,buf,256);} 
	}
	return 0;	
}

int check_input(int argc, char** argv) {
	int page_width = atoi(argv[1]);
	char buf[256];
	// check page width is valid
	if (page_width <= 0) {
			perror("Error: Page width invalid.");
			exit(EXIT_FAILURE);
	}
	if (argc < 3) {
		return process_standard_input(&page_width, buf);
	}
	else {
		char* name = argv[2];
		// check if directory or file
		if (is_directory(name) == 1) {
			return process_directory(name, &page_width, buf);
		}
		else {
			return process_file(name, &page_width, buf);
		}
	}
	return 0;
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

