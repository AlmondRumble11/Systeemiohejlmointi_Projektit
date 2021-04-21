/*
Name: Jesse Mustonen
Studentnumber: 0541805
Project: mycat
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>


//printing the contents
void print_file_contents(char file[]){

   FILE *filename; 
   //char *buffer; 
   char buffer[1024]; //fixed size. Used the same as size as the lecture materials. 

    //opening the file and checking it worked
   filename = fopen(file, "r");
   if (filename == NULL){
        //printf("my-cat: cannot open file\n");
        printf("wcat: cannot open file\n");
        exit(1);
   }

    //reading the file and inputing it to the user
    //as the hint was to use fgets i used it but hte optimal would be to use getline as the it reallocates memory itself and the line can be as long as possible
   while(fgets(buffer, 1024, filename) != NULL){
	   printf("%s",buffer);
    }
   //close the file 
   fclose(filename);

}

//main fucntion
int main(int argc, char *argv[]){
    
    //check that has files
    if (argc < 2){
        //printf("No file(s) given. Exiting the program...\n");
        exit(0);
    }

    //going through the files and printing the contents
    for (int i = 1; i < argc; i++){
        print_file_contents(argv[i]);
    }
    return 0;
}
