#include "ZQ_common_conf.h"
#include "getopt.h"
#include <iostream>
#include <sstream>

#define MAX_LOG_LENGTH		4096

void help() {
	std::cerr << "Usage: LogPage <options> [arg]\n\n"
              << "Options:\n"
              << "-h                show this help\n"
              << "-f <filename>     full path to the log file\n"
              << "-s <size>         size for each page in bytes\n"
              << "-p <page>         page number to display\n"
              << std::endl;
}

unsigned getPageCount(int64 size, const std::string& file) {
	if(size <= 0) {
		return 1;
    }
    unsigned count = 1;
    FILE* handle = fopen(file.c_str(), "r");

	if(handle) {
        fseek(handle, 0, SEEK_END);
        long pos = ftell(handle);

        fclose(handle);

		if(pos % size == 0) {
			count = pos/size;
        }
		else {
			count = pos/size + 1;
        }
	}

	return count;
}

void displayPage(const std::string& file, int64 size, unsigned page) {
	if(size <= 0) {
		return;
    }
    
    FILE* handle = fopen(file.c_str(), "r");
	if(!handle) {
		fprintf(stderr, "failed to open file [%s]\n", file.c_str());
		return;
	}
	
    fseek(handle, 0, SEEK_END);
    long pos = ftell(handle);

    int64 startPoint = (page-1)*size;
    if(startPoint >= pos) {
        fclose(handle);
        return;
    }
		
    char buff[MAX_LOG_LENGTH+1];
    memset(buff, '\0', MAX_LOG_LENGTH);

    fseek(handle, startPoint, SEEK_SET);
    
    int64 left = size;
    int64 total = 0; 

read:
    if(!feof(handle) && !ferror(handle)) {
        size_t read = fread(buff, 1, MAX_LOG_LENGTH, handle);

        left -= read;
        total += read;

        std::cout << buff;
        if(left > 0) {
            goto read; 
        }
        else {
            buff[read] = '\0';
        }
    } 
    
    std::cout << std::endl;

    fclose(handle);
}

int main(int argc, char* argv[]) {
    if(argc < 2) {
        help();
        return (-1);
    } 

    std::string file;
    int64 size = 0;
    unsigned page = 0;
    
    int ch = 0;
    while((ch = getopt(argc, argv, "hf:c:s:p:t:")) != EOF) {
        if(ch == 'h') {
            help();
            return (0);
        }			
        else if(ch == 'f') {
            file = optarg;
        }
        else if(ch == 's' || ch == 'c') {
            std::istringstream iss(optarg);
            iss >> size;
        }
        else if(ch == 'p') {
            std::istringstream iss(optarg);
            iss >> page;
        }
        else if(ch == 't') {
            
        }
        else {
            std::cerr << "invalid option" <<  std::endl;
            help();
            return (0);
        }
	}

    if(!file.length() || !size) {
        fprintf(stderr, "invalid parameters\n");
        help();
        return (-1);
    }

    if(page && size && file.length()) {
        displayPage(file, size, page);
        return (0);
    }
    if(!page && size && file.length()) {
        std::cout << getPageCount(size, file); 
        return (0);
    }    
    
    return (0);
}

