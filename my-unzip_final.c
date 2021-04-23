/*
Name: Jesse Mustonen
Studentnumber: 0541805
Project: my-unzip
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

//struct to keep the count and the character
struct characters{
    int count;
    char chart;
  
};

//funciton to unzip the file
void unzipping(char *filename){
    FILE *file;

    size_t read_bytes;
    struct characters characters;

    //opening the file(binary file)
    file = fopen(filename, "rb");
    if(file == NULL){
        printf("Error opening file: %s\n",filename);
        exit(1);
    }

 
    //reading the file
    //source: https://overiq.com/c-programming-101/fread-function-in-c/
    //EXAMPLE 5 and final example
    while((read_bytes = fread(&characters,sizeof(characters.count)+sizeof(characters.chart),1,file))){
            //if end of the file
            if(read_bytes != 1){
                break;
            }
            //print the character as many times as the count was
            for(int i=0;i<characters.count;i++){
                    printf("%c",characters.chart);
            }
    }

    //closing the file
    fclose(file);
  
}

//main function
int main(int argc, char *argv[]){

    //has files
    if (argc < 2){
        printf("wunzip: file1 [file2 ...]\n");
        exit(1);
    }

    //unzip the files
    for(int i = 1; i<argc; i++){
        unzipping(argv[i]);
    }

    return 0;
}
