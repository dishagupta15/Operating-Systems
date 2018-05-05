/*
* Disha Gupta
* Operating Systems - Lab 1 (Linker)
*/

// Libraries
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stdbool.h>

// Constants
#define MEMORY_SIZE 200 // Target machine memory size.
#define MAX_MODULES 10 // Max number of modules in input file.	
#define MAX_SYMBOLS 10 // Max number of symbols in each module.
#define MAX_INSTRUCTIONS 20 // Max number of instructions in each module.

// Functions
void print_errors(int, char symbol[10]); 

int main(int argc, char * argv[])
{
  	FILE *fp_in, *fp_out;
	int module, NUM_MODULES; 
	int	address, max_address, base_address; 
	int	cnt, cnt1, cnt2;
	int symbol_cnt, max_symbol;
	char symbol[MAX_SYMBOLS];
	char lsb; 
	char range_error_symbol[10];
	int	range_error_flag, range_error_module; 
	int E_ref_flag[MAX_MODULES], E_ref_module[MAX_MODULES];
	char E_ref_symbol[MAX_MODULES][20];
	bool symbol_found; 

	// Define variables to read the input file and store for processing.
	struct SYMBOL_D{
		char name[20]; // Name of symbol.
		int offset; // Offset of symbol in module (from input file).
		int absolute; // Absolute value of symbol after resolving base address.
		bool used; // Flag to mark if defined symbol is referenced in any module (used for error handling).
	} Symbol_Defined[MAX_MODULES][MAX_SYMBOLS];

	char Symbol_Referenced[MAX_MODULES][MAX_SYMBOLS][20];  

	struct INSTR{	
		char type[10]; // Type of address: I, A, R, or E (stored as string).
		char value[10]; // 4 digit instructions (stored as string).
	} instructions[MAX_MODULES][MAX_INSTRUCTIONS];

	int	num_instructions[MAX_MODULES]; 
	
	// Define variables for linker output.
	struct Symbol_T{ 
		char name[20]; // Name of symbol.
		int	absolute; // Absolute value of symbol after resolving base address.
		int count; // Number of times a symbol is defined.
	} Symbol_Table[MAX_SYMBOLS];

	char Memory_Map[MEMORY_SIZE][10]; // Memory map is array of instructions stored as strings.
	int Error_Code[MEMORY_SIZE]; // Error code for instructions.
	char Symbol_Code[MEMORY_SIZE][10];

	// Initialize the defined symbol table values.
	for (cnt1 = 0; cnt1 < MAX_MODULES; cnt1++){
		for (cnt2 = 0; cnt2 < MAX_SYMBOLS; cnt2++){
			Symbol_Defined[cnt1][cnt2].offset = -1; 
			Symbol_Defined[cnt1][cnt2].absolute = -1;
		}
	}

	for (cnt = 0; cnt < MAX_SYMBOLS; cnt++)
	{
		Symbol_Table[cnt].absolute = -1;  
		Symbol_Table[cnt].count = 0;
	}

	// Initialize the Memory_Map and Error_Code.
	for (cnt=0; cnt < MEMORY_SIZE; cnt++)
	{
		strcpy(Memory_Map[cnt], "\0"); 
		Error_Code[cnt] = 0;
	}

	// Initialize referenced symbol entries to null.
	for (cnt1 = 0; cnt1 < MAX_MODULES; cnt1++)
		for (cnt2 = 0; cnt2 < MAX_SYMBOLS; cnt2++)
			strcpy(Symbol_Referenced[cnt1][cnt2], "\0"); 
		
	// Initialize num_instructions for each module.
	for (cnt=0; cnt< MAX_MODULES; cnt++)
		num_instructions[cnt]= 0; 
 
	// Check command line input.
	if(argc != 2){
      		printf("usage: ./input_filename output_filename\n");
      		printf("input_filename: the name of the file that contains the input program modules\n");
     		exit(1);
  	}

  	// Open files for reading and writing.
  	if (!(fp_in = fopen(argv[1],"r")) ){
      		printf("Cannot open file %s\n", argv[1]);
      		exit(1);
  	}

  	// Read input file and store the modules.
 	fscanf(fp_in, "%d", &NUM_MODULES);
		
	// Read each module.
	for (module = 0; module < NUM_MODULES; module++)
	{ 
		// Read number of symbols declared and store in declared symbol table.
		fscanf(fp_in, "%d", &cnt1);
		for (cnt = 0; cnt < cnt1; cnt++)
			fscanf(fp_in, "%s %d", &Symbol_Defined[module][cnt].name, &Symbol_Defined[module][cnt].offset);

		// Read number of symbols referenced and store in referenced symbol table.
		fscanf(fp_in, "%d", &cnt2);
		for (cnt = 0; cnt < cnt2; cnt++)
			fscanf(fp_in, "%s", &Symbol_Referenced[module][cnt]);

		// Read instructions.
		fscanf(fp_in, "%d", &num_instructions[module]);	
		for (cnt = 0; cnt < num_instructions[module]; cnt++) 
			fscanf(fp_in, "%s %s ", &instructions[module][cnt].type, &instructions[module][cnt].value);		
	}

	// First pass (resolve and print addresses for symbols defined).
	module = 0; 
	base_address = 0; 
	symbol_cnt = 0;
	range_error_flag = 0;
	range_error_module = 0;

	strcpy(range_error_symbol, "/0");
	printf("\n Symbol Table \n");

	while (num_instructions[module] != 0)
	{
		cnt = 0; 
		while (Symbol_Defined[module][cnt].offset != -1)
		{
			Symbol_Defined[module][cnt].used = false; // Set used flag to false.
			
			// Check if offset is within range.
			if (Symbol_Defined [module][cnt].offset > num_instructions[module])
			{
				Symbol_Defined [module][cnt].offset = 0;
				range_error_flag = 1;
				range_error_module = 1;
				strcpy(range_error_symbol, Symbol_Defined[module][cnt].name);
			}
			
			Symbol_Defined[module][cnt].absolute = Symbol_Defined [module][cnt].offset + base_address;
			strcpy(Symbol_Table[symbol_cnt].name, Symbol_Defined[module][cnt].name); 
			Symbol_Table[symbol_cnt].absolute = Symbol_Defined[module][cnt].absolute;

			// Check if offset is within range.
			if (Symbol_Defined [module][cnt].offset > num_instructions[module])
				Symbol_Defined [module][cnt].offset = 0;
		
			printf("%4s = %2d \n", Symbol_Defined[module][cnt].name, Symbol_Defined [module][cnt].absolute);
			cnt++;
			symbol_cnt++; 
		}
		base_address = base_address + num_instructions[module];
		module++;
	}
	max_symbol = symbol_cnt;

	if (range_error_flag)
		printf ("\n Error: In module %2d the def of %4s exceeds the module size, zero (relative) used.\n", (range_error_module + 1), range_error_symbol);

	// Check for symbols defined multiple times.
	for (cnt = 0; cnt < max_symbol; cnt++)
	{
		for (cnt1 = 0; cnt1 < NUM_MODULES; cnt1++)
		{
			cnt2 = 0;
			while (Symbol_Defined[cnt1][cnt2].offset != -1)
			{
				if (!strcmp(Symbol_Defined[cnt1][cnt2].name, Symbol_Table[cnt].name))
					Symbol_Table[cnt].count++; 
				cnt2++; 
			}
		}
	}
	
	for (cnt=0; cnt< max_symbol; cnt++)
	{
		if (Symbol_Table[cnt].count > 1)
		{
			printf (" WARNING: Symbol %4s is multiply defined; first value used. \n", Symbol_Table[cnt].name );
			cnt = max_symbol; 
		}
	}

	// First pass (create and print initial memory map).
	module = 0; 
	address = 0; 
	while (num_instructions[module] != 0)
	{
		for (cnt = 0; cnt < num_instructions[module]; cnt++)
		{
			strcpy(Memory_Map[address], instructions[module][cnt].value);
			address++;
		}
	module++;
	}
		
	max_address = address; 

	// Second pass (resolve R and E memory references in Memory_Map).
	module = 0; 
	address = 0; 
	base_address = 0; 
	while (num_instructions[module] != 0)
	{
		E_ref_flag[module] = 0; 
		E_ref_module[module] = module; 
		strcpy(E_ref_symbol[module],Symbol_Referenced[module][0]);

		for (cnt = 0; cnt < num_instructions[module]; cnt++)
		{
			// Check absolute address range.
			if (!strcmp(instructions[module][cnt].type, "A"))
			{
				cnt1 = atoi(Memory_Map[address]) % 1000;
				if (cnt1 > MEMORY_SIZE)
				{ 
					Error_Code[address] = 6; 
					cnt2 = atoi(Memory_Map[address]) - cnt1; 
					sprintf(Memory_Map[address], "%d", cnt2);
				}
			}

			// Relocate Relative Addresses
			if (!strcmp(instructions[module][cnt].type, "R"))
			{
				cnt1 = atoi(Memory_Map[address]);
				cnt2 = cnt1 % 1000;
				if (cnt2 >= num_instructions[module])
				{
					Error_Code[address] = 7; 
					cnt1 = cnt1 - cnt2;
					sprintf(Memory_Map[address], "%d", cnt1);
				}
				else
				{
					cnt1 += base_address;
					sprintf(Memory_Map[address], "%d", cnt1);
				}
			}

			// Resolve External Addresses
			if (!strcmp(instructions[module][cnt].type, "E"))
			{	
				// Extract last digit of instruction to index into Symbol_Referenced.
				E_ref_flag[module] = 1; 
				lsb = instructions[module][cnt].value[3]; 
				symbol_cnt = (int) lsb & 0xf; 
				strcpy(symbol, Symbol_Referenced[module][symbol_cnt]);
				
				// Search for referenced symbol in Symbol_Table.
				symbol_found = false; 
				for (cnt1 = 0; cnt1 < max_symbol; cnt1++)
				{
					if (!strcmp(Symbol_Table[cnt1].name,symbol))
					{
						cnt2 = atoi(Memory_Map[address]) - symbol_cnt;
						cnt2 += Symbol_Table[cnt1].absolute;
						sprintf(Memory_Map[address], "%d", cnt2);
						symbol_found = true; 
						break;
					}
				}

				// Check if external address is within range.
				if (!strcmp(Symbol_Referenced[module][symbol_cnt], "\0"))
				{
					Error_Code[address] = 4; 
					strcpy(Symbol_Code[address], symbol);
				}
				else if (!symbol_found)
				{
					Error_Code[address] = 2; 
					strcpy(Symbol_Code[address], symbol);
					cnt2 = atoi(Memory_Map[address]) - symbol_cnt;
					sprintf(Memory_Map[address], "%d", cnt2);			
				}
			}

			address++; 	
		}
		base_address += num_instructions[module];
		module++; 
		symbol_cnt = 0; 
	}

	// Print Memory_Map
	printf("\n Memory Map:\n");
	for (address = 0; address < max_address; address++) 
	{			
		printf("%3d : %4s", address, Memory_Map[address]);
		print_errors(Error_Code[address], Symbol_Code[address]);
	}

	// Error Messages
	for (module = 0; module < NUM_MODULES; module++)
		if (!E_ref_flag[module])
			if (strcmp(Symbol_Referenced[module][0], "\0"))
				printf("\n Warning: In module %2d %4s appeared in the use list but was not actually used. \n", E_ref_module[module], E_ref_symbol[module]);
	
	// Check which defined symbols have been referenced.

	printf("\n");

	for (module = 0; module < NUM_MODULES; module++)
	{
		symbol_cnt = 0; 
		while (strcmp(Symbol_Referenced[module][symbol_cnt], "\0"))
		{ 
			for (cnt1 = 0; cnt1 < NUM_MODULES; cnt1++)
			{
				cnt2 = 0;
				while (Symbol_Defined[cnt1][cnt2].offset != -1)
				{
					if (!strcmp(Symbol_Referenced[module][symbol_cnt], Symbol_Defined[cnt1][cnt2].name))
						Symbol_Defined[cnt1][cnt2].used = true; 
					cnt2++; 
				}
			}
			symbol_cnt++;
		}
	}

	// Print unused symbols.
	for (module=0; module < NUM_MODULES; module++)
	{
		cnt = 0; 
		while (Symbol_Defined[module][cnt].offset != -1)
		{
			if (!Symbol_Defined[module][cnt].used)
				printf(" Warning: %4s was defined in module %2d but never used. \n", Symbol_Defined[module][cnt].name, module);  
			cnt++; 
		}
	}

	return 0;
}

void print_errors(int code, char symbol[10])
{
	switch (code)
	{
		case 0: printf("\n"); break;
		case 1: printf("\n"); break;
		case 2: printf("  Error: Symbol %4s is not defined; zero used.\n", symbol); break;
		case 4: printf("  Error: External address exceeds length of use list; treated as immediate. \n"); break;
		case 6: printf("  Error: Absolute address exceeds machine size; zero used. \n"); break;
		case 7: printf("  Error: Relative address exceeds module size; zero used. \n"); break;
		default: printf("\n"); break;
	}
}