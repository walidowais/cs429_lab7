/*
	Name: Walid Owais
	EID: wo783
*/

// stderr

//Library Imports
#include <stdlib.h>
#include <stdio.h>

//Registers
int reg_arr[9];
#define a reg_arr[0]
#define b reg_arr[1]
#define c reg_arr[2]
#define d reg_arr[3]
#define pc reg_arr[4]
#define psw reg_arr[5]
#define sp reg_arr[6]
#define spl reg_arr[7]
#define link_bit reg_arr[8]

//Function Prototypes
void process_obj(void);
void run(void);
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



int main(int argc, char const *argv[]){
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
	run();

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
	psw = 1;

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
	a = 0;
	b = 0;
	c = 0;
	d = 0;
	sp = 0;
	spl = 0;
	link_bit = 0;
	psw = 1;

	int instruction = 0;
	int opcode = 0;
	int rest = 0;

	while(psw){
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
				pc++;
				break;
			case 14:
				reg(instruction);
				break;
			case 15:
				opr(instruction);
				break;
			default:
				fprintf(stderr, "Invalid Opcode (run)\n");
				exit(1);
		}
	}
}

void zero(int instruction){
	char *op;
	int old_pc = pc;
	int old_sp = sp;
	int old_psw = psw;

	switch(instruction & 0x0FFF){
		case 0:
			//NOP
			op = "NOP";
			pc++;

			if(verbose){
				fprintf(stderr, verbose_format, 
					time, old_pc, instruction, op);
				fprintf(stderr, "\n");
			}

			break;
		case 1:
			//HLT
			op = "HLT";
			psw = 0;

			if(verbose){
				fprintf(stderr, verbose_format, 
					time, old_pc, instruction, op);
				fprintf(stderr, ": %s -> 0x%04x, 0x%04x -> %s\n", 
					"PSW", old_psw, psw, "PSW");
			}

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

			if(verbose){
				fprintf(stderr, verbose_format, 
					time, old_pc, instruction, op);
				fprintf(stderr, ": SP -> 0x%04x, 0x%04x -> SP, M[0x%04x] -> 0x%04x, 0x%04x -> PC\n", 
					old_sp, sp, sp, pc, pc);
			}

			break;
		default:
			fprintf(stderr, "Invalid Opcode (zero)\n");
			exit(1);
	}
}


void add(int instruction){
	int indirect = instruction & 0x0200;
	int address = 0;
	char *op;
	char reg_name;
	int reg_val;
	int x, r;


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
			op = (indirect ? "I ADDA" : "ADDA");
			reg_name = 'A';
			reg_val = a;
			x = mem[address];
			r = a + x;

			if(((a & 0x8000) == (x & 0x8000)) && 
				((a & 0x8000) != (r & 0x8000)))
				link_bit = (link_bit ? 0 : 1);

			a = r & 0xFFFF;
			time++;
			break;
		case 1:
			op = (indirect ? "I ADDB" : "ADDB");
			reg_name = 'B';
			reg_val = b;
			x = mem[address];
			r = b + x;

			if(((b & 0x8000) == (x & 0x8000)) && 
				((b & 0x8000) != (r & 0x8000)))
				link_bit = (link_bit ? 0 : 1);

			b = r & 0xFFFF;
			time++;
			break;
		case 2:
			op = (indirect ? "I ADDC" : "ADDC");
			reg_name = 'C';
			reg_val = c;
			x = mem[address];
			r = c + x;

			if(((c & 0x8000) == (x & 0x8000)) && 
				((c & 0x8000) != (r & 0x8000)))
				link_bit = (link_bit ? 0 : 1);

			c = r & 0xFFFF;
			time++;
			break;
		case 3:
			op = (indirect ? "I ADDD" : "ADDD");
			reg_name = 'D';
			reg_val = d;
			x = mem[address];
			r = d + x;

			if(((d & 0x8000) == (x & 0x8000)) && 
				((d & 0x8000) != (r & 0x8000)))
				link_bit = (link_bit ? 0 : 1);

			d = r & 0xFFFF;
			time++;
			break;
	}

	if(verbose){
		fprintf(stderr, verbose_format, time, pc, instruction, op);
		fprintf(stderr, ": %c -> 0x%04x, M[0x%04x] -> 0x%04x, 0x%04x -> %c\n", 
			reg_name, reg_val, address, x, r, reg_name);
	}

	pc++;
}


void sub(int instruction){
	int indirect = instruction & 0x0200;
	int address = 0;
	char *op;
	char reg_name;
	int reg_val;
	int x, r;


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
			op = (indirect ? "I SUBA" : "SUBA");
			reg_name = 'A';
			reg_val = a;
			x = mem[address];
			r = a - x;

			if(((a & 0x8000) == (x & 0x8000)) && 
				((a & 0x8000) != (r & 0x8000)))
				link_bit = (link_bit ? 0 : 1);

			a = r & 0xFFFF;
			time++;
			break;
		case 1:
			op = (indirect ? "I SUBB" : "SUBB");
			reg_name = 'B';
			reg_val = b;
			x = mem[address];
			r = b - x;

			if(((b & 0x8000) == (x & 0x8000)) && 
				((b & 0x8000) != (r & 0x8000)))
				link_bit = (link_bit ? 0 : 1);

			b = r & 0xFFFF;
			time++;
			break;
		case 2:
			op = (indirect ? "I SUBC" : "SUBC");
			reg_name = 'C';
			reg_val = c;
			x = mem[address];
			r = c - x;

			if(((c & 0x8000) == (x & 0x8000)) && 
				((c & 0x8000) != (r & 0x8000)))
				link_bit = (link_bit ? 0 : 1);

			c = r & 0xFFFF;
			time++;
			break;
		case 3:
			op = (indirect ? "I SUBD" : "SUBD");
			reg_name = 'D';
			reg_val = d;
			x = mem[address];
			r = d - x;

			if(((d & 0x8000) == (x & 0x8000)) && 
				((d & 0x8000) != (r & 0x8000)))
				link_bit = (link_bit ? 0 : 1);

			d = r & 0xFFFF;
			time++;
			break;
	}

	if(verbose){
		fprintf(stderr, verbose_format, time, pc, instruction, op);
		fprintf(stderr, ": %c -> 0x%04x, M[0x%04x] -> 0x%04x, 0x%04x -> %c\n", 
			reg_name, reg_val, address, x, r, reg_name);
	}

	pc++;
}


void mul(int instruction){
	int indirect = instruction & 0x0200;
	int address = 0;
	char *op;
	char reg_name;
	int reg_val;
	int x, r;


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
			op = (indirect ? "I MULA" : "MULA");
			reg_name = 'A';
			reg_val = a;
			x = mem[address];
			r = a * x;

			if(((a & 0x8000) == (x & 0x8000)) && 
				((a & 0x8000) != (r & 0x8000)))
				link_bit = (link_bit ? 0 : 1);

			a = r & 0xFFFF;
			time++;
			break;
		case 1:
			op = (indirect ? "I MULB" : "MULB");
			reg_name = 'B';
			reg_val = b;
			x = mem[address];
			r = b * x;

			if(((b & 0x8000) == (x & 0x8000)) && 
				((b & 0x8000) != (r & 0x8000)))
				link_bit = (link_bit ? 0 : 1);

			b = r & 0xFFFF;
			time++;
			break;
		case 2:
			op = (indirect ? "I MULC" : "MULC");
			reg_name = 'C';
			reg_val = c;
			x = mem[address];
			r = c * x;

			if(((c & 0x8000) == (x & 0x8000)) && 
				((c & 0x8000) != (r & 0x8000)))
				link_bit = (link_bit ? 0 : 1);

			c = r & 0xFFFF;
			time++;
			break;
		case 3:
			op = (indirect ? "I MULD" : "MULD");
			reg_name = 'D';
			reg_val = d;
			x = mem[address];
			r = d * x;

			if(((d & 0x8000) == (x & 0x8000)) && 
				((d & 0x8000) != (r & 0x8000)))
				link_bit = (link_bit ? 0 : 1);

			d = r & 0xFFFF;
			time++;
			break;
	}

	if(verbose){
		fprintf(stderr, verbose_format, time, pc, instruction, op);
		fprintf(stderr, ": %c -> 0x%04x, M[0x%04x] -> 0x%04x, 0x%04x -> %c\n", 
			reg_name, reg_val, address, x, r, reg_name);
	}

	pc++;
}


void dvd(int instruction){
	int indirect = instruction & 0x0200;
	int address = 0;
	char *op;
	char reg_name;
	int reg_val;
	int x, r;


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
			op = (indirect ? "I DIVA" : "DIVA");
			reg_name = 'A';
			reg_val = a;
			x = mem[address];
			r = a / x;
			a = r & 0xFFFF;
			time++;
			break;
		case 1:
			op = (indirect ? "I DIVB" : "DIVB");
			reg_name = 'B';
			reg_val = b;
			x = mem[address];
			r = b / x;
			b = r & 0xFFFF;
			time++;
			break;
		case 2:
			op = (indirect ? "I DIVC" : "DIVC");
			reg_name = 'C';
			reg_val = c;
			x = mem[address];
			r = c / x;
			c = r & 0xFFFF;
			time++;
			break;
		case 3:
			op = (indirect ? "I DIVD" : "DIVD");
			reg_name = 'D';
			reg_val = d;
			x = mem[address];
			r = d / x;
			d = r & 0xFFFF;
			time++;
			break;
	}

	if(verbose){
		fprintf(stderr, verbose_format, time, pc, instruction, op);
		fprintf(stderr, ": %c -> 0x%04x, M[0x%04x] -> 0x%04x, 0x%04x -> %c\n", 
			reg_name, reg_val, address, x, r, reg_name);
	}

	pc++;
}


void and(int instruction){
	int indirect = instruction & 0x0200;
	int address = 0;
	char *op;
	char reg_name;
	int reg_val;
	int x, r;


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
			op = (indirect ? "I ANDA" : "ANDA");
			reg_name = 'A';
			reg_val = a;
			x = mem[address];
			r = a & x;
			a = r & 0xFFFF;
			time++;
			break;
		case 1:
			op = (indirect ? "I ANDB" : "ANDB");
			reg_name = 'B';
			reg_val = b;
			x = mem[address];
			r = b & x;
			b = r & 0xFFFF;
			time++;
			break;
		case 2:
			op = (indirect ? "I ANDC" : "ANDC");
			reg_name = 'C';
			reg_val = c;
			x = mem[address];
			r = c & x;
			c = r & 0xFFFF;
			time++;
			break;
		case 3:
			op = (indirect ? "I ANDD" : "ANDD");
			reg_name = 'D';
			reg_val = d;
			x = mem[address];
			r = d & x;
			d = r & 0xFFFF;
			time++;
			break;
	}

	if(verbose){
		fprintf(stderr, verbose_format, time, pc, instruction, op);
		fprintf(stderr, ": %c -> 0x%04x, M[0x%04x] -> 0x%04x, 0x%04x -> %c\n", 
			reg_name, reg_val, address, x, r, reg_name);
	}

	pc++;
}


void or(int instruction){
	int indirect = instruction & 0x0200;
	int address = 0;
	char *op;
	char reg_name;
	int reg_val;
	int x, r;


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
			op = (indirect ? "I ORA" : "ORA");
			reg_name = 'A';
			reg_val = a;
			x = mem[address];
			r = a | x;
			a = r & 0xFFFF;
			time++;
			break;
		case 1:
			op = (indirect ? "I ORB" : "ORB");
			reg_name = 'B';
			reg_val = b;
			x = mem[address];
			r = b | x;
			b = r & 0xFFFF;
			time++;
			break;
		case 2:
			op = (indirect ? "I ORC" : "ORC");
			reg_name = 'C';
			reg_val = c;
			x = mem[address];
			r = c | x;
			c = r & 0xFFFF;
			time++;
			break;
		case 3:
			op = (indirect ? "I ORD" : "ORD");
			reg_name = 'D';
			reg_val = d;
			x = mem[address];
			r = d | x;
			d = r & 0xFFFF;
			time++;
			break;
	}

	if(verbose){
		fprintf(stderr, verbose_format, time, pc, instruction, op);
		fprintf(stderr, ": %c -> 0x%04x, M[0x%04x] -> 0x%04x, 0x%04x -> %c\n", 
			reg_name, reg_val, address, x, r, reg_name);
	}

	pc++;
}


void xor(int instruction){
	int indirect = instruction & 0x0200;
	int address = 0;
	char *op;
	char reg_name;
	int reg_val;
	int x, r;


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
			op = (indirect ? "I XORA" : "XORA");
			reg_name = 'A';
			reg_val = a;
			x = mem[address];
			r = a ^ x;
			a = r & 0xFFFF;
			time++;
			break;
		case 1:
			op = (indirect ? "I XORB" : "XORB");
			reg_name = 'B';
			reg_val = b;
			x = mem[address];
			r = b ^ x;
			b = r & 0xFFFF;
			time++;
			break;
		case 2:
			op = (indirect ? "I XORC" : "XORC");
			reg_name = 'C';
			reg_val = c;
			x = mem[address];
			r = c ^ x;
			c = r & 0xFFFF;
			time++;
			break;
		case 3:
			op = (indirect ? "I XORD" : "XORD");
			reg_name = 'D';
			reg_val = d;
			x = mem[address];
			r = d ^ x;
			d = r & 0xFFFF;
			time++;
			break;
	}

	if(verbose){
		fprintf(stderr, verbose_format, time, pc, instruction, op);
		fprintf(stderr, ": %c -> 0x%04x, M[0x%04x] -> 0x%04x, 0x%04x -> %c\n", 
			reg_name, reg_val, address, x, r, reg_name);
	}

	pc++;
}

void ld(int instruction){
	int reg_num = (instruction & 0x0C00) >> 10;
	int indirect = instruction & 0x0200;
	int old_address = 0;
	int address = 0;
	char *op;
	char reg_name;

	address += (instruction & 0x00FF);
	if(instruction & 0x0100){
		address += (pc & 0xFF00);
	}
	if(indirect){
		time++;
		old_address = address;
		address = mem[address];
	}

	switch(reg_num){
		case 0:
			op = (indirect ? "I LDA" : "LDA");
			reg_name = 'A';
			break;
		case 1:
			op = (indirect ? "I LDB" : "LDB");
			reg_name = 'B';
			break;
		case 2:
			op = (indirect ? "I LDC" : "LDC");
			reg_name = 'C';
			break;
		case 3:
			op = (indirect ? "I LDD" : "LDD");
			reg_name = 'D';
			break;

	}

	reg_arr[reg_num] = mem[address];
	time++;

	if(verbose){
		fprintf(stderr, verbose_format, time, pc, instruction, op);
		if(indirect){
			fprintf(stderr, ": M[0x%04x] -> 0x%04x, M[0x%04x] -> 0x%04x, 0x%04x -> %c\n",
				old_address, address, address, reg_arr[reg_num], reg_arr[reg_num], reg_name);
		}
		else{
			fprintf(stderr, ": M[0x%04x] -> 0x%04x, 0x%04x -> %c\n", 
				address, reg_arr[reg_num], reg_arr[reg_num], reg_name);
		}
	}

	pc++;
}

void st(int instruction){
	int indirect = instruction & 0x0200;
	int address = 0;
	char *op;
	char reg_name;
	int reg_val;
	int old_address = 0;

	address += (instruction & 0x00FF);
	if(instruction & 0x0100){
		address += (pc & 0xFF00);
	}
	if(indirect){
		time++;
		old_address = address;
		address = mem[address];
	}

	switch((instruction & 0x0C00) >> 10){
		case 0:
			op = (indirect ? "I STA" : "STA");
			reg_name = 'A';
			mem[address] = a;
			reg_val = a;
			time++;
			break;
		case 1:
			op = (indirect ? "I STB" : "STB");
			reg_name = 'B';
			mem[address] = b;
			reg_val = b;
			time++;
			break;
		case 2:
			op = (indirect ? "I STC" : "STC");
			reg_name = 'C';
			mem[address] = c;
			reg_val = c;
			time++;
			break;
		case 3:
			op = (indirect ? "I STD" : "STD");
			reg_name = 'D';
			mem[address] = d;
			reg_val = d;
			time++;
			break;
	}

	if(verbose){
		fprintf(stderr, verbose_format, time, pc, instruction, op);
		if(indirect){
			fprintf(stderr, ": M[0x%04x] -> 0x%04x, %c -> 0x%04x, 0x%04x -> M[0x%04x]\n",
				old_address, address, reg_name, reg_val, reg_val, address);
		}
		else{
			fprintf(stderr, ": %c -> 0x%04x, 0x%04x -> M[0x%04x]\n", 
				reg_name, reg_val, reg_val, address);
		}
	}

	pc++;
}


void iot(int instruction){
	char reg_name;
	char *op = "";
	int reg_val = 0;
	int device = (instruction & 0x03F8) >> 3;

	switch((instruction & 0x0C00) >> 10){
		case 0:
			reg_name = 'A';
			if(device == 3){
				op = "IOT 3";
				a = getchar();
				a = a & 0xFFFF;
			}
			else if(device == 4){
				op = "IOT 4";
				putchar(a & 0xFF);
			}
			else{
				fprintf(stderr, "Invalid IOT Device\n");
				exit(1);
			}
			reg_val = a;
			break;
		case 1:
			reg_name = 'B';
			if(device == 3){
				op = "IOT 3";
				b = getchar();
				b = b & 0xFFFF;
			}
			else if(device == 4){
				op = "IOT 4";
				putchar(b & 0xFF);
			}
			else{
				fprintf(stderr, "Invalid IOT Device\n");
				exit(1);
			}
			reg_val = b;
			break;
		case 2:
			reg_name = 'D';
			if(device == 3){
				op = "IOT 3";
				c = getchar();
				c = c & 0xFFFF;
			}
			else if(device == 4){
				op = "IOT 4";
				putchar(c & 0xFF);
			}
			else{
				fprintf(stderr, "Invalid IOT Device\n");
				exit(1);
			}
			reg_val = c;
			break;
		case 3:
			reg_name = 'D';
			if(device == 3){
				op = "IOT 3";
				d = getchar();
				d = d & 0xFFFF;
			}
			else if(device == 4){
				op = "IOT 4";
				putchar(d & 0xFF);
			}
			else{
				fprintf(stderr, "Invalid IOT Device\n");
				exit(1);
			}
			reg_val = d;
			break;
	}

	if(verbose){
		fprintf(stderr, verbose_format, time, pc, instruction, op);
		if(device == 3){
			fprintf(stderr, ": 0x%04x -> %c\n", reg_val, reg_name);
		}
		else if(device == 4){
			fprintf(stderr, ": %c -> 0x%04x\n", reg_name, reg_val);
		}
		else{
			fprintf(stderr, "\n");
		}
	}

	pc++;
}


void mov(int instruction){
	int indirect = instruction & 0x0200;
	int old_address;
	int old_val = 0;
	int address = 0;
	int old_psw = psw;
	int old_pc = 0;
	char *op = "";
	
	old_address = address;
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
			//ISZ
			op = (indirect ? "I ISZ" : "ISZ");
			pc++;
			old_val = mem[address];
			mem[address]++;
			if(mem[address] == 0){
				pc++;
			}

			if(verbose){
				fprintf(stderr, verbose_format, time, old_pc, instruction, op);
				fprintf(stderr, ": M[0x%04x] -> 0x%04x, 0x%04x, M[0x%04x]\n", 
					address, old_val, mem[address], address);
			}
			break;
		case 1:
			//JMP
			op = (indirect ? "I JMP" : "JMP");
			pc = address;

			if(verbose){
				fprintf(stderr, verbose_format, time, old_pc, instruction, op);
				if(indirect){
					fprintf(stderr, ": M[0x%04x] -> 0x%04x, 0x%04x -> PC\n", 
						address, pc, pc);
				}
				else{
					fprintf(stderr, ": 0x%04x -> PC\n", pc);
				}
			}
			break;
		case 2:
			//CALL
			if(sp < spl){
				psw = 0;
				op = (indirect ? "I CALL Stack Overflow" : "CALL Stack Overflow");
			}
			else{
				op = (indirect ? "I CALL" : "CALL");
				mem[sp] = pc + 1;
				sp--;
				pc = address;
			}

			if(verbose){
				fprintf(stderr, verbose_format, time, pc, instruction, op);
				if(psw == 0){
					fprintf(stderr, ": M[0x%04x] -> 0x%04x, PSW -> 0x%04x, 0x%04x -> PSW\n", 
						address, mem[address], old_psw, psw);
				}
				else{
					if(indirect){				
						fprintf(stderr, ": M[0x%04x] -> 0x%04x, 0x%04x -> M[0x%04x], 0x%04x -> SP, 0x%04x -> PC\n", 
							old_address, address, old_pc + 1, sp + 1, sp, pc);
					}
					else{
						fprintf(stderr, ": 0x%04x -> M[0x%04x], 0x%04x -> SP, 0x%04x -> PC\n", 
							old_pc + 1, sp + 1, sp, pc);
					}
				}
			}

			break;
		default:
			fprintf(stderr, "Invalid Opcode (mov)\n");
			exit(1);
	}
}


void stk(int instruction){
	int indirect = instruction & 0x0200;
	int address = 0;
	int old_psw = psw;
	char *op;

	address += instruction & 0xFF;
	if(instruction & 0x0100){
		address += pc & 0xFF00;
	}
	if(indirect){
		address = mem[address];
		time++;
	}

	switch((instruction & 0x0C00) >> 10){
		case 0:
			if(sp < spl){
				//overflow
				psw = 0;
				op = indirect ? "I PUSH Stack Overflow" : "PUSH Stack Overflow";
				fprintf(stderr, "Stack Pointer = 0x%04x; Stack Limit = 0x%04x\n", sp, spl);
			}
			else{
				//push
				op = indirect ? "I PUSH" : "PUSH";
				mem[sp] = mem[address];
				time += 2;
				sp--;
			}

			if(verbose){
				fprintf(stderr, verbose_format, time, pc, instruction, op);
				if(psw == 0){
					fprintf(stderr, ": M[0x%04x] -> 0x%04x, PSW -> 0x%04x, 0x%04x -> PSW\n", 
						address, mem[address], old_psw, psw);
				}
				else{
					fprintf(stderr, ": M[0x%04x] -> 0x%04x, 0x%04x -> M[0x%04x], 0x%04x -> SP\n", 
						address, mem[address], mem[address], sp + 1, sp);
				}
			}

			break;
		case 1:
			if(sp >= 0xFFFF){
				//underflow
			}
			else{
				//pop
			}
			break;
		default:
			fprintf(stderr, "Invalid Opcode (stk)\n");
			exit(1);
	}
}


void reg(int instruction){
	int reg1 = (instruction & 0x01C0) >> 6;
	int reg2 = (instruction & 0x0038) >> 3;
	int reg3 = instruction & 0x0007;
	char *name1;
	char *name2;
	char *name3;
	char *op;

	switch((instruction & 0x0E00) >> 9){
		case 0:
			//MOD
			op = "MOD";
			reg_arr[reg1] = reg_arr[reg2] % reg_arr[reg3];
			reg_arr[reg1] &= 0xFFFF;
			break;
		case 1:
			//ADD
			op = "ADD";
			reg_arr[reg1] = reg_arr[reg2] + reg_arr[reg3];
			if(((reg_arr[reg2] & 0x8000) == (reg_arr[reg3] & 0x8000)) && 
				((reg_arr[reg2] & 0x8000) != (reg_arr[reg1] & 0x8000)))
				link_bit = (link_bit ? 0 : 1);
			reg_arr[reg1] &= 0xFFFF;
			break;
		case 2:
			//SUB
			op = "SUB";
			reg_arr[reg1] = reg_arr[reg2] - reg_arr[reg3];
			if(((reg_arr[reg2] & 0x8000) == (reg_arr[reg3] & 0x8000)) && 
				((reg_arr[reg2] & 0x8000) != (reg_arr[reg1] & 0x8000)))
				link_bit = (link_bit ? 0 : 1);
			reg_arr[reg1] &= 0xFFFF;
			break;
		case 3:
			//MUL
			op = "MUL";
			reg_arr[reg1] = reg_arr[reg2] * reg_arr[reg3];
			if(((reg_arr[reg2] & 0x8000) == (reg_arr[reg3] & 0x8000)) && 
				((reg_arr[reg2] & 0x8000) != (reg_arr[reg1] & 0x8000)))
				link_bit = (link_bit ? 0 : 1);
			reg_arr[reg1] &= 0xFFFF;
			break;
		case 4:
			//DIV
			op = "DIV";
			reg_arr[reg1] = reg_arr[reg2] / reg_arr[reg3];
			reg_arr[reg1] &= 0xFFFF;
			break;
		case 5:
			//AND
			op = "AND";
			reg_arr[reg1] = reg_arr[reg2] & reg_arr[reg3];
			reg_arr[reg1] &= 0xFFFF;
			break;
		case 6:
			//OR
			op = "OR";
			reg_arr[reg1] = reg_arr[reg2] | reg_arr[reg3];
			reg_arr[reg1] &= 0xFFFF;
			break;
		case 7:
			//XOR
			op = "XOR";
			reg_arr[reg1] = reg_arr[reg2] ^ reg_arr[reg3];
			reg_arr[reg1] &= 0xFFFF;
			break;
		default:
			fprintf(stderr, "Invalid Opcode (reg)\n");
			exit(1);
	}

	switch(reg1){
		case 0:
			name1 = "A";
			break;
		case 1:
			name1 = "B";
			break;
		case 2:
			name1 = "C";
			break;
		case 3:
			name1 = "D";
			break;
		case 4:
			name1 = "PC";
			break;
		case 5:
			name1 = "PCW";
			break;
		case 6:
			name1 = "SP";
			break;
		case 7:
			name1 = "SPL";
			break;
		default:
			fprintf(stderr, "Invalid Register (reg1)\n");
			exit(1);
	}
	switch(reg2){
		case 0:
			name2 = "A";
			break;
		case 1:
			name2 = "B";
			break;
		case 2:
			name2 = "C";
			break;
		case 3:
			name2 = "D";
			break;
		case 4:
			name2 = "PC";
			break;
		case 5:
			name2 = "PCW";
			break;
		case 6:
			name2 = "SP";
			break;
		case 7:
			name2 = "SPL";
			break;
		default:
			fprintf(stderr, "Invalid Register (reg2)\n");
			exit(1);
	}
	switch(reg3){
		case 0:
			name3 = "A";
			break;
		case 1:
			name3 = "B";
			break;
		case 2:
			name3 = "C";
			break;
		case 3:
			name3 = "D";
			break;
		case 4:
			name3 = "PC";
			break;
		case 5:
			name3 = "PCW";
			break;
		case 6:
			name3 = "SP";
			break;
		case 7:
			name3 = "SPL";
			break;
		default:
			fprintf(stderr, "Invalid Register (reg3)\n");
			exit(1);
	}

	if(verbose){
		fprintf(stderr, verbose_format, time, pc, instruction, op);
		fprintf(stderr, ": %s -> 0x%04x, %s -> 0x%04x, 0x%04x -> %s\n", 
			name2, reg_arr[reg2], name3, reg_arr[reg3], reg_arr[reg1], name1);
	}

	pc++;
}


void opr(int instruction){
	//work
}
