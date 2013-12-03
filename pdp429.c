/*
	Name: Walid Owais
	EID: wo783
	Skip days: 1
*/

//Library Imports
#include <stdlib.h>
#include <stdio.h>

//Function Prototypes
void process_obj(void);

//Interpreter Variables
int verbose;
FILE *input = NULL;
int mem[65536];

//Registers
int pc = -1;
int a = 0;
int b = 0;
int c = 0;
int d = 0;


int main(int argc, char const *argv[])
{
	if(argc < 2){
		fprintf(stderr, "Usage: pdp429 [-v] object.obj\n");
		exit(1);
	}
	else if(argc == 2){
		input = fopen(argv[1], "r");
	}
	else if(argc == 3){
		if((argv[1][0] == '-') && (argv[1][1] == 'v') && (argv[1][2] == '\0')){
			verbose = 1;
			input = fopen(argv[2], "r");
		}
		else{
			fprintf(stderr, "Usage: pdp429 [-v] object.obj\n");
			exit(1);
		}
	}
	else{
		fprintf(stderr, "Usage: pdp429 [-v] object.obj\n");
		exit(1);
	}

	if(input == NULL){
		fprintf(stderr, "Error opening file.\n");
		exit(1);
	}

	process_obj();

	return 0;
}

void process_obj(void){
	int block_size = 0;
	int current_size = 0;
	int current_addr = -1;

	if((getc(input) != 'O') || (getc(input) != 'B') || 
		(getc(input) != 'J') || (getc(input) != 'G')){
		fprintf(stderr, "Object file format invalid.\n");
		exit(1);
	}

	pc = (getc(input) << 8) + getc(input);

	block_size = getc(input);
	while(block_size != EOF){

		current_addr = (getc(input) << 8) + getc(input);
		current_size = 3;

		while(current_size < block_size){
			mem[current_addr] = (getc(input) << 8) + getc(input);

			fprintf(stderr, "0x%0x :   0x%0x\n", 
				current_addr, mem[current_addr]);

			current_addr++;
			current_size += 2;
		}

		block_size = getc(input);
	}



}