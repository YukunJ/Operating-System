#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define MAX_ADDRESS 299       /* the highest address we are allowed */
#define MAX_VAR_NAME  8       /* the maximum length of a symbol */
#define MAX_VAR  300          /* the maximum number of symbols */
#define MAX_INSTRUCTION 300   /* the maximum number of instructions */

typedef struct Symbol
{ // this is the pseudo-class of each symbol
	char var_name[MAX_VAR_NAME+1]; // the name of a symbol
	int relative_address; // its relative address
	int absolute_address; // its absolute address
	int belong; // in which module is it defined
	int define_size; // the size of that module in which it's defined
	int used; // to detect whether it's used or not
	int multiply; // to detect whether it's multiply defined or not
	int exceed_module; // to flag whether this symbol once exceed the size of its module
	int toolong; // to flag whether the symbol name is cut down because too long(8-character at most)
} symbol;

typedef struct Instruction
{ // this is the pseudo-class of each instruction
	int instruction; // the original 5 digits instruction
	int base; // the base address of the module in which this instruction resides
	int to_print; // after two-pass-linker, the memory address to print
	int not_defined; // detect whether a symbol is used without definition
	char use_var_name[MAX_VAR_NAME+1]; // possible symbol usage in this instruction
	int not_exceed;
	int not_multiple;
} instruction;

int search_mul_defined(char * src, symbol * symbol_lst)
{ //helper function
	// @ input: src: the symbol name we want to find whose absolute address
	//					symbol_lst: the whole symbol table
	// @ output: -1 indicates not multiply defined, good case
	//          other number indicate the index of symbol table of which is multiply defined
	int i;
	for (i = 0 ;i < MAX_VAR ; i++)
	{
		if ( strcmp(src, symbol_lst[i].var_name) == 0)
		{
			symbol_lst[i].multiply = 1; // to indicate it's multiply defined once
			return i;
		}
	}
	return -1; // not multiply defined
}
int search_var_address(char * src, symbol * symbol_lst)
{ //  helper function
	// @ input: src: the symbol name we want to find whose absolute address
	//					symbol_lst: the whole symbol table
	// @ output: int, the absolute address of the symbol src
  //         if -1 ,indicates finding failure
	int i;
	for (i = 0 ;i < MAX_VAR ;i++)
	{
		if ( strcmp(src,symbol_lst[i].var_name) == 0)
		{
			symbol_lst[i].used = 1; // to indicate it's used
			return symbol_lst[i].absolute_address;
		}
	}
	return -1; // failure finding
}

int search_index(char * src, symbol * symbol_lst)
{
	//helper function
	// @ input: src: the symbol name we want to find whose index in symbol table
	//					symbol_lst: the whole symbol table
	// @ output: int, the index of that symbol in symbol table
  //         if -1 ,indicates finding failure
	int i;
	for (i = 0 ;i < MAX_VAR ;i++)
	{
		if (strcmp(src, symbol_lst[i].var_name) == 0)
			return i;
	}
	return -1;
}
int main()
{
	int num_module = 0; /* count of how many modules we will have */
	int num_symbol = 0; /* count of how many symbols we will define */
	symbol symbol_lst[MAX_VAR]; /* the symbol table */
	instruction instruction_lst[MAX_INSTRUCTION]; /* the instruction table */
	int i,j,k; /* dummy variable */
	
	// default initialize symbol table & instruction table
	for (i=0;i<MAX_VAR;i++)
	{
		strcpy(symbol_lst[i].var_name, "None");
		symbol_lst[i].relative_address = -1;
		symbol_lst[i].absolute_address = -1;
		symbol_lst[i].used = 0;
		symbol_lst[i].multiply = 0;
		symbol_lst[i].exceed_module = 0;
		symbol_lst[i].toolong = 0;
	}
	for (i=0;i<MAX_INSTRUCTION;i++)
	{
		instruction_lst[i].instruction = -1;
		instruction_lst[i].to_print = -1;
		instruction_lst[i].not_defined = 0;
		instruction_lst[i].not_exceed = 0;
		instruction_lst[i].not_multiple = 0;
		strcpy(instruction_lst[i].use_var_name, "None");
	}
	printf("---------------------------------------------\n");
	printf("Welcome to Two-Pass Linker. Start your input:\n");
	scanf("%d", &num_module); /* get how many modules we will have */
	int module_count = 0;
	int base_symbol[num_module]; /* the base number of symbols starting at each module */
	int base_address[num_module]; /* the base address table for each module */
	int base_count = 0; /* count of how many instructions given so far */
	int def_num; /* how many symbols defined so far */
	int use_num; /* how many symbols used so far in current module */
	int instruction_num; /* how many insturctions in each module scanned */
	int instruct; /* the 5-digit instruction scanned */
	char name[MAX_VAR_NAME+1]; /* the symbol name scanned */
	int use_address; /* in which instruction the symbol is used */
	
	
	
	/* -----------First Pass----------- */
	int mul_define;
	char current_name_buffer[500];
	int current_relative_address;
	for ( module_count=0; module_count < num_module; module_count++)
	{
		base_symbol[module_count] = num_symbol;
		base_address[module_count] = base_count; /* update the base address for this module */
		scanf("%d",&def_num);
		for (i=0;i<def_num;i++)
		{ // process all the symbols defined in this module
			scanf("%s", current_name_buffer);
			if (strlen(current_name_buffer)>= 8) /* too long a symbol name, cut it down to first 8 */
			{
				current_name_buffer[8] = '\0'; /* only take first 8 characters */
				symbol_lst[num_symbol].toolong = 1;
			}
			scanf("%d", &current_relative_address);
			mul_define = search_mul_defined(current_name_buffer, symbol_lst);
			if (mul_define == -1)
			{ // not multiply defined
				strcpy(symbol_lst[num_symbol].var_name,current_name_buffer);
				symbol_lst[num_symbol].relative_address = current_relative_address;
				symbol_lst[num_symbol].belong = module_count;
				num_symbol += 1;
			}
			else
			{ //  is multiply defined
				symbol_lst[mul_define].relative_address = current_relative_address;
				symbol_lst[mul_define].belong = module_count;
			}
		}
		
		scanf("%d", &use_num);

		for (i=0; i<use_num;i++)
		{ // process all the symbols used in this module
			scanf("%s",current_name_buffer);
			current_name_buffer[8] = '\0'; /* allow at most 8-character long symbol name */
			scanf("%d",&use_address);
			while( use_address != -1)
			{  // in the instruction, indicate that there is a symbol associated
				if (strcmp(instruction_lst[base_count + use_address].use_var_name, "None") != 0) /* multiple usage in same instruction detected */
					instruction_lst[base_count + use_address].not_multiple = 1;
				strcpy(instruction_lst[base_count + use_address].use_var_name, current_name_buffer);
				scanf("%d",&use_address);
			}
		}
		
		scanf("%d", &instruction_num);
		//go back and check if any defintion exceed the size of this module
		for (i = 0 ; i < num_symbol ; i++)
		{
			if (symbol_lst[i].belong == module_count) /* only cencern in those updated in this module */
			{
				if (symbol_lst[i].relative_address >= instruction_num)
				{
					symbol_lst[i].exceed_module = 1;
					symbol_lst[i].relative_address = instruction_num - 1; /* last word in that module */
				}
			}
		}
		
		for (i = 0 ;i < instruction_num; i++)
		{ // process all the instructions
			scanf("%d", &instruct);
			instruction_lst[base_count+i].instruction = instruct;
			instruction_lst[base_count+i].base = base_count;
		}

		base_count += instruction_num; // update the base address
	}
	
  /* ------- figure out each symbol's absolute address -------- */
	for (i = 0 ;i < num_symbol ;i++)
	{
		symbol_lst[i].absolute_address = base_address[symbol_lst[i].belong] + symbol_lst[i].relative_address;
	}
	
	/* ---------- Second Pass ----------- */
	int oper_type; /* the operation type 1,2,3,4 */
	int find_flag; /* use to record whether a symbol is used but never defined */
	char external_name[MAX_VAR_NAME+1]; /* the name of the variable we want to look for its address */
	for (i = 0 ;i < base_count ;i++)
	{ // iterate through each insturction
		oper_type = instruction_lst[i].instruction % 10;
		switch (oper_type)
		{
			case 1: /* immediate */
				instruction_lst[i].to_print = instruction_lst[i].instruction/10;
				break;
			case 2: /* absolute */
				instruction_lst[i].to_print = instruction_lst[i].instruction/10;
				break;
			case 3: /* relative */
				instruction_lst[i].to_print = instruction_lst[i].instruction/10 + instruction_lst[i].base;
				break;
			case 4: /* external */
				strcpy(external_name, instruction_lst[i].use_var_name);
				find_flag = search_var_address(external_name,symbol_lst); // call helper function to find the absolute address
				if (find_flag != -1) // the symbol used is already defined
					instruction_lst[i].to_print = 1000 * (instruction_lst[i].instruction / 10000) + find_flag;
				else
				{
					instruction_lst[i].to_print = 1000 * (instruction_lst[i].instruction / 10000) + 111; /* not properly defined, use 111 */
					instruction_lst[i].not_defined = 1; // set up flag to indicate it's not defined
				}
				break;
			default:
				printf("Not supposed to enter here.\n");
				break;
		}
	}
	
	// finish Two-Pass Processing, starting to output
	printf("\nSymbol Table\n");
	for (i = 0 ;i < num_symbol ;i++)
	{
		printf("%s=%d ",symbol_lst[i].var_name, symbol_lst[i].absolute_address);
		if (symbol_lst[i].toolong == 1) /* original name too long and get cut down */
		{
			printf(" Warning: Original symbol name too long; cut down to only first 8 characters. ");
		}
		if (symbol_lst[i].exceed_module == 1) /* once exceed the size of its module */
		{
			printf(" Error: Definition exceeds module size; last word in module used. ");
		}
		if (symbol_lst[i].multiply == 1) /* once multiply defined */
		{
			printf(" Error: This variable is multiply defined; last value used. ");
		}
		printf("\n");
	}
	printf("\nMemory Map\n");
	for (i = 0 ;i < base_count ;i++)
	{
		printf("%d: ",i);
		if (i < 10) // to look beautiful
			printf(" ");
		if (instruction_lst[i].to_print % 1000 > MAX_ADDRESS && (instruction_lst[i].instruction % 10) != 1) /* exceed the max address of the machine 300 and is not of type 1 immediate*/
		{
			printf("%d ", 1000* (instruction_lst[i].to_print / 1000)+ 299); /* retain the first operand, the rest put 299 */
			printf("Error: Absolute address exceeds machine size; largest legal value used. ");
		}
		else
		{
		printf("%d",instruction_lst[i].to_print);
		}
		if (instruction_lst[i].not_multiple == 1) // multiple symbols are used in this instruction
		{
			printf(" Error: Multiple variables used in instruction; all but last ignored. ");
		}
		if (instruction_lst[i].not_defined == 1) // this symbol used is not properly defined before
		{
			printf(" Error: %s is not defined; 111 used. ",instruction_lst[i].use_var_name);
		}
		printf("\n");
	}
	 // check if every symbol is used somewhere
	for (i = 0 ;i < num_symbol ;i++)
	{
		if (symbol_lst[i].used == 0)
		{
			printf("Warning: %s was defined in module %-2d but never used.\n",symbol_lst[i].var_name, symbol_lst[i].belong);
		}
	}
	return 0;
}

