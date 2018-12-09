#include <cstdio>
#include <vector>
#include <cstdlib>
#include <cmath>
#include <random>
#include <algorithm>
#include <cfloat>

#include "tournament.hpp"
#include "character.hpp"
#include "csv.hpp"

#define MAX_X 8
#define MIN_X -8
#define ROSTER_SIZE 20

std::vector<character_t> bestPopulation;
double bestFitness = DBL_MAX;

double evalFitness(std::vector<character_t> population, character_t character, int character_index)
{
    double fitness = 0.0;
    double win_range_scale = 1.5; // The win range should be more important than the number of wins
    double stats_range_scale = 5.0; // to scale the current character's range of stat values (to penalize if the stats are too similar)

    /* get average total stat count for the rest of the population */
    double popAverageStatSum = 0.0;
    double curStatSum = std::accumulate(character.stats.begin(), character.stats.end(), 0.0f);
    for (int i = 0; i < population.size(); i++)
    {
        /* Don't add the current character as an average */
        if (i != character_index)
            popAverageStatSum += std::accumulate(population.at(i).stats.begin(), population.at(i).stats.end(), 0.0f);
    }
    /* Remove the current character from the size */
    popAverageStatSum /= (population.size() - 1);

    fitness += abs(popAverageStatSum - curStatSum);
    
    double singleStatAverage = 0.0;
	for(int s = 0; s < population.at(0).stats.size(); ++s)
	{
			for (int i = 0; i < population.size(); i++)
			{
				/* Don't add the current character as an average */
				if (i != character_index)
				{
					singleStatAverage += population.at(i).stats.at(s);
				}
			}

			singleStatAverage /= (population.size() - 1);

			fitness -= pow(abs(singleStatAverage - character.stats.at(s)), 2);
	}

    /* The current character's stats shouldn't be too similar
     * If the range is large then this is a good thing -> subtract from fitness */
    auto minmax = std::minmax_element(character.stats.begin(), character.stats.end());

    fitness -= (double)(*minmax.second - *minmax.first) * stats_range_scale;
    /**********************************************************/

    int win_range = compete(population, character, character_index);

    int num_wins = character.numWins;

    /* Winning more tournaments should improve the score */
    fitness -= num_wins;

    /* A large win range (e.g. someone's winning alot and someone else 
     * isn't winning) should decrease the score */
    fitness += win_range * win_range_scale;

    //std::cout << "  Num Wins: " << num_wins << "  Win Range: " << win_range << std::endl;
    /*****************************************************/

    return fitness;
}

void printVector(std::vector<double> v)
{
    for (double x : v)
    {
        printf("%f, ", x);
    }
}

//void genRandSol(character_t &character)
//{
    /* Set each stat value */
//    for (int i = 0; i < NUM_STATS; i++)
//    {
//        character.stats.push_back( (double) (rand() % MAX_STAT_VAL) );
//    }

    /* Pick one or two immutable stats */
//    int stat1 = rand() % NUM_STATS;
//    character.constValIndices[0] = stat1;

//    character.popularity = rand() % NUM_STATS;
//}

std::vector<double> getRandDir(int n)
{
    std::vector<double> d(n);
	
	/* THESE CAN BE PLAYED WITH!!! */
    // BJ changed as our values have a much wider value now
    double MIN = -3.0;
    double MAX = 3.0;

    /* Since it's just a direction we want between 0 and 1,
     * and the values will be scaled accordingly 
     * 
     * Right now step size is only 0.1 though, so might need to make
     * higher... */
    for (int i = 0; i < n; i++)
    {
        double num = MIN + (double)rand() / ((double)RAND_MAX / (MAX-MIN));
        d.at(i) = num;
    }

    return d;
}

/* The cell interaction equation `g()` as described by the
 * clever algorithms textbook */
double cellInteraction(std::vector<character_t> population, character_t cell, double ATTRACT_D, double ATTRACT_W, double REPEL_H, double REPEL_W)
{
    double sumAttract = 0.0;
    double sumRepel = 0.0;

    for (character_t neighborCell : population)
    {
        double diff = 0.0;

        for (int i = 0; i < neighborCell.stats.size(); i++)
        {
            diff += pow((cell.stats.at(i) - neighborCell.stats.at(i)), 2.0);
        }

        sumAttract += -1.0 * ATTRACT_D * exp(-1.0 * ATTRACT_W * diff);
        sumRepel += REPEL_H * exp(-1.0 * REPEL_W * diff);
    }

    return sumAttract + sumRepel;
}

void chemotaxisAndSwim(
    std::vector<character_t> &population,
    int n,
    const double STEP_SIZE,     // Same as book
    const int    ELDISP_STEPS,  // elimination/dispersal events
    const int    REPRO_STEPS,   // reproduction steps
    const int    CHEMO_STEPS,   // chemotaxis/swim events
    const int    SWIM_LEN,      // how long to swim?
    const double ELIM_PROB,     // Probability of elimination
    const double ATTRACT_D,     // attraction coefficient
    const double ATTRACT_W,     // attraction weight?
    const double REPEL_H,       // repel coefficient
    const double REPEL_W        // repel weight 
)
{    
    for( int cellNum = 0; cellNum < population.size(); cellNum++ )
    {
    // ------------------- TUMBLING ------------------------------
    // -----------------------------------------------------------

        // calculate the current cell's fitness
        population.at(cellNum).fitness = evalFitness(population, population.at(cellNum), cellNum) + cellInteraction(population, population.at(cellNum), ATTRACT_D, ATTRACT_W, REPEL_H, REPEL_W);

        for (int stepNum = 0; stepNum < CHEMO_STEPS; stepNum++)
        {
            /* Create a temp cell and have it take a random step */
            character_t tempCell;
            tempCell.bacteriaHealth = population.at(cellNum).bacteriaHealth;
            tempCell.stats.reserve(n);
            std::vector<double> dir = getRandDir(n);

            character_t curCell = population.at(cellNum);
            //changes the stats, moves bacteria (character) around
            for (int i = 0; i < n; i++) {
                if (i != curCell.constValIndices[0] && i != curCell.constValIndices[1]) {
                    tempCell.stats.at(i) = curCell.stats.at(i) + STEP_SIZE * dir.at(i);
                }
                //else {
                //    std::cout << "Ignoring const val index: " << i << std::endl;
                //}
            }

            tempCell.fitness = evalFitness(population, tempCell, cellNum) + cellInteraction(population, tempCell, ATTRACT_D, ATTRACT_W, REPEL_H, REPEL_W);
            /* Exit if we didn't find a better solution? 
             * because we're MAXIMIZING a problem less is worse*/
            if (tempCell.fitness < population.at(cellNum).fitness)
                stepNum = CHEMO_STEPS;
            else {
                /* Otherwise the cell = the new cell, and add to the
                * overall bacteriaHealth of the cell */
                population.at(cellNum) = tempCell;
                population.at(cellNum).bacteriaHealth += tempCell.fitness;
            }
        }

        
    // -------------------- END TUMBLING ------------------------
    // ---------------------------------------------------------- 
    }
}

/* Eliminate part of the population */
void eliminatePop( std::vector<character_t> &population )
{
    /* Sort by bacteriaHealth 
     * cells now sorted greatest bacteriaHealth -> least bacteriaHealth */
    std::sort(population.begin(), population.end(),
               [](character_t a, character_t b) {
                return a.bacteriaHealth > b.bacteriaHealth;
                }
             );

    /* Replace the bottom half (worse) of the cells
     * with their top half (better) counterparts 
     * 
     * this could be played with/adjusted
     * to calculate based on the total number of cells
     * we want to replace !!! */
    for (int i = 0; i < population.size() / 2; i++)
        population.at(i + (population.size()/2)) = population.at(i);
}

/* n is the number of dimensions */
/* https://gist.github.com/x0xMaximus/8626921 */
void bacterialOptimization(int n)
{
    const int    POP_SIZE = 20;     // population size - book uses 50
    const double STEP_SIZE = 1.0;   // Same as book
    const int    ELDISP_STEPS = 5;  // elimination/dispersal events
    const int    REPRO_STEPS = 4;   // reproduction steps
    const int    CHEMO_STEPS = 350; // chemotaxis/swim events - set as 7*ELDISP_STEPS
    const int    SWIM_LEN = 4;      // how long to swim?
    const double ELIM_PROB = 0.25;  // Probability of elimination
    const double ATTRACT_D = 0.1;   // attraction coefficient
    const double ATTRACT_W = 0.2;   // attraction weight?
    const double REPEL_H = 0.1;     // repel coefficient
    const double REPEL_W = 10.0;    // repel weight 

    std::vector<character_t> population;

    /* Generate the initial population */
    for (int i = 0; i < POP_SIZE; i++)
    {
        //population.at(i).stats = genRandSol(n);
        //genRandSol(population.at(i));

        character_t character;
        population.push_back(character);
        
		population.at(i).fitness = 0.0;
        population.at(i).bacteriaHealth = 0.0;
    }

    printInitCSV(population);

    /* Elimination/Dispersal Events */
    for (int l = 0; l < ELDISP_STEPS; l++)
    {
        std::cout << "Elimination Step " << l << std::endl;
        for (int k = 0; k < REPRO_STEPS; k++)
        {
            std::cout << "Reproduction Step " << k << std::endl;
            for (int j = 0; j < CHEMO_STEPS; j++)
            {
                /* Swim about */
                chemotaxisAndSwim(population, n, STEP_SIZE, ELDISP_STEPS, REPRO_STEPS,
                    CHEMO_STEPS, SWIM_LEN, ELIM_PROB, ATTRACT_D, ATTRACT_W, REPEL_H,
                    REPEL_W);

				double currentFitness = 0;
				for(character_t & character : population)
				{
					currentFitness += character.fitness;	
				}	

				if(currentFitness < bestFitness)
				{
					bestPopulation = population;
					bestFitness = currentFitness;

					printf("%f\n", bestFitness);		
				}

            }// end CHEMO_STEPS
    
            // elimination step
            eliminatePop(population);
        
        } // end REPRO_STEPS
    
        /* Randomly replace a cell at a new location */
        const double MAXPROB = 1.0;
        for (int cellNum = 0; cellNum < population.size(); cellNum++)
        {
            double num = (double)rand() / ((double)RAND_MAX / (MAXPROB));
            if (num < ELIM_PROB) {
                //population.at(cellNum).stats = genRandSol(n);
                //genRandSol(population.at(cellNum));
                population.at(cellNum).generateRandomStats();
                population.at(cellNum).bacteriaHealth = 0.0;
                population.at(cellNum).fitness = evalFitness(population, population.at(cellNum), cellNum);
            }
        }

    } // end ELDISP steps
   
	printResultsCSV(population);

	printf("%f\n", bestFitness);		

}//end of bacterialOptimization function

int main(int argc, char *argv[])
{
    srand(time(0));
	
	bacterialOptimization(8);

    return 0;
}
