		#include "mpi.h"
		#include <stdio.h>
		#include <stdlib.h>
		#include <string.h>
		#include <ctype.h>
		#define MAX_LEN 1024
		#define MAX_WORDS 20000
		#define MAX_NUMBER_FILE 20

		char* strlwr(char* s);
		void sortArray(char words[MAX_WORDS][50], int count[MAX_WORDS], int index);
		
		int main( int argc, char **argv )
		{
			
			
			int myrank;
			int common_size;
			double start, end;
			
			//List of files
			char file_name[MAX_LEN];
			int num_files;

			char ch;

		  	int word_count = 0, index, isUnique,i, len, firstIndex = 0,indexFile = 0, startOfAnotherPosition = 0,currentPositionInFile=0, maxWords = 0, maxResto = 0;
		  	
		  	FILE *fp;
		  	FILE *fpt;

		  	// List of distinct words
		    char words[MAX_WORDS][50];
			char word[50];
			char finalWord[MAX_WORDS][50];
			char list_file[MAX_NUMBER_FILE][20];
			char uniqueWord[MAX_WORDS][50];
		    
		    // Count of distinct words
		    int count[MAX_WORDS];
		    int finalCountWord[MAX_WORDS];
		    int uniqueCount[MAX_WORDS];

			MPI_Status status;
			MPI_Datatype typeCount, typeWord, typeNameFile;

			MPI_Init( &argc,&argv );

			MPI_Type_contiguous( MAX_WORDS, MPI_INT, &typeCount );
	    	MPI_Type_commit(&typeCount);
	    	MPI_Type_contiguous( MAX_WORDS, MPI_CHAR, &typeWord );
	    	MPI_Type_commit(&typeWord);
	    	MPI_Type_contiguous( MAX_NUMBER_FILE, MPI_CHAR, &typeNameFile );
	    	MPI_Type_commit(&typeNameFile);

			MPI_Comm_rank(MPI_COMM_WORLD, &myrank);
			MPI_Comm_size(MPI_COMM_WORLD, &common_size);
			if( myrank == 0 ) {

				printf("How many files do you want to send?\n");
				scanf("%d", &num_files);

				printf("Enter a files name:\n");
				for(int i = 0; i < num_files; i++){
					scanf("%s", list_file[i]);
				}

				start = MPI_Wtime();

				char totalWord[50];
				int maxW = 0;
				//The first operation is count how many words must you read
				int size = sizeof(list_file)/sizeof(list_file[0]);
				for (int i = 0; i < size; ++i) {	
					FILE* fp = fopen(list_file[i], "r");
					// checking if the file exist or not
					if (fp != NULL) {
					    while (fscanf(fp, "%s", totalWord) != EOF){
					    	word_count++;
					    }
					    fclose(fp);
					}	
				}

				maxWords = (maxW+=word_count)/(common_size-1);
				maxResto = (word_count)%(common_size-1);

				MPI_Bcast(&maxWords,1,MPI_INT,0,MPI_COMM_WORLD);
				MPI_Bcast(&maxResto,1,MPI_INT,0,MPI_COMM_WORLD);

				MPI_Bcast(list_file,MAX_NUMBER_FILE,typeNameFile,0,MPI_COMM_WORLD);

				int indexUnique = 0;
				
				for(int indexProcess = 1; indexProcess<common_size; indexProcess++) {
					MPI_Recv(count, 1, typeCount, indexProcess, 17, MPI_COMM_WORLD, &status);
					MPI_Recv(words, 1, typeWord, indexProcess, 25, MPI_COMM_WORLD, &status);
					MPI_Recv(&index, 1, MPI_INT, indexProcess, 90, MPI_COMM_WORLD, &status);

					if(indexProcess == 1) {
						firstIndex = index;
						for(int i = 0; i<index; i++) {
							strcpy(finalWord[i], words[i]);
							finalCountWord[i] = count[i];
						}
					} else {
						for(int i = 0; i < index; i++) {
							isUnique = 0;
							for(int j = 0; j < firstIndex; j++) {
								if(strcmp(words[i],finalWord[j]) == 0) {
									isUnique = 1;
									finalCountWord[j] = finalCountWord[j] + count[i];
								} 
							}
							//If word is unique for process insert it in the array which contains all uniqueWord
							if(isUnique == 0) {			
								strcpy(uniqueWord[indexUnique],words[i]);
								uniqueCount[indexUnique]++;
								indexUnique+=1;
							}
						}
					}
				}


				//In this for I merge the final array with the UniqueWord 
				//which contains the list of unique word of all Process
				for(int i = 0; i < indexUnique; i++){
					int isRealUnique=0;
					for(int j=0; j<index;j++) {
						if(strcmp(finalWord[j], uniqueWord[i]) == 0) {
							isRealUnique=1;
							finalCountWord[j] = finalCountWord[j] + uniqueCount[i];
						}
					}
					if(isRealUnique == 0) {
							strcpy(finalWord[index], uniqueWord[i]);
							finalCountWord[index] += uniqueCount[i];
							index++;
						}
				}

				//Function that sorts the final array
				sortArray(finalWord,finalCountWord,index);

				fpt = fopen("FinalHistograms.csv", "w+");
			    fprintf(fpt, "Words , Frequency\n");
				for (i=0; i<index; i++) {
				    if(finalCountWord[i] !=0) {
				        fprintf(fpt,"%s, %d\n", finalWord[i], finalCountWord[i]);
				    }
			   	}

			   	fclose(fpt);

			} else {

				int max_read_words = 0, resto = 0, maxParole = 0, word_count = 0;
				char wordForCount[50];
				MPI_Bcast(&maxWords,1,MPI_INT,0,MPI_COMM_WORLD);
				MPI_Bcast(&maxResto,1,MPI_INT,0,MPI_COMM_WORLD);

				MPI_Bcast(list_file,MAX_NUMBER_FILE,typeNameFile,0,MPI_COMM_WORLD);

				//If it is the last rank, add the max of words with the rest
				if(myrank == common_size-1) {
                    maxWords += maxResto;
                }

			    	int count_read_word = 0;
			    	//Initialize words count to 0
			    	for (i=0; i<MAX_WORDS; i++) {
					    count[i] = 0;
					}
					index = 0;

			    	do {

			    		//Check that the process has rank 1 in this case it opens the file at the first position of the array.
 						//If it is greater than one, it checks a series of parameters startOfAnotherPosition, indexFile, currentPositionInFile and 
 						//based on these it decides whether to open the file to the next position from the beginning or starting from a specific point 
			    		if(myrank > 1) {
			    			if(count_read_word == 0) {
			    				MPI_Recv(&startOfAnotherPosition, 1, MPI_INT, myrank-1, 30, MPI_COMM_WORLD, &status);
			    				MPI_Recv(&indexFile, 1, MPI_INT, myrank-1, 31, MPI_COMM_WORLD, &status);
			    				MPI_Recv(&currentPositionInFile, 1, MPI_INT, myrank-1, 32, MPI_COMM_WORLD, &status);
			    			}
			    			fp = fopen(list_file[indexFile], "r");
			    			if(fp == NULL) {
							   fp = fopen(list_file[indexFile-1], "r");
				  			}

				  			if(startOfAnotherPosition == 1) {
				  				fseek(fp, currentPositionInFile, SEEK_SET);
				  			}
			    		} else {
			    			fp = fopen(list_file[indexFile], "r");

				  			if(fp == NULL) {
							    printf("Unable to open file.\n");
					        	printf("Please check you have read previleges.\n");

					        	exit(EXIT_FAILURE);
				  			}
				  		}
			    		
			    		//Read the word in text file and create the array containing word and occurrence
			    		while (fscanf(fp, "%s", word) != EOF && count_read_word<maxWords ){

			    			  //Position of curson in file
			    			  currentPositionInFile = ftell(fp);
			    			  count_read_word++;

			    			  //Function that lowercase all words
							  strlwr(word);

							  //Removes the point if present next to the word
							  len = strlen(word);
					          if (ispunct(word[len - 1]) != 0){
					            	word[len - 1] = '\0';
					          }
							    // Check if word exits in list of all distinct words
							    isUnique = 1;
							    for (i=0; i<index && isUnique; i++) {
							        if (strcmp(words[i], word) == 0)
							            isUnique = 0;
							    }

							    // If word is unique then add it to distinct words list
							    // and increment index. Otherwise increment occurrence 
							    // count of current word.
							    if (isUnique) {
							        strcpy(words[index], word);
							        count[index]++;

							        index++;
							    } else {
							        count[i - 1]++;
							    }
			          	}

			          
			          //Check the reason why I left the while in case of myrank < commonsize-1
			          //First case max number of words not reached but I have reached the end of the file
			          if(myrank < common_size-1 || common_size == 2) {
				          if(count_read_word < maxWords && feof(fp)) {
				          	startOfAnotherPosition = 0;
				          	fclose(fp);
				          	indexFile++;
				          }
			      	  }

			      	  //Equal case but rank is equal a commonsize-1
			      	  if(myrank == common_size-1) {
			      	  	if(count_read_word < maxWords && feof(fp)) {
				          	startOfAnotherPosition = 0;
				          	fclose(fp);
				          	indexFile++;
				          }
			      	  }

			      	  ////Second case max number of words reached
			          if (count_read_word == maxWords)
			          {
			          	//If have reached the end of the file
			          	if(feof(fp)){
			          		if(word[0] == '\0') {
			          			startOfAnotherPosition = 0;
			          			fclose(fp);
			          			indexFile++;
			          		} else {
			          			//Count the last word in file
			          			len = strlen(word);
					          	if (ispunct(word[len - 1]) != 0){
					            	word[len - 1] = '\0';
					          	}
			          			strcpy(words[index+1], word);
							    count[index+1]++;

			          		}
			          	} else {
				          	startOfAnotherPosition = 1; 	
			          	}
			          }
		      		}while(count_read_word < maxWords);

		      	//The useful information to understand how and which file to open is sent to the next process
		      	if(myrank < common_size-1) {
			      	MPI_Send(&startOfAnotherPosition, 1, MPI_INT, myrank+1, 30, MPI_COMM_WORLD);
			      	MPI_Send(&indexFile,1,MPI_INT,myrank+1, 31, MPI_COMM_WORLD);
			      	MPI_Send(&currentPositionInFile, 1, MPI_INT, myrank+1, 32, MPI_COMM_WORLD);
		      	}

		      	//With this instruction I send the local histograms 
		      	//of each process to the MASTER process
		        MPI_Send(count, 1, typeCount, 0, 17, MPI_COMM_WORLD);
		        MPI_Send(words, 1, typeWord, 0, 25, MPI_COMM_WORLD);
		        MPI_Send(&index, 1, MPI_INT,0, 90, MPI_COMM_WORLD);
			  

			  fclose(fp);
			    
			}
			end = MPI_Wtime();

			MPI_Finalize();

			if (myrank == 0) { /* Master node scrive su stdout il tempo o su file */
    			printf("Time in ms = %f\n", end-start);
			}
			return 0;
		}

		void sortArray(char words[MAX_WORDS][50], int count[MAX_WORDS], int index) {
			char tmpName[50];
			int tmpCount;
			for (int i = 0; i < index; ++i)
			{
				for (int j = i +1; j < index; ++j)
				{
					if (count[i] < count[j])
					{
						tmpCount = count[i];
						strcpy(tmpName, words[i]);
						count[i] = count[j];
						strcpy(words[i], words[j]);
						count[j] = tmpCount;
						strcpy(words[j], tmpName);
					}
				}
			}
		}

		char* strlwr(char* s)
		{
		    char* tmp = s;

		    for (;*tmp;++tmp) {
		        *tmp = tolower((unsigned char) *tmp);
		    }

		    return s;
		}


