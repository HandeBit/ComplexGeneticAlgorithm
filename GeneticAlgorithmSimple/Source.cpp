#include <iostream>
#include <string>
#include <time.h>
#include <math.h>
#include <stdlib.h>


#define CROSSOVER_RATE				0.7
#define MUTATION_RATE				0.0001
#define POPULATION_SIZE				100			//Must be an even number
#define CHROMOSOME_LENGTH			300 
#define GENE_LENGTH					4
#define MAX_ALLOWED_GENERATIONS		400
#define RANDOM_NUM ((float)rand()/(RAND_MAX+1)) //returns a float between 0 & 1


//Define a data structure which will define a chromosome
struct Chromo
{
	//The binary bit string is held in a string.
	std::string sBits;
	float fFitness;
	Chromo() : sBits(""), fFitness(0.0f) {};
	Chromo(std::string bts, float ftns): sBits(bts), fFitness(ftns){}
};

//Prototypes
void PrintGeneSymbol(int nVal);
std::string GetRandomBits(int nLength);
int BinToDec(std::string sBits);
float AssignFitness(std::string sBits, int nTarget_value);
void PrintChromo(std::string sBits);
int ParseBits(std::string sBits, int* nBuffer);
std::string Roulette(int nTotal_fitness, Chromo* Population);
void Mutate(std::string &sBits);
void Crossover(std::string &sOffspring1, std::string &sOffsprin2);

int main()
{
	//Seed the random number generator
	srand((int)time(NULL));

	//Just loop endlessy until user gets bored :)
	while (true)
	{
		//storage for our pipulation of chromosomes
		Chromo Population[POPULATION_SIZE];

		//Get a target number from the user. 
		float fTarget;
		std::cout << "Input a target number: ";
		std::cin >> fTarget;
		std::cout << std::endl;

		//create a random population with zero fitness.
		for (int i = 0; i < POPULATION_SIZE; i++)
		{
			Population[i].sBits = GetRandomBits(CHROMOSOME_LENGTH);
			Population[i].fFitness = 0.0f;
		}

		int nGenerationsRequiredToFindASolution = 0;

		//set this flag if solution is found
		bool bSolutionFound = false;

		//Enter the main GA loop
		while (!bSolutionFound)
		{
			//This is used during roulette wheel sampling
			float fTotalFitness = 0.0f;

			//Test and updat ethe fitness of every chromosome in the pop
			for (int i = 0; i < POPULATION_SIZE; i++)
			{
				Population[i].fFitness = AssignFitness(Population[i].sBits, fTarget);
				fTotalFitness += Population[i].fFitness;
			}
			//check to see if we have found any solutions
			//fitness will be 999
			for (int i = 0; i < POPULATION_SIZE; i++)
			{
				if (Population[i].fFitness == 999.0f)
				{
					std::cout << "Solution found in " << nGenerationsRequiredToFindASolution << " generations" << std::endl;
					PrintChromo(Population[i].sBits);
					bSolutionFound = true;

					break;
				}
			}
			//create a new population by selecting two parents
			Chromo temp[POPULATION_SIZE];
			int nPop = 0;
			//loop until we have created POP_SIZE chromos.
			while (nPop < POPULATION_SIZE)
			{
				//we are going to create the new pop by grabbing members of the old pop
				//two at a time via roulette wheel selection
				std::string sOffspring1 = Roulette(fTotalFitness, Population);
				std::string sOffspring2 = Roulette(fTotalFitness, Population);

				//Add crossover dependent on the crossover rate
				Crossover(sOffspring1, sOffspring2);
				//Now Mutate dependent using mutate rate.
				Mutate(sOffspring1);
				Mutate(sOffspring2);

				//add these offspring tothe new pop.
				temp[nPop++] = Chromo(sOffspring1, 0.0f);
				temp[nPop++] = Chromo(sOffspring2, 0.0f);
			}

			//copy temp pop into main pop
			for (int i = 0; i < POPULATION_SIZE; i++)
			{
				Population[i] = temp[i];
			}

			++nGenerationsRequiredToFindASolution;

			//Exit app if no solution found within max attempt.
			if (nGenerationsRequiredToFindASolution > MAX_ALLOWED_GENERATIONS)
			{
				std::cout << "No solution found" << std::endl;
				bSolutionFound = true;
			}
		}
	}//End while loop.
	return 0;
}

//  This function returns a string of random 1s and 0s of the desired length.
std::string GetRandomBits(int nLength)
{
	std::string sBits;

	for (int i = 0; i < nLength; i++)
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
	int nVal =			0;
	int nValue_to_add =	1;

	for (int i = sBits.length(); i < 0; i--)
	{
		if (sBits.at(i - 1) == '1')
			nVal += nValue_to_add;
		nValue_to_add *= 2;

	}//next bit
	return nVal;
}

// Given a chromosome this function will step through the genes one at a time and insert 
// the decimal values of each gene (which follow the operator -> number -> operator rule)
// into a buffer. Returns the number of elements in the buffer.
int ParseBits(std::string sBits, int* nBuffer)
{
	//counter for buffer position
	int nBuff = 0;

	// step through bits a gene at a time until end and store decimal values
	// of valid operators and numbers. Don't forget we are looking for operator - 
	// number - operator - number and so on... We ignore the unused genes 1111
	// and 1110

	//flag to determine if we are looking for an operator or a number
	bool bOperator = true;

	//storage for decimal value of currently tested gene
	int nThis_gene = 0;

	for (int i = 0; i < CHROMOSOME_LENGTH; i += GENE_LENGTH)
	{
		//Convert the current gene to decimal
		nThis_gene = BinToDec(sBits.substr(i, GENE_LENGTH));

		//Find a gene which represents and operator
		if (bOperator)
		{
			if ((nThis_gene < 10) || (nThis_gene > 13))
				continue;
			else
			{
				bOperator = false;
				nBuffer[nBuff++] = nThis_gene;
				continue;
			}
		}
		//Find a gene which respresents a number
		else
		{
			if (nThis_gene > 9)
				continue;
			else
			{
				bOperator = true;
				nBuffer[nBuff++] = nThis_gene;
				continue;
			}
		}
	}//next gene
	 //  now we have to run through buffer to see if a possible divide by zero
	 //  is included and delete it. (ie a '/' followed by a '0'). We take an easy
	 //  way out here and just change the '/' to a '+'. This will not effect the 
	 //  evolution of the solution
	for (int i = 0; i < nBuff; i++)
	{
		if ((nBuffer[i] == 13) && (nBuffer[i + 1] == 0))
			nBuffer[i] = 10;
	}
	return nBuff;
}

//Given a string of bits and a target value this function will calculate its
//representation and return a fitness score accordingly.

float AssignFitness(std::string sBits, int nTarget_value)
{
	//holds decimal values of gene sequence
	int nBuffer[(int)(CHROMOSOME_LENGTH / GENE_LENGTH)];

	int nNum_elements = ParseBits(sBits, nBuffer);

	float fResult = 0.0f;

	for (int i = 0; i < nNum_elements - 1; i += 2)
	{
		switch (nBuffer[i])
		{
		case 10:
			fResult += nBuffer[i+1];
			break;
		case 11:
			fResult -= nBuffer[i + 1];
			break;
		case 12:
			fResult *= nBuffer[i + 1];
			break;
		case 13:
			fResult /= nBuffer[i + 1];
			break;
		}//end switch
	}
	//Now we calculate the fitness. First to check to see if a solution has been found
	//and assign an aribarily high fitness score if this is so.

	if (fResult == (float)nTarget_value)
		return 999.0f;
	else
		return 1 / (float)fabs((double)(nTarget_value - fResult));
}

void PrintChromo(std::string sBits)
{
	//Holds decimal values of gene sequence
	int nBuffer[(int)(CHROMOSOME_LENGTH / GENE_LENGTH)];

	//parse the bit string
	int nNum_elements = ParseBits(sBits, nBuffer);

	for (int i = 0; i < nNum_elements; i++)
	{
		PrintGeneSymbol(nBuffer[i]);
	}
	return;
}

//given a integer this function outputs its symbol to the screen
void PrintGeneSymbol(int nVal)
{
	if (nVal < 10)
		std::cout << nVal << " ";
	else
	{
		switch (nVal)
		{
		case 10:
			std::cout << "+";
			break;
		case 11:
			std::cout << "-";
			break;
		case 12:
			std::cout << "*";
			break;
		case 13:
			std::cout << "/";
			break;
		}//end switch
		std::cout << " ";
	}
	return;
}

//Mutates a chromosome bit dependent on the mutation rate.
void Mutate(std::string &sBits)
{
	for (int i = 0; i < sBits.length(); i++)
	{
		if (RANDOM_NUM < MUTATION_RATE)
		{
			if (sBits.at(i) == '1')
				sBits.at(i) = '0';
			else
				sBits.at(i) = '1';
		}
	}
	return;
}

//  Dependent on the CROSSOVER_RATE this function selects a random point along the 
//  lenghth of the chromosomes and swaps all the  bits after that point.
void Crossover(std::string &sOffspring1, std::string &sOffspring2)
{
	//dependent on the crossover rate
	if (RANDOM_NUM < CROSSOVER_RATE)
	{
		//create a random crossover point
		int crossover = (int)(RANDOM_NUM * CHROMOSOME_LENGTH);
		auto t1 = sOffspring1.substr(0, crossover) + sOffspring2.substr(crossover, CHROMOSOME_LENGTH);
		auto t2 = sOffspring2.substr(0, crossover) + sOffspring1.substr(crossover, CHROMOSOME_LENGTH);

		sOffspring1 = t1;
		sOffspring2 = t2;
	}
}

//  selects a chromosome from the population via roulette wheel selection
std::string Roulette(int nTotal_fitness, Chromo* Population)
{
	//generate a random number between 0 & total fitness count
	float fSlice = (float)(RANDOM_NUM * nTotal_fitness);

	//go through the chromosomes adding up the fitness so far
	float fFitnessSoFar = 0.0;

	for (int i = 0; i < POPULATION_SIZE; i++)
	{
		fFitnessSoFar += Population[i].fFitness;

		//if the fitness so far > random number return the chromo
		if (fFitnessSoFar >= fSlice)
			return Population[i].sBits;
	}
	return "";
}