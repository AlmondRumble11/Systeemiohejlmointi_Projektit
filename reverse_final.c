/*
Name: Jesse Mustonen
Studentnumber: 0541805
Project: reverse
*/

#define  _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

//for the lins
struct Node{
    char *line;
    struct Node *pNext;  
};
typedef struct Node node;

//print the list contents(lines)
void print_the_list(node *pA){
    node *temp = pA;
    while(temp != NULL){
        fprintf(stdout,"%s", temp->line); 
        temp = temp->pNext; 
    }
}

//enpty the list
node *tyhjennys(node *pA){
	node *ptr = pA;
	while (ptr != NULL){
		pA = ptr->pNext;
		free(ptr->line); //free the char array
		free(ptr);
		
		ptr = pA;
	}
	pA = NULL;
	return pA;
}

//reverse the list
node *reverse_linked_list(node *pA){
    node *pPrev;
    node *pNow;
    node *pNext;

    pPrev = NULL;
    pNext = NULL;
    pNow = pA;

    //reverse the linked list
    //source: https://medium.com/outco/reversing-a-linked-list-easy-as-1-2-3-560fbffe2088
    while(pNow != NULL){
        pNext = pNow->pNext;
        pNow->pNext = pPrev;
        pPrev = pNow;
        pNow = pNext;
    }
    return pPrev;
}

//writing to output file
void write_to_output(node *pA, char *filename){
    FILE *output_file;
    node *ptr_start;
  
    //opening the output file
    if ((output_file = fopen(filename, "w")) == NULL){
        fprintf(stderr ,"reverse: cannot open file '%s'\n",filename);
        exit(1);	
    }
    //starting point is the start of the linked list
    ptr_start = pA;

    //writing the rows from the linked list
    while (ptr_start != NULL){
        fprintf(output_file,"%s",ptr_start->line);
        ptr_start = ptr_start->pNext;
    }
   
    //closing the output file
    fclose(output_file);

}
//open the input file and add lines to list
node *open_input_file(node *pA, char *filename){
    //variables
    FILE *input_file;
    node *ptr_new = NULL;
    node *pEnd = pA;
    char *line = NULL;
    size_t len = 0;

    //if the linked list is empty
    if (pA == NULL){

        //opening the file
        if ((input_file = fopen(filename, "r")) == NULL){
            fprintf(stderr ,"reverse: cannot open file '%s'\n",filename);
            exit(1);	
        }
        //reading the file
        while (getline(&line,&len, input_file) != -1){
            // allocating memory 
            if((ptr_new = (node *)malloc(sizeof(node))) == NULL){
                fprintf(stderr,"malloc failed\n");
                exit(1);	
            }
            ptr_new->line = (char* )malloc((strlen(line)+1)*sizeof(char));
            if (ptr_new->line == NULL){
                fprintf(stderr,"malloc failed\n");
                exit(1);
            }

            //saving value 
            strcpy(ptr_new->line, line);
            ptr_new->pNext = NULL;

            //moving the pointer
            if (pA == NULL){
                pA = ptr_new;
                pEnd = ptr_new;
            }else{
                pEnd->pNext = ptr_new;
                pEnd = ptr_new;
            }
        }
        //close the file and free buffer
        fclose(input_file);
        free(line);
    }
    return pA;
}

//if user did not give any arguments
void user_input(node *pA){
    char *line = NULL;
    size_t len = 0;
    node *ptr_new = NULL;
    node *pEnd;

    //https://c-for-dummies.com/blog/?p=1112
    //fprintf(stdout,"Type text:\n");
    while (getline(&line,&len,stdin)  != -1){
           if((ptr_new = (node *)malloc(sizeof(node))) == NULL){
                fprintf(stderr,"malloc failed\n");
                exit(1);	
            }

            // allocating memory 
            ptr_new->line = malloc((strlen(line)+1)*sizeof(char));
            if (ptr_new->line == NULL){
                fprintf(stderr,"malloc failed\n");
                exit(1);
            }
            //saving value 
            strcpy(ptr_new->line, line);
            ptr_new->pNext = NULL;
       
            //moving the pointer
            if (pA == NULL){
                pA = ptr_new;
                pEnd = ptr_new;
            }else{
                //new node to end of the list
                pEnd->pNext = ptr_new;
                pEnd = ptr_new;
            }
    }
    //reverse the given lines
    pA = reverse_linked_list(pA);

    //print the reverse liness
    print_the_list(pA);

    //free the buffer and empty the list
    free(line);
    tyhjennys(pA);
}

//main function
int main(int argc, char *argv[]) { 
    //start of the list of liens and output and input file chars
    node *pA = NULL;
	char *input;
	char *output;

    //if no files-->get form stdin
    if (argc == 1){
        user_input(pA);
    
    //only the input file
    }else if (argc == 2){
        
        //open the file and pritn to screen
        pA = open_input_file(pA,argv[1]);
        print_the_list(pA);

        //empty the list
        tyhjennys(pA);
    
    //has both input and output files
    }else if (argc == 3){
 		input = argv[1]; //input file
 		output = argv[2]; //output file
 		
        //add contents oof the file to list
		pA = open_input_file(pA,input);
		
		
		//source: https://stackoverflow.com/questions/7180293/how-to-extract-filename-from-path first awnser
        //check if files are the same
        if (strcmp(basename(input),basename(output)) != 0){

            //reverse the lsit
            pA = reverse_linked_list(pA);

            //write to output file
            write_to_output(pA,output);

            //empty the list of lines
            tyhjennys(pA);
 
        }else{  //the files are the same
            fprintf(stderr ,"reverse: input and output file must differ\n");
			exit(1);	
        }
    
    //no files given
    }else { 
        fprintf(stderr,"usage: reverse <input> <output>\n");
        exit(1);
    }
    return 0;
}
