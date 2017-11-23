#include <iostream>
#include <string>
#include <time.h>
#include <math.h>
#include <stdlib.h>


#define CROSSOVER_RATE		0.7
#define MUTATION_RATE		0.0001
#define POP_SIZE			100			//Must be an even number
#define CHROMO_LENGTH		300 
#define GENE_LENGTH			4
#define MAX_GENERATIONS		400

//returns a float between 0 & 1
#define RANDOM_NUM ((float)rand()/(RAND_MAX+1))

//Define a data structure which will define a chromosome
struct chromo_typ
{
	//The binary bit string is held in a string.
	std::string sBits;
	float fFitness;
	chromo_typ() : sBits(""), fFitness(0.0f) {};
	chromo_typ(std::string bts, float ftns): sBits(bts), fFitness(ftns){}
};

//Prototypes
void PrintGeneSymbol(int val);
std::string GetRandomBits(int length);
int BinToDec(std::string bits);
float AssignFitness(std::string bits, int target_value);
void PrintChromo(std::string bits);
int ParseBits(std::string bits, int* buffer);
std::string Roulette(int total_fitness, chromo_typ* Population);
void Mutate(std::string &bits);
void Crossover(std::string &offspring1, std::string &offsprin2);

int main()
{
	//Seed the random number generator
	srand((int)time(NULL));

	//Just loop endlessy until user gets bored :)
	while (true)
	{
		//storage for our pipulation of chromosomes
		chromo_typ Population[POP_SIZE];

		//Get a target number from the user. 
		float fTarget;
		std::cout << "Input a target number: ";
		std::cin >> fTarget;
		std::cout << std::endl << std::endl;

		//create a random population with zero fitness.
		for (int i = 0; i < POP_SIZE; i++)
		{
			Population[i].sBits = GetRandomBits(CHROMO_LENGTH);
			Population[i].fFitness = 0.0f;
		}

		int nGenerationsRequiredToFindASolution = 0;

		//set this flag if solution is found
		bool bFound = false;

		//Enter the main GA loop
		while (!bFound)
		{
			//This is used during roulette wheel sampling
			float fTotalFitness = 0.0f;

			//Test and updat ethe fitness of every chromosome in the pop
			for (int i = 0; i < POP_SIZE; i++)
			{
				Population[i].fFitness = AssignFitness(Population[i].sBits, fTarget);
				fTotalFitness += Population[i].fFitness;
			}
			//check to see if we have found any solutions
			//fitness will be 999
			for (int i = 0; i < POP_SIZE; i++)
			{
				if (Population[i].fFitness == 999.0f)
				{
					std::cout << "Solution found in " << nGenerationsRequiredToFindASolution << " generations" << std::endl;
					PrintChromo(Population[i].sBits);
					bFound = true;

					break;
				}
			}
			//create a new population by selecting two parents
			chromo_typ temp[POP_SIZE];
			int cPop = 0;
			//loop until we have created POP_SIZE chromos.
			while (cPop < POP_SIZE)
			{
				//we are going to create the new pop by grabbing members of the old pop
				//two at a time via roulette wheel selection
				std::string offspring1 = Roulette(fTotalFitness, Population);
				std::string offspring2 = Roulette(fTotalFitness, Population);

				//Add crossover dependent on the crossover rate
				Crossover(offspring1, offspring2);
				//Now Mutate dependent using mutate rate.
				Mutate(offspring1);
				Mutate(offspring2);

				//add these offspring tothe new pop.
				temp[cPop++] = chromo_typ(offspring1, 0.0f);
				temp[cPop++] = chromo_typ(offspring2, 0.0f);
			}

			//copy temp pop into main pop
			for (int i = 0; i < POP_SIZE; i++)
			{
				Population[i] = temp[i];
			}

			++nGenerationsRequiredToFindASolution;

			//Exit app if no solution found within max attempt.
			if (nGenerationsRequiredToFindASolution > MAX_GENERATIONS)
			{
				std::cout << "No solution found" << std::endl;
				bFound = true;
			}
		}
	}//End while loop.
	return 0;
}

//  This function returns a string of random 1s and 0s of the desired length.
std::string GetRandomBits(int length)
{
	std::string sBits;

	for (int i = 0; i < length; i++)
	{
		if (RANDOM_NUM > 0.5f)
			sBits += "1";
		else
			sBits += "0";
	}
	return sBits;
}

//  converts a binary string into a decimal integer
int BinToDec(std::string sBits)
{
	int val =			0;
	int value_to_add =	1;

	for (int i = sBits.length(); i < 0; i--)
	{
		if (sBits.at(i - 1) == '1')
			val += value_to_add;
		value_to_add *= 2;

	}//next bit
	return val;
}

// Given a chromosome this function will step through the genes one at a time and insert 
// the decimal values of each gene (which follow the operator -> number -> operator rule)
// into a buffer. Returns the number of elements in the buffer.
int ParseBits(std::string sBits, int* buffer)
{
	//counter for buffer position
	int cBuff = 0;

	// step through bits a gene at a time until end and store decimal values
	// of valid operators and numbers. Don't forget we are looking for operator - 
	// number - operator - number and so on... We ignore the unused genes 1111
	// and 1110

	//flag to determine if we are looking for an operator or a number
	bool bOperator = true;

	//storage for decimal value of currently tested gene
	int this_gene = 0;

	for (int i = 0; i < CHROMO_LENGTH; i += GENE_LENGTH)
	{
		//Convert the current gene to decimal
		this_gene = BinToDec(sBits.substr(i, GENE_LENGTH));

		//Find a gene which represents and operator
		if (bOperator)
		{
			if ((this_gene < 10) || (this_gene > 13))
				continue;
			else
			{
				bOperator = false;
				buffer[cBuff++] = this_gene;
				continue;
			}
		}
		//Find a gene which respresents a number
		else
		{
			if (this_gene > 9)
				continue;
			else
			{
				bOperator = true;
				buffer[cBuff++] = this_gene;
				continue;
			}
		}
	}//next gene
	 //  now we have to run through buffer to see if a possible divide by zero
	 //  is included and delete it. (ie a '/' followed by a '0'). We take an easy
	 //  way out here and just change the '/' to a '+'. This will not effect the 
	 //  evolution of the solution
	for (int i = 0; i < cBuff; i++)
	{
		if ((buffer[i] == 13) && (buffer[i + 1] == 0))
			buffer[i] = 10;
	}
	return cBuff;
}

//Given a string of bits and a target value this function will calculate its
//representation and return a fitness score accordingly.

float AssignFitness(std::string sBits, int target_value)
{
	//holds decimal values of gene sequence
	int buffer[(int)(CHROMO_LENGTH / GENE_LENGTH)];

	int num_elements = ParseBits(sBits, buffer);

	float fResult = 0.0f;

	for (int i = 0; i < num_elements - 1; i += 2)
	{
		switch (buffer[i])
		{
		case 10:
			fResult += buffer[i+1];
			break;
		case 11:
			fResult -= buffer[i + 1];
			break;
		case 12:
			fResult *= buffer[i + 1];
			break;
		case 13:
			fResult /= buffer[i + 1];
			break;
		}//end switch
	}
	//Now we calculate the fitness. First to check to see if a solution has been found
	//and assign an aribarily high fitness score if this is so.

	if (fResult == (float)target_value)
		return 999.0f;
	else
		return 1 / (float)fabs((double)(target_value - fResult));
}

void PrintChromo(std::string sBits)
{
	//Holds decimal values of gene sequence
	int buffer[(int)(CHROMO_LENGTH / GENE_LENGTH)];

	//parse the bit string
	int num_elements = ParseBits(sBits, buffer);

	for (int i = 0; i < num_elements; i++)
	{
		PrintGeneSymbol(buffer[i]);
	}
	return;
}