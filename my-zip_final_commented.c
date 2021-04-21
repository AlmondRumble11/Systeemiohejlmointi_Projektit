/*
Name: Jesse Mustonen
Studentnumber: 0541805
Project: my-zip
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//struct for the lines
struct lines{
    int line;//https://stackoverflow.com/questions/11438794/is-the-size-of-c-int-2-bytes-or-4-bytes the source says that int is 4 bytes nowadays in 32 and 64 bit systems

    struct lines *next;
};
typedef struct lines lines;

//empty the list of lines
void tyhjennys(lines **start){
	lines *ptr = *start;
	while (ptr != NULL){
		*start = ptr->next;
		free(ptr);
		ptr = *start;
	}
}

//open file and add lines to list
void open_file(lines **start,char *filename,lines **last){
    FILE *input_file;
    lines *new_line = NULL;
    int c;

    //open file and check if opening worked
    input_file = fopen(filename, "r");
    if (input_file == NULL){
        printf("Error opening file: %s\n",filename);
        exit(1);
    }

    //going through the file and adding to linked list one character at a time
    while(1){

        //get the character
        //use of fgetc(): https://www.tutorialspoint.com/c_standard_library/c_function_fgetc.htm
        c = fgetc (input_file);
        //stop if end of file
        if ( c == EOF )
            break;

        //alocating memory for new list item
        if((new_line = (lines *)malloc(sizeof(lines))) == NULL){
            printf("malloc failed\n");
            exit(1);	
        }

        //saving value 
        new_line->line = c;
        new_line->next = NULL;
 
        //move the pointer to right spot
        if (*start == NULL){
            *start = new_line;
        }else{
            (*last)->next = new_line; 
            
        }
        *last = new_line;
    }
    //fclose the file
    fclose(input_file);
}


//idea of how to compare next and current char: https://www.geeksforgeeks.org/maximum-occurring-character-linked-list/

//compress the file
void compress_file(lines *start){
    //varibles
    lines *temp = start;
    lines *next;
    int char_count;
    char character;
   
    //go through the linked list
    while(temp != NULL){
     
        //get next "char"(integer of ascii charater) from the list and add to count
        next = temp->next;
        char_count = 1;
        
        //count the occurrence of the character 
        while(next != NULL){
            if(temp->line == next->line){
                char_count++;
            }else{
                break;
            }
            next = next->next;
        }

        //write to stdout
        character = temp->line;
        fwrite(&char_count,4,1,stdout);        //4 bytes for the int
        fwrite(&character,sizeof(character),1,stdout);//sizeof char 1 byte
        
        //set the next element as the next character and reset the count
        temp = next;
        char_count = 0;
    }
}

//main function
int main(int argc, char *argv[]){
    //start and end of the list
    lines *end = NULL;
    lines *start = NULL;

    //has files
    if (argc < 2){
        printf("wzip: file1 [file2 ...]\n");
        exit(1);
    }

    //going throught the files
    for(int i = 1; i<argc; i++){
        open_file(&start, argv[i],&end);
    }
    //compress the file
    compress_file(start);
    //empty list on lines
    tyhjennys(&start);
    return 0;
}

