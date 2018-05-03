/*
* Disha Gupta
* Operating Systems - Lab 4 (Demand Paging)
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stdbool.h>

// Constants
#define MAX_PROCESS 10 		// maximum number of processes
#define	MAX_VIRTUAL 100 	// maximum number of virtual pages		
#define	MAX_PHYSICAL 100 	// maximum number of physical pages			
#define MAX_INT 2147483647

int	random_no(FILE *fp, float param1, float param2, float param3); 
int	random_val(FILE *fp_rand, int param);
	
int main(int argc, char * argv[])
{
	FILE *fp_rand; 
	int index, index1, index2, indexP, indexF, indexV, indexp, cnt, cnt1, cnt2, cnt3, flag, indexF2;
	int	clock, min, max;  
	int	M; 					// machine size in words
	int	P; 					// page size in words
	int	S; 					// size of each process
	int	J; 					// job mix
	int	N; 					// number of references for each process
	int	V; 					// verbose flag
	int	Q; 					// number of round robin references per process
	char R[10];				// replacement algorithm
	int process_no; 		// number of processes
	float A[MAX_PROCESS], B[MAX_PROCESS], C[MAX_PROCESS], avg; 			
						
	// parameters for probability calculations
	struct	PT{
		int	FT_index;
		int	valid; 
		int	referenced; 	// is page referenced (not required)
		int	residency;		// page residency time
	} page_table[MAX_PROCESS][MAX_VIRTUAL];

	struct	FT{
		int	process;		// process number
		int	page;			// page number
		int	valid; 
		int	referenced; 
	} frame_table[MAX_PHYSICAL];

	struct PS{
		int	residency; 		// running sum of residency time
		int	faults; 		// running sum of number of faults
		int	reference_word;	// reference index for each process
		int	evictions; 
	} process_stats[MAX_PROCESS]; 

	int	actual_virtual; 	// actual number of virtual pages
	int	actual_physical;	// actual number of physical pages

	// check command line input
	if((argc < 7) | (argc > 8)){
      	printf("usage: <program-name> M P S J N R <V> \n");
      	printf("M : machine size in words \n");
  		printf("P : page size in words \n");
  		printf("S : size of each process \n");
  		printf("J : job mix (1 to 4) \n");
  		printf("N : number of references for each process \n");
  		printf("R : replacement algorithm - FIFO, RANDOM or LRU \n");
		printf("V : verbose flag \n"); 
     		exit(1);
  	}

	// echo input
	M = atoi(argv[1]);
	P = atoi(argv[2]);
	S = atoi(argv[3]);
	J = atoi(argv[4]);
	N = atoi(argv[5]);
	strcpy(R, argv[6]); 
	V = 0; 

	printf("The machine size is %3d.\n", M); 
	printf("The page size is %3d.\n", P);
	printf("The process size is %3d.\n", S);
	printf("The job mix number is %2d.\n", J); 
	printf("The number of references per process is %3d.\n", N);
	printf("The replacement algorithm is %6s.\n", R); 
	printf("The level of debugging output is %2d.\n", V); 

 	// open files for reading and writing
	if (!(fp_rand = fopen("RandomNumbers.txt","r")) ){
  		printf("Cannot open Random Number file.\n");
  		exit(1);
  	}

	// set parameters
	switch (J) {
		case 1: 
			process_no = 1; 
			for (cnt = 0; cnt < P; cnt++){
				A[cnt] = 1.0; B[cnt] = 0; C[cnt] = 0;
			} 
			break;
		case 2: 
			process_no = 4; 
			for (cnt = 0; cnt < P; cnt++){
				A[cnt] = 1.0; B[cnt] = 0; C[cnt] = 0;
			} 
			break;
		case 3: 
			process_no = 4; 
			for (cnt = 0; cnt < P; cnt++){
				A[cnt] = 0; B[cnt] = 0; C[cnt] = 0;
			} 
			break;
		case 4: 
			process_no = 4; 
			A[0] = 0.75; B[0] = 0.25; C[0] = 0.00;
			A[1] = 0.75; B[1] = 0.00; C[1] = 0.25;
			A[2] = 0.75; B[2] = 0.125; C[2] = 0.125;
			A[3] = 0.5; B[3] = 0.125; C[3] = 0.125;
			break;
	}

	actual_virtual = S / P; 
	actual_physical = M / P; 

	// initialize variables
	for (cnt1 = 0; cnt1 < MAX_PROCESS; cnt1++){
		for (cnt2 = 0; cnt2 < MAX_VIRTUAL; cnt2++){ 
			page_table[cnt1][cnt2].FT_index = 0; 
			page_table[cnt1][cnt2].valid = 0; 
			page_table[cnt1][cnt2].referenced = 0;
			page_table[cnt1][cnt2].residency = 0;
		}
	}

	for (cnt = 0; cnt < MAX_PHYSICAL; cnt++){
		frame_table[cnt].process = 0; 
		frame_table[cnt].page = 0;
		frame_table[cnt].valid = 0;
		frame_table[cnt].referenced = 0; 	 
	}

	for (cnt = 0; cnt < MAX_PROCESS; cnt++){
		process_stats[cnt].residency = 0;
		process_stats[cnt].faults = 0;
		process_stats[cnt].reference_word = S - P + 1; 
		process_stats[cnt].evictions = 0;
	}

	Q = 3; 

	// start demand paging simulation

	clock = 0; 
	indexF2 = actual_physical - 1; 

	while(clock < N){
		// for each process
		for (cnt1 = 0; cnt1 < process_no; cnt1++){
			// for round robin with quantum 3
			for (cnt2 = 0; (cnt2 < Q) ; cnt2++){
				if ((clock + cnt2) >= N) break; 
				// simulate current reference
				// get virtual page
				indexV = (int) (process_stats[cnt1].reference_word) / P; 
				// check if page loaded
				flag = 0; 

				if (!page_table[cnt1][indexV].valid){
					process_stats[cnt1].faults++;
					// find a free frame
					for (cnt = (actual_physical -1); cnt >= 0; cnt--){
						indexF = cnt; 
						if (frame_table[cnt].valid == 0){
							// load new page table into frame
							frame_table[indexF].valid = 1; 
							frame_table[indexF].process = cnt1; 
							frame_table[indexF].page = indexV; 
							frame_table[indexF].referenced = clock + cnt2; 
							// update page table entries
							page_table[cnt1][indexV].valid = 1; 
							page_table[cnt1][indexV].FT_index = cnt; 
							page_table[cnt1][indexV].referenced = 1; 
							page_table[cnt1][indexV].residency = 1; 
							cnt = -1; 
							flag = 1; 
						}		
					}

					// else find victim frame
					if (!flag){
						process_stats[cnt1].evictions++;
						if ((!strcmp(R, "lru")) | (!strcmp(R, "LRU"))){
							min = MAX_INT; 
							// find frame with lru
							for (cnt = 0; cnt < actual_physical; cnt++){
								if (frame_table[cnt].referenced <= min){
									min = frame_table[cnt].referenced; 
									indexF = cnt; 
								} 
							} 
						}
						else if ((!strcmp(R, "fifo")) | (!strcmp(R, "FIFO"))){
							indexF = indexF2; 
							indexF2 = (indexF2 - 1 + actual_physical) % actual_physical;
						}
						else if ((!strcmp(R, "random")) | (!strcmp(R, "RANDOM"))){
							indexF = random_val(fp_rand, actual_physical);
						} 

						// invalidate corresponding page table entries
						indexP = frame_table[indexF].process; 
						indexp = frame_table[indexF].page;
						page_table[indexP][indexp].valid = 0;
						process_stats[indexP].residency += page_table[indexP][indexp].residency; 
						// running sum of residency for process
						page_table[indexP][indexp].residency = 0; 
						// load new page table into frame
						frame_table[indexF].valid = 1; 
						frame_table[indexF].process = cnt1; 
						frame_table[indexF].page = indexV; 
						frame_table[indexF].referenced = clock + cnt2; 
						// update page table entries
						page_table[cnt1][indexV].valid = 1; 
						page_table[cnt1][indexV].FT_index = indexF; 
						page_table[cnt1][indexV].referenced = 1; 
						page_table[cnt1][indexV].residency = 1;
					}
				}
				// update stats for valid page
				else{ 
					page_table[cnt1][indexV].residency++; 	
					indexF = page_table[cnt1][indexV].FT_index; 
					frame_table[indexF].referenced = clock + cnt2; 
				}
				
				// calculate next reference
				index = random_no(fp_rand, A[cnt1], B[cnt1], C[cnt1]);
				switch (index) {
					case 1 : 
						process_stats[cnt1].reference_word = (process_stats[cnt1].reference_word + 1) % S;
						break; 
					case 2 : 
						process_stats[cnt1].reference_word = (process_stats[cnt1].reference_word -5 + S) % S;
						break;
					case 3 : 
						process_stats[cnt1].reference_word = (process_stats[cnt1].reference_word + 4) % S;
						break;
					case 4 : 
						process_stats[cnt1].reference_word = random_val(fp_rand, S); 
						break;
				}				

			}
		}
		clock += cnt2;	
	
	}

	// add for the last process
	cnt1 -=1; 
	process_stats[cnt1].residency += page_table[cnt1][indexV].residency;
	if (process_stats[cnt1].reference_word >= N) 
		process_stats[cnt1].evictions++; 

	// print stats
	for (cnt = 0, cnt1 = 0, cnt2 = 0, cnt3 = 0; cnt < process_no; cnt++){
		if (process_stats[cnt].evictions > 0){
			avg = (float) process_stats[cnt].residency / (float) process_stats[cnt].evictions ;
			printf("Process %2d had %2d faults and %4.1f average residency.\n", cnt+1, process_stats[cnt].faults, avg);
		}
		else{
			printf("Process %2d had %2d faults.\n", cnt+1, process_stats[cnt].faults);
			printf("With no evictions, the average residence is undefined\n");
		}
		cnt1 += process_stats[cnt].evictions;
		cnt2 += (process_stats[cnt].residency -1);
		cnt3 += process_stats[cnt].faults; 	
	}

	if (cnt1 > 0)
		printf("Total number of faults is %2d and the overall average residency is %4.1f\n", cnt3, ((float) cnt2 / (float) cnt1)); 
	else{
		printf("Total number of faults is %2d\n", cnt3);
		printf("With no evictions, the overall average residence is undefined\n");
	}

	fclose(fp_rand);
	return 0; 
}

int	random_no(FILE *fp_rand, float A, float B, float C)
{
	int r, c; 
	double y;
	
	fscanf(fp_rand, "%d", &r); 
	y = r/(MAX_INT + 1.0);
	if (y < A) 
		c = 1;
	else if (y < (A+B))
		c = 2;
	else if (y < (A+B+C))
		c = 3; 
	else
		c = 4; 

	return(c);
}

int	random_val(FILE *fp_rand, int S)
{
	int r, c; 
	double y;
	
	fscanf(fp_rand, "%d", &r); 
	y = r/(MAX_INT + 1.0);
	c = y * S;
	return(c);
}