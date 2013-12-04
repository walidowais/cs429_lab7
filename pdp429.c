/*
	Name: Walid Owais
	EID: wo783
*/

//Library Imports
#include <stdlib.h>
#include <stdio.h>

//Function Prototypes
void process_obj(void);
void zero(int instruction);
void add(int instruction);
void sub(int instruction);
void mul(int instruction);
void dvd(int instruction);
void and(int instruction);
void or(int instruction);
void xor(int instruction);
void ld(int instruction);
void st(int instruction);
void iot(int instruction);
void mov(int instruction);
void stk(int instruction);
void reg(int instruction);
void opr(int instruction);

//Interpreter Variables
int verbose = 0;
FILE *input = NULL;
int mem[65536];
long long int time = 0;
char *verbose_format = "Time %3lld: PC=0x%04X instruction = 0x%04X (%s)";

//Registers
int pc = -2;
int sp = 0;
int spl = 0;
int psw = 0;
int a = 0;
int b = 0;
int c = 0;
int d = 0;
int link_bit = 0;


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

			// fprintf(stderr, "0x%0x :   0x%0x\n", 
			// 	current_addr, mem[current_addr]);

			current_addr++;
			current_size += 2;
		}

		block_size = getc(input);
	}
}

void run(void){
	int instruction = 0;
	int opcode = 0;
	int rest = 0;

	while(pc >= 0){
		//fetch
		instruction = mem[pc];
		time++;

		//decode
		opcode = ((instruction & 0xF000) >> 12) & 0x000F;

		//execute
		switch(opcode){
			case 0:
				zero(instruction);
				break;
			case 1:
				add(instruction);
				break;
			case 2:
				sub(instruction);
				break;
			case 3:
				mul(instruction);
				break;
			case 4:
				dvd(instruction);
				break;
			case 5:
				and(instruction);
				break;
			case 6:
				or(instruction);
				break;
			case 7:
				xor(instruction);
				break;
			case 8:
				ld(instruction);
				break;
			case 9:
				st(instruction);
				break;
			case 10:
				iot(instruction);
				break;
			case 11:
				mov(instruction);
				break;
			case 12:
				stk(instruction);
				break;
			case 13:
				//no instruction;
				break;
			case 14:
				reg(instruction);
				break;
			case 15:
				opr(instruction);
				break;
			default:
				fprintf(stderr, "Invalid Opcode\n");
				exit(1);
		}
	}
}

void zero(int instruction){
	char *op;
	int old_pc = pc;

	switch(instruction & 0x0FFF){
		case 0:
			//NOP
			op = "NOP";
			pc++;
			break;
		case 1:
			//HLT
			op = "HLT";
			pc = -99;
			break;
		case 2:
			//RET
			op = "RET";
			sp++;
			if(sp >> 16){
				//underflow
				link_bit = 1;
			}
			pc = mem[sp];
			break;
		default:
			fprintf(stderr, "Invalid Opcode\n");
			exit(1);
	}

	if(verbose){
		fprintf(stderr, verbose_format, time, old_pc, instruction, op);
	}
}
void add(int instruction){
	int indirect = instruction & 0x0200;
	int address = 0;
	char *op;
	char reg_name;
	int reg_val;
	int result;


	address += (instruction & 0x00FF);
	if(instruction & 0x0100){
		address += (pc & 0xFF00);
	}
	if(indirect){
		time++;
		address = mem[address];
	}

	switch((instruction & 0x0C00) >> 10){
		case 0:
			op = "ADDA";
			reg_name = 'A';
			reg_val = a;
			a += mem[address];
			link_bit = ((a > 0xFFFF) ? 1 : 0);
			a &= 0xFFFF;
			time++;
			result = a;
			break;
		case 1:
			op = "ADDB";
			reg_name = 'B';
			reg_val = b;
			b += mem[address];
			link_bit = ((b > 0xFFFF) ? 1 : 0);
			b &= 0xFFFF;
			time++;
			result = b;
			break;
		case 2:
			op = "ADDC";
			reg_name = 'C';
			reg_val = c;
			c += mem[address];
			link_bit = ((c > 0xFFFF) ? 1 : 0);
			c &= 0xFFFF;
			time++;
			result = c;
			break;
		case 3:
			op = "ADDD";
			reg_name = 'D';
			reg_val = d;
			d += mem[address];
			link_bit = ((d > 0xFFFF) ? 1 : 0);
			d &= 0xFFFF;
			time++;
			result = d;
			break;
	}

	if(verbose){
		fprintf(stderr, verbose_format, time, pc, instruction, "ADDA");
		fprintf(stderr, ": %c -> 0x%04x, M[0x%04x] -> 0x%04x, 0x%04x -> %c\n", 
			reg_name, reg_val, address, mem[address], result, reg_name);
	}

	pc++;
}
void sub(int instruction){
	//work
}
void mul(int instruction){
	//work
}
void dvd(int instruction){
	//work
}
void and(int instruction){
	//work
}
void or(int instruction){
	//work
}
void xor(int instruction){
	//work
}
void ld(int instruction){
	//work
}
void st(int instruction){
	//work
}
void iot(int instruction){
	//work
}
void mov(int instruction){
	//work
}
void stk(int instruction){
	//work
}
void reg(int instruction){
	//work
}
void opr(int instruction){
	//work
}