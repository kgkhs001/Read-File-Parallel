// Modified by Craig Wills from:
// Copyright (c) Michael Still, released under the terms of the GNU GPL
//Further Modified by Krishna Garg for the purposes of project2

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include<sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <ctype.h>
//MAX BYTES TO CHUNK
#define MAX_CHUNK 8192
int isNumber(char * number);
int ret_ascii(char input);
int ret_upper(char input);
int ret_lower(char input);
int ret_digit(char input);
int ret_space(char input);


int main(int argc, char *argv[]){
    


    int fd; //this is the file to open
    char *pchFile; //this is the pointer to contents of the opened file
    struct stat sb; //-> this is what the fstat function returns to
    int chunk_size = 1024;
    //all the things we are trying to find
    int num_ascii;
    int num_upper;
    int num_lower;
    int num_digit;
    int num_space;
    char par_char[20];
    char par_dig[20];
    //PART 2 VARIABLES
    //starting prepwork
    if(argc == 3 && strcmp(argv[2], "mmap") != 0){
        strcpy(par_char, argv[2]);
        strncpy(par_dig, &par_char[1], strlen(argv[2]) - 1);
    }
    
    //end prepwork
    //END OF PART 2 VARIABLES
    

    //this will check if the input is a number 
    if(argc == 3 && isNumber(argv[2]) == 0){
        if(atoi(argv[2]) > MAX_CHUNK){
            fprintf(stderr, "You can enter a maximum of 8192 for byte read size\n");
            exit(1);
        }
        else{
            chunk_size = atoi(argv[2]);
        }
    }

    if(argc < 2) {
	fprintf(stderr, "Usage: %s <input>\n", argv[0]);
	exit(1);
    }

    /* Map the input file into memory */
    if ((fd = open (argv[1], O_RDONLY)) < 0) { //O_RDONLY creates a read only file
	perror("Could not open file");
	exit (1);
    }

    if(fstat(fd, &sb) < 0){
	perror("Could not stat file to obtain its size");
	exit(1);
    }

    //if the user enters mmap then we use mmap
    if(argc == 3 && strcmp(argv[2], "mmap") == 0){
        //printf("MAPPING\n");
        if ((pchFile = (char *) mmap (NULL, sb.st_size, PROT_READ, MAP_SHARED, fd, 0)) == (char *) -1)	{ //pchFile is the pointer to the entire contents of the file
	    perror("Could not mmap file\n");
	    exit (1);
        }

        for(int i = 0; i < sb.st_size; i++){
            num_ascii += ret_ascii(pchFile[i]);
            num_upper += ret_upper(pchFile[i]);
            num_lower += ret_lower(pchFile[i]);
            num_digit += ret_digit(pchFile[i]);
            num_space += ret_space(pchFile[i]);
        }

        //unmap everything
        if(munmap(pchFile, sb.st_size) < 0){
        perror("Could not unmap memory");
        exit(1);
        }
        //ending sequence
        close(fd);
        printf("ascii = %d, upper-case = %d, lower-case = %d, digits = %d, spaces = %d ", num_ascii, num_upper, num_lower, num_digit, num_space);
        printf("out of %ld bytes\n", sb.st_size);
        exit(0);
    }//end of if the user enters mmap


    if(argc == 3 && par_char[0] == 'p' && isNumber(par_dig) == 0){
        int num_children = atoi(par_dig);
        if(num_children > 16){
            fprintf(stderr, "Maximum of 16 child processes can be created\n");
            exit(1);
        }
        int r1 = sb.st_size % num_children;
        int pid;
        int start_here = 0;
        int loop_count = 0;
        chunk_size = sb.st_size / num_children;
        
        //map the file to memory
        if ((pchFile = (char *) mmap (NULL, sb.st_size, PROT_READ, MAP_SHARED, fd, 0)) == (char *) -1)	{ //pchFile is the pointer to the entire contents of the file
	    perror("Could not mmap file\n");
	    exit (1);
        }

        for(int i = 0; i < num_children; i++){
            num_ascii = 0;
            num_upper = 0;
            num_lower = 0;
            num_digit = 0;
            num_space = 0;

            if(i < sb.st_size){
                if(num_children > sb.st_size){
                    chunk_size = 1;
                }
                start_here = (i * chunk_size);

                if(r1 > 0 && (sb.st_size - (start_here + chunk_size)) == r1){
                    chunk_size = chunk_size + r1;
                }

                //create the child process
                if((pid = fork()) < 0){
                    perror("Problem while forking\n");
                    exit(1);
                }

                //if you are successful then do all this stuff
                else if(pid == 0){
                        for(int k = start_here; k < (start_here + chunk_size); k++){
                            num_ascii += ret_ascii(pchFile[k]);
                            num_upper += ret_upper(pchFile[k]);
                            num_lower += ret_lower(pchFile[k]);
                            num_digit += ret_digit(pchFile[k]);
                            num_space += ret_space(pchFile[k]);
                        }//end of for loop
                        
                        printf("ascii = %d, upper-case = %d, lower-case = %d, digits = %d, spaces = %d ", num_ascii, num_upper, num_lower, num_digit, num_space);
                        printf("out of %d bytes\n", chunk_size);
                        exit(0);
                    }

                else{
                    //you are now in the parent
                    wait(0);
                } 
                //end of  
            }
            else{
                if((pid = fork()) < 0){
                    perror("Problem while forking\n");
                    exit(1);
                }

                else if(pid == 0){
                    //in the child process
                    printf("ascii = %d, upper-case = %d, lower-case = %d, digits = %d, spaces = %d ", num_ascii, num_upper, num_lower, num_digit, num_space);
                    printf("out of %d bytes\n", 0);
                    exit(0);   
                }

                else{
                    //parent process
                    wait(0);
                }
            }
        }

        if(munmap(pchFile, sb.st_size) < 0){
            perror("Could not unmap memory");
            exit(1);
        }
        //ending sequence
        close(fd);
        exit(0);
    }
    

    //otherwise we're going to be using the read function
    else{
        //printf("READING\n");
        char buf[chunk_size];
        int cnt;
        while((cnt = read(fd, buf, chunk_size)) > 0){
            //make sure you actually read the file
            if(cnt < 0){
                perror("There was an error reading the file\n");
                exit(1);
            }
            
            //then read through the chunk
            for(int i = 0; i < cnt; i++){
                num_ascii += ret_ascii(buf[i]);
                num_upper += ret_upper(buf[i]);
                num_lower += ret_lower(buf[i]);
                num_digit += ret_digit(buf[i]);
                num_space += ret_space(buf[i]);
            }
        }
        //if it is read then you should go through and add to ascii things
        //printf("chunk size = %d\n", chunk_size);
        printf("ascii = %d, upper-case = %d, lower-case = %d, digits = %d, spaces = %d ", num_ascii, num_upper, num_lower, num_digit, num_space);
        printf("out of %ld bytes\n", sb.st_size);
        close(fd);
        exit(0);
    }
}

//Create functions

//1. ret_ascii -> increments asciis by 1
int ret_ascii(char input){
    if(isascii(input) != 0){
        return 1;
    }
    else return 0;
}
//2. ret_upper -> increments uppers by 1
int ret_upper(char input){
    if(isupper(input) != 0){
        return 1;
    }
    else{
        return 0;
    }
}
//3. ret_lower -> increments lowers by 1
int ret_lower(char input){
    if(islower(input) != 0){
        return 1;
    }
    else{
        return 0;
    }
}
//4. ret_digit -> increments digits by 1
int ret_digit(char input){
    if(input >= '0' && input <= '9'){
        return 1;
    }
    else{
        return 0;
    }
}
//4. ret_space -> increments spaces by 1
int ret_space(char input){
    if(input == ' '){
        return 1;
    }
    else{
        return 0;
    }
}


//Helper function to tell if an input is number
//returns 1 if it is not and 0 if it is
int isNumber(char *var){
    char num[strlen(var) + 1];
    strcpy(num, var);
    int boolean = 0;
    int j;
    while(j < strlen(var) && boolean == 0){
        if(num[j] >= '0' && num[j] <= '9'){
            j++;
        }
        else{
            boolean = 1;
        }
    }
    return boolean;
}