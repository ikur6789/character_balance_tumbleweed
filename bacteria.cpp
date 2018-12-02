#include <cstdio>
#include <vector>
#include <cstdlib>
#include <cmath>
#include <random>
#include <algorithm>

#include "tournament.hpp"
#include "character.hpp"

#define MAX_X 8
#define MIN_X -8
#define ROSTER_SIZE 20
#define NUM_TOURNAMENTS 100

double evalFitness(std::vector<character_t> population, int character_index)
{
    double fitness = 0.0;

    /* get average total stat count for the rest of the population */
    double popAverageStatSum = 0.0;
    double curStatSum = std::accumulate(population.at(character_index).stats.begin(), population.at(character_index).stats.end(), 0.0f);
    for (int i = 0; i < population.size(); i++)
    {
        /* Don't add the current character as an average */
        if (i != character_index)
            popAverageStatSum += std::accumulate(population.at(i).stats.begin(), population.at(i).stats.end(), 0.0f);
    }
    /* Remove the current character from the size */
    popAverageStatSum /= (population.size() - 1);

    std::cout << "Rest of population average: " << popAverageStatSum << std::endl;
    std::cout << "Current Character sum: " << curStatSum << std::endl;

    fitness += abs(popAverageStatSum - curStatSum);

#if 0
    /* Method 2: add the difference between each individual rest of the population */
#endif

    return fitness;
}

void printVector(std::vector<double> v)
{
    for (double x : v)
    {
        printf("%f, ", x);
    }
}

void genRandSol(character_t &character)
{
    /* Set each stat value */
    for (int i = 0; i < NUM_STATS; i++)
    {
        character.stats.push_back( (double) (rand() % MAX_STAT_VAL) );
    }

    /* Pick one or two immutable stats */
    int stat1 = rand() % NUM_STATS;
    character.constValIndices[0] = stat1;

    character.popularity = rand() % NUM_STATS;
}

std::vector<double> getRandDir(int n)
{
    std::vector<double> d(n);
	
	/* THESE CAN BE PLAYED WITH!!! */
    double MIN = -1.0;
    double MAX = 1.0;

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

    //printf("Random direction:\n");
    //printVector(d);
    //printf("\n");

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

#if 0
void chemotaxisAndSwim(
    std::vector<character_t> &population,
    int n,
    const double STEP_SIZE,     // Same as book
    const int    ELDISP_STEPS,    // elimination/dispersal events
    const int    REPRO_STEPS,     // reproduction steps
    const int    CHEMO_STEPS,    // chemotaxis/swim events
    const int    SWIM_LEN,        // how long to swim?
    const double ELIM_PROB,    // Probability of elimination
    const double ATTRACT_D,       // attraction coefficient
    const double ATTRACT_W,     // attraction weight?
    const double REPEL_H,       // repel coefficient
    const double REPEL_W      // repel weight 
)
{
    //printf("Population size: %d\n", population.size());
    //printf("N:%d\n", n);
    
    for( int cellNum = 0; cellNum < population.size(); cellNum++ )
    {
    // ------------------- TUMBLING ------------------------------
    // -----------------------------------------------------------

        //printf("cell num %d\n", cellNum);

        // calculate the current cell's fitness
        population.at(cellNum).fitness = evalFitness(population.at(cellNum).stats) + cellInteraction(population, population.at(cellNum), ATTRACT_D, ATTRACT_W, REPEL_H, REPEL_W);

        for (int stepNum = 0; stepNum < CHEMO_STEPS; stepNum++)
        {
            /* Create a temp cell and have it take a random step */
            character_t tempCell;
            tempCell.health = population.at(cellNum).health;
            tempCell.stats.reserve(n);
            std::vector<double> dir = getRandDir(n);

            character_t curCell = population.at(cellNum);
            for (int i = 0; i < n; i++)
            {
                //printf("Before tumble\n");
                //printf("Dir: "); printVector(dir); printf("\n");
                //printf("curcell stats: "); printVector(curCell.stats); printf("\n");
                //printf("tempcell stats size: %d\n", tempCell.stats.size());
                tempCell.stats.push_back( curCell.stats.at(i) + STEP_SIZE * dir.at(i) );
                //printf("After tumble\n");

                if (tempCell.stats.at(i) > MAX_X) tempCell.stats.at(i) = MAX_X;
                if (tempCell.stats.at(i) < MIN_X) tempCell.stats.at(i) = MIN_X;

            }

            tempCell.fitness = evalFitness(tempCell.stats) + cellInteraction(population, tempCell, ATTRACT_D, ATTRACT_W, REPEL_H, REPEL_W);
            /* Exit if we didn't find a better solution? 
             * because we're MAXIMIZING a problem less is worse*/
            //if (tempCell.fitness > population.at(cellNum).fitness) {
            if (tempCell.fitness < population.at(cellNum).fitness) {
                stepNum = CHEMO_STEPS;
            }
            else {
                /* Otherwise the cell = the new cell, and add to the
                * overall health of the cell */
                population.at(cellNum) = tempCell;
                population.at(cellNum).health += tempCell.fitness;
            }
        }

        
    // -------------------- END TUMBLING ------------------------
    // ---------------------------------------------------------- 
    }
}
#endif

/* Eliminate part of the population */
void eliminatePop(
    std::vector<character_t> &population
)
{
    /* Sort by health 
     * cells now sorted greatest health -> least health */
    std::sort(population.begin(), population.end(),
            [](character_t a, character_t b) {
                return a.health > b.health;
            }
    );

    /* Replace the bottom half (worse) of the cells
     * with their top half (better) counterparts 
     * 
     * TODO - this could be played with/adjusted
     * to calculate based on the total number of cells
     * we want to replace !!! */
    for (int i = 0; i < population.size() / 2; i++)
    {
        population.at(i + (population.size()/2)) = population.at(i);
    }
}

#if 0
/* n is the number of dimensions */
/* https://gist.github.com/x0xMaximus/8626921 */
void bacterialOptimization(int n)
{
    const int    POP_SIZE = 20;       // population size - book uses 50
    const double STEP_SIZE = 0.1;     // Same as book
    const int    ELDISP_STEPS = 5;    // elimination/dispersal events
    const int    REPRO_STEPS = 4;     // reproduction steps
    const int    CHEMO_STEPS = 350;    // chemotaxis/swim events - set as 7*ELDISP_STEPS
    const int    SWIM_LEN = 4;        // how long to swim?
    const double ELIM_PROB = 0.25;    // Probability of elimination
    const double ATTRACT_D = 0.1;       // attraction coefficient
    const double ATTRACT_W = 0.2;     // attraction weight?
    const double REPEL_H = 0.1;       // repel coefficient
    const double REPEL_W = 10.0;      // repel weight 

    std::vector<character_t> population(POP_SIZE);

    character_t best; // best cell;
    best.fitness = -9999;

    /* Generate the initial population */
    //printf("Initial pop:\n");
    for (int i = 0; i < POP_SIZE; i++)
    {
        //population.at(i).stats = genRandSol(n);
        genRandSol(population.at(i));
        population.at(i).fitness = 0.0;
        population.at(i).health = 0.0;
    //    printVector(population.at(i).stats);
    //    printf("\n");
    }
    //printf("\n");

    /* Elimination/Dispersal Events */
    for (int l = 0; l < ELDISP_STEPS; l++)
    {
        for (int k = 0; k < REPRO_STEPS; k++)
        {
            for (int j = 0; j < CHEMO_STEPS; j++)
            {
                /* Swim about */
                chemotaxisAndSwim(population, n, STEP_SIZE, ELDISP_STEPS, REPRO_STEPS,
                    CHEMO_STEPS, SWIM_LEN, ELIM_PROB, ATTRACT_D, ATTRACT_W, REPEL_H,
                    REPEL_W);

                /* Check for a new best */
                for (character_t cell : population)
                {
                    /* -9999 for the initial cell */
                    //if (best.fitness == -9999 || cell.fitness >= best.fitness)
                    if (cell.fitness > best.fitness)
                    {
                        best = cell;
                        //printf("New Best: "); printVector(best.stats); printf("\n");
                        //printf("Fitness: %f\n", evalFitness(best.stats));
                    }
                }
            } // end CHEMO_STEPS

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
                genRandSol(population.at(cellNum));
                population.at(cellNum).health = 0.0;
                population.at(cellNum).fitness = evalFitness(population.at(cellNum).stats);
            }
        }
    } // end ELDISP steps

    

    printf("Best: "); printVector(best.stats); printf("\n");
    printf("Fitness: %f\n", evalFitness(best.stats));
}
#endif

void fill_roster(std::vector<character_t> &roster)
{
    for(int i=0;i<ROSTER_SIZE;++i)
    {
        character_t addition;
        roster.push_back(addition);
    }

    return;
}

int main(int argc, char *argv[])
{
    srand(time(0));

	std::vector<int> dims = {1, 2, 3, 5, 8, 13};

	std::vector<character_t> roster;
	fill_roster(roster);
	
	/*for(int x : dims)
	{
		printf("N = %d\n--------------\n", x);
		bacterialOptimization(x);
		printf("\n");
	}*/
	
    std::vector<character_t> population;
    for (int i = 0; i < ROSTER_SIZE; i++)
    {
        character_t character;
        population.push_back(character);
    }

    for (int i = 0; i < population.size(); i++)
    {
        double fitness = evalFitness(population, i);
        std::cout << "Fitness: " << fitness << std::endl;
    }

    return 0;
}
