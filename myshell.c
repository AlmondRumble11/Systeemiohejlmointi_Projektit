/*
Name: Jesse Mustonen
Studentnumber: 0541805
Project: myshell
*/


#define   _GNU_SOURCE //https://man7.org/linux/man-pages/man3/getdelim.3.html
#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/types.h>
#include<string.h>
#include<sys/wait.h>
#include<signal.h>
#include<ctype.h>
#include<fcntl.h>
#include<sys/stat.h>


//source for the basic undestanding of the shell: https://brennan.io/2015/01/16/write-a-shell-in-c/
//used in fork and getting the arguments from the command line
//struct for the commands
struct commands{
    char *command;
    struct commands *next;

};
//struct for the paths
struct paths{
	char *path;
	struct paths *next;

};

//freeing the path list
void free_path(struct paths **start){
	struct paths *ptr;
	if(*start == NULL){
		return;
	}
	while (*start != NULL){
		ptr = (*start)->next;
		free((*start)->path); //free the char array
		free(*start);
		*start = ptr;
	}
	*start = NULL;
}

//freeing the commnd list
void free_commands(struct commands **start){
	struct commands *ptr = *start;
	if(*start == NULL){
		return;
	}
	while (*start != NULL){
		ptr = (*start)->next;
		free((*start)->command); //free the char array
		free(*start);
		*start = ptr;
	}
	*start = NULL;
	
}

//prompt
void print_start(void){
	printf("wish> ");
}

//running the commands. Takes a lot of arguments so if the comman fails it frees them
void run_command(char **commands, struct paths *path, int redirect, int command_count, int parallel_count, struct commands **all_commands, FILE *fp, char *buffer){
	
	char error_message[30] = "An error has occurred\n";
	pid_t pids;
	char full_path[200]; //for full path

	if((pids = fork()) < 0){ //fork failed	
	

			printf("frok failed\n");
			write(STDERR_FILENO, error_message, strlen(error_message));
			exit(1);
	}else if(pids == 0){	//child process

			//if redirect
			if(redirect==1){
				//source to how to redirect to file: https://stackoverflow.com/questions/2605130/redirecting-exec-output-to-a-buffer-or-file
				//source to truncate: http://codewiki.wikidot.com/c:system-calls:open
				//open file. It is the last of the commands
				int file = open(commands[command_count-1], O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR| S_IWUSR | S_IXUSR);
				//printf("opening file %s\n",commands[command_count-1]);
				if(file < 0){
					//printf("failed to open file %s\n", commands[command_count-1]);
					write(STDERR_FILENO, error_message, strlen(error_message));
					return;
				}
				//delete the filename from the commands
				commands[command_count-1] = NULL;
		
				//stdout
				if(dup2(file, STDOUT_FILENO) < 0){
					printf("dup1 failed\n");
					write(STDERR_FILENO, error_message, strlen(error_message));
				}
			
				//stderr
				if(dup2(file, STDERR_FILENO) < 0){
					printf("dup2 failed\n");
					write(STDERR_FILENO, error_message, strlen(error_message));
				}
				//close the opened file
				close(file);	
			}
			
			int i = 0; //to count tested paths
			int error_count = 0; //how many access errors
			struct paths *temp = path; 
			//test all of the paths available
			while(temp != NULL){
				
				//make the full path
				strcpy(full_path,temp->path);
				strcat(full_path,"/"); //  tests/p1.sh
				strcat(full_path,commands[0]);
				
				//check if access to path
				if(access(full_path, X_OK)!=0){
					error_count++;
				}else{

					//run command
					if(execv(full_path, commands) == -1){	
							//printf("in exec\n");
							free_commands(all_commands);
							free(commands);
							free_path(&path);
							free(buffer);
							fclose(fp);
							write(STDERR_FILENO, error_message, strlen(error_message));
							exit(1);
					}	
				}
				//increment tested paths and go next
				i++;
				temp = temp->next;	
			}

			//if no access to paths--> free everything and print error message
			if(error_count == i){
				free_commands(all_commands);
				free(commands);
				free_path(&path);
				free(buffer);
				fclose(fp);
				//printf("no access\n");
				write(STDERR_FILENO, error_message, strlen(error_message));	
				exit(1);
			}
			exit(0); //if everything was successful
	}		
}

//build in cd command
void build_in_cd(char **arguments){
	char error_message[30] = "An error has occurred\n";

	//check that something after cd
	if(arguments[1]==NULL){
		//printf("no path give\n");
		write(STDERR_FILENO, error_message, strlen(error_message));
		return;
		
	}else{
		//source chdir() usage:https://www.geeksforgeeks.org/chdir-in-c-language-with-examples/
		//change to given directory 
		//cd only takes 1 arguments so index numbers can be used. arguments[0] is the cd command
		if(chdir(arguments[1]) != 0){
			//printf("error in chidri\n");
			write(STDERR_FILENO, error_message, strlen(error_message));
		}
	}
}

//adding to path 
void add_to_path(struct paths **start,char *path_name, struct paths **last){
	struct paths *new_path;

	//memory allocation for the next eelemnet and the name	
	if((new_path = (struct paths *)malloc(sizeof(struct paths))) == NULL){
            	printf("malloc failed\n");
            	exit(1);	
    }
	if((new_path->path = (char*)malloc(sizeof(char)*strlen(path_name)+1))==NULL){
		printf("malloc failed\n");
		exit(1);
	}
	
	//saving value 
	strcpy(new_path->path,path_name);
	new_path->next = NULL;

	//move the pointer to right spot
	if (*start == NULL){
		*start = new_path;
		*last = new_path;
	}else{
		(*last)->next = new_path; 
		*last = new_path;
	}
}


//build in path fuction
void build_in_path(char **args, int *only_build_ins, struct paths **path, struct paths **last){
	
	//does not have any paths to add-->empty the path ansd only the build ins allowed
	if(args[1]==NULL){
		*only_build_ins = 1;
		free_path(path);
	//has something after path
	}else{
		
		*only_build_ins = 0; //to check that other than build in can be driven
		int i = 1; //start from the path given
		free_path(path); //free the path so only new paths available

		//go trought the given path args and add them to list of paths
		while(args[i] != NULL){
			add_to_path(path,args[i],last);
			i++;
		}
	}
}

//check the build ins(other than exit as i had trouble free all of the commands for it-->it can be found in the main function)
int check_build_in(char **arguments,int *only_build_ins,struct paths **path, struct paths **end,struct commands **start,FILE *fp,char **buffer){

	//arguments is not empty-->if not error ticket
    if(arguments == NULL){
		return -1;
	}

	//has something after the cd/path--> if not error ticket
	if(arguments[0] == NULL){
		return -1;
	}
	
	//check if path
    if(strcmp(arguments[0],"path")==0){
		build_in_path(arguments, only_build_ins,path,end);
		return 1;
	
	//check if cd
    }else if(strcmp(arguments[0],"cd")==0){
		build_in_cd(arguments);
        return 1;
    }

	//if  either of them
    return 0;
}

//getting the arguments form the line split by the whiteline
char **get_arguments(char *command){
	int buffer_size; //buffer size
	buffer_size = strlen(command); //buffer size is the line lenght
	int wp_command_count = 0; //how many commands
	char *arg = command; //temp for the command
	char **white_space_commands = malloc(buffer_size*sizeof(char*)); //allocate momory for the parsed commands
	char *white_space = strtok(arg," "); //get the first command
	white_space_commands[wp_command_count] = white_space; //add it to all of the commands
	
	//checks if only whitespaces and frees the allocated memeory for the commands
	if(white_space == NULL){
		free(white_space_commands);
		return NULL;

	}
	
	//go through the next commnds
	while(white_space != NULL){
		//take next command and add it to lsit of commands
		white_space = strtok(NULL," ");
		wp_command_count++;
		white_space_commands[wp_command_count]=white_space;
	}
	
	//return the list of commands
	return white_space_commands;
}

//checks for the '>' redirect symbol
char ** check_redirect(char **arguments, int *redirect, int *command_count){
	char error_message[30] = "An error has occurred\n";
	char **final_args = NULL; //list of final command and its arguments
	char *new_arg;
	int i=0;
	int arg_index = 0;

	

	//should have something
	if(arguments[arg_index]== NULL){
		//printf("something went wrong\n");
		write(STDERR_FILENO, error_message, strlen(error_message));
	}
	final_args = malloc(100*sizeof(char)+1);

	//check the alone >
	if(strcmp(arguments[0],">")==0){
		//printf("> cannot be alone\n");
		free(final_args);
		write(STDERR_FILENO, error_message, strlen(error_message));
		return NULL;
	}
	
	int j=0;
	int after_redirect = 0;
	*redirect =0;
	//check rediorects
	while(arguments[i]!=NULL){
		
		//only the > symbol as argument. for example ls > aaaa.txt
		if(strcmp(arguments[i],">")==0){
			*redirect = *redirect +1;
			i++;
			continue;
		}
		j = 0;
		//has > symbol
        //source: https://www.tutorialspoint.com/c_standard_library/c_function_strstr.htm
		if(strstr(arguments[i], ">")!=NULL){
			//check how many >. used to check >>>>> for example
			while(arguments[i][j]!='\0'){

				//if > found-->add to > count
				if(arguments[i][j]=='>'){
					*redirect = *redirect +1;
				}

				//if more than oen > in the command
				if(*redirect>1){
					//printf("too many >\n");
					*redirect = -1;
					free(final_args);
					return NULL;
				}
				j++;
			}

			//has 1  redirects			
			if(*redirect < 2){
		
				new_arg=strtok(arguments[i],">"); //get commands
				final_args[arg_index]=new_arg; //add command to list

				//go trough the args and agg to list
				while (new_arg!=NULL){
					final_args[arg_index]=new_arg;
					new_arg=strtok(NULL,">"); //file
					arg_index++;
				}
			}
		//didi not have > in the argument
		}else{
			
			//already had > at least one time
			if(*redirect>=1){
				after_redirect++;
			}

			//checks if more than 1 argument after > command
			if(after_redirect > 1){
				*redirect = -1;
				//printf("too many files after >\n");
				free(final_args);
				return NULL;
			}

			//add to final list of arguments
			final_args[arg_index]=arguments[i];
			arg_index++;
		}
		i++;
	}

	//add null to end of the list
	*command_count = arg_index;
	final_args[arg_index]=NULL;

	//if no file given
	if((*redirect==1)&&(arg_index==1)){
		//printf("no fiel given\n");
		*redirect = -1;
		free(final_args);
		return NULL;
	}
	return final_args;
}

//main function of the program
int main(int argc, char *argv[]){
	//all of the varibales
	char error_message[30] = "An error has occurred\n";
	char **arguments=NULL; //getting the first arguments parsed by whitespace
	char *buffer=NULL; //buffer for the getline
	char *parallel_token; //token to get & 
	char **final_arguments=NULL; //final arguments
    int batch_mode = 0; // if batch mode
	int build_in = 0; //is a buildin command
	int only_build_ins = 0; //accepts only build ins
	int redirection = 0; //redirect(>) count
	int parallel_count = 0; //parallel(&) count
	int command_count = 0; //how many commands
	int i=0; //for loop
	size_t buffer_size=0; //buffer size
	size_t number_characters;
	struct commands *start =NULL; //start of the command list
	struct commands *temp; //to go trought the commands
	struct commands *last = NULL; //last of the commands list
	struct commands *new_command; //new command give
	struct paths *paths = NULL; //start fo the path list
	struct paths *end_path = NULL; //end of the path list
	FILE *fp; //file 0r stdin
	
	//adding the first path
	add_to_path(&paths, "/bin", &end_path);
	
    //shell
    while(1){
        //batch mode
        if(argc > 1){
            batch_mode = 1;
            
            //only one file 
            if (argc < 3){

				//open the fle and check it worked
            	fp = fopen(argv[1],"r");
                if(fp== NULL){
					//printf("failed to open file\n");
                    write(STDERR_FILENO, error_message, strlen(error_message));
                    exit(1);
                }
         
			//too many files
            }else{
				//printf("too many files\n");
                write(STDERR_FILENO, error_message, strlen(error_message));
                exit(1);
            }
		//interactive mode
        }else{
            //promt and stdin as the input
			fp =stdin;
            print_start();
            fflush(NULL);
        }
		
		//get commands
		while((number_characters = getline(&buffer,&buffer_size, fp)) != -1){
			char *arg=buffer; // for the freeing of the buffer
			//parse command by the & symbol to get how many parallel operations
            //source: https://www.geeksforgeeks.org/strtok-strtok_r-functions-c-examples/
			while((parallel_token = strtok_r(arg,"&",&arg))){
				parallel_count = parallel_count +1;
				if((new_command = (struct commands *)malloc(sizeof(struct commands))) == NULL){
					printf("malloc failed\n");
					exit(1);	
				}

				if((new_command->command = (char*)malloc(sizeof(char)*strlen(parallel_token)+1))==NULL){
					printf("malloc failed\n");
					exit(1);
				}
				//saving value 
				strcpy(new_command->command,parallel_token);
				new_command->next = NULL;
				
				//move the pointer to right spot
				if (start == NULL){
					start = new_command;
				}else{
					last->next = new_command; 
					
				}
					last = new_command;	
			}

		temp = start;
		//run the comamnds for as many times ad the line had &
        //source: https://stackoverflow.com/questions/876605/multiple-child-process
		for(i=0; i<parallel_count;i++){
		//go command by line
			while(temp != NULL){
			
				//delete '\n'. does not wokr if only \n
				strtok(temp->command, "\n");
				//check that not '\n'
				if(strcmp(temp->command, "\n")!=0){

					//get arguments parsed by whitespace
					arguments = get_arguments(temp->command);
					
					//had arguments
					if(arguments != NULL){

						//final parsed argumetn and check the redirect
						final_arguments = check_redirect(arguments, &redirection, &command_count);
						free(arguments); //free the original arguments

						//if error in redirect reset values and go next command
						if(redirection==-1){
						//	printf("Redirection error");
							free(final_arguments);
							write(STDERR_FILENO, error_message, strlen(error_message));
							redirection = 0;
							build_in = 0;
							only_build_ins=0;
							temp = temp->next;
							parallel_count = 0;
							continue;
						}
						
						//check bfinal args not empty and cehcks buld in exit
						if(final_arguments!=NULL){

							//has exit nad nothing after it and free the allocated momory and exits
							if((strcmp(final_arguments[0], "exit")==0) && (final_arguments[1] == NULL)){
								if(paths!=NULL){
									free_path(&paths);
								}
								if(final_arguments!=NULL){
									free(final_arguments);
								}
								if(start!=NULL){
									free_commands(&start);
								}
								free(buffer);
								fclose(fp);
								exit(0);
							//no exit and has something after it
							}else{

								//if exit reset variables and go next
								if(strcmp(final_arguments[0], "exit")==0){
									free(final_arguments);
									redirection = 0;
									build_in = 0;
									only_build_ins=0;
									temp = temp->next;
									write(STDERR_FILENO, error_message, strlen(error_message));
									continue;
								}
							}
						}
						//check for cd/path
						build_in = check_build_in(final_arguments,&only_build_ins, &paths, &end_path,&start,fp,&buffer);
					
						//check if was cd/path and if only build ins
						if((build_in == 0)&&(only_build_ins==0)){
							//run the command
							run_command(final_arguments,paths,redirection,command_count, parallel_count,&start,fp,buffer);
						
						//if only build command or only build ins
						}else{

							//if only build ins
							if((only_build_ins==1)&&(build_in == 0)){

								write(STDERR_FILENO, error_message, strlen(error_message));
							}
						}
						//free arguments
						free(final_arguments);
					}
				}
			//reset the variables and go tho next command
			redirection = 0;
			build_in = 0;
			only_build_ins=0;
			temp = temp->next;
			}

			//free
			free_commands(&start);
		}

		//wait for the parallel command to finnish
		int status;
		for (int x = 0; x<parallel_count;x++){
			wait(&status);
				
		}
		
		//reset parallel_count for the next comman
		parallel_count = 0;
		//if interacvite mode break the getline so get the next command
		if(!batch_mode){
				break;
		}	
	}
	//free commands list
	free_commands(&start);
	//if was batch mode break the loop and free allocatec memory
	if(batch_mode){
		free(buffer);
		fclose(fp);
		free_path(&paths);
		free_commands(&start);
		break;
	}
	}
	return 0;
}
