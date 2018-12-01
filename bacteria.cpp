#include <cstdio>
#include <vector>
#include <cstdlib>
#include <cmath>
#include <random>
#include <algorithm>

#define MAX_X 8
#define MIN_X -8
#define ROSTER_SIZE 20

enum STAT_TYPE {
    MAX_HP,
    STRENGTH,
    MAGIC,
    SKILL,
    SPEED,
    LUCK,
    DEFENSE,
    RESISTANCE,

    // This defines how many stats there are
    NUM_STATS
};

const int MAX_STAT_VAL = 100;

class character_t
{
	public:
        //Function members:
        character_t()
        {
           for(int s=0;s<NUM_STATS;++s)
            	stats.push_back((double)(rand() % MAX_STAT_VAL)+20); //random stat from 20-100 for every stat
			
            //Creating non changeable stats
                //first non-changable stat
            constValIndices[0] = rand() % NUM_STATS;

                //second non-changable stat, if they have one
            if(rand()%2) //"If an odd number, true, if an even number, false"
            {
                do
                {
                    constValIndices[1] = rand() % NUM_STATS;
                }
                while(constValIndices[1]!=constValIndices[0]); //Ensures two unique unchangable stats
                
            }

			health = stats[0]; //Starting health is their max health stat

        }

        //Data members:
    	std::vector<double> stats;
    	double              fitness;
    	double              health;
    	int                 constValIndices[2] = {-1,-1 };
    	double              popularity = 0; //All characters have 0 popularity, they gain more by winning tournaments


};

/*
battle calculations
based on FE7 from https://fireemblem.fandom.com/wiki/Battle_Formulas

accuracy = 2*skill + 0.5*luck
dodge = 2*speed + luck
chance to hit = accuracy - dodge

phys damage = strength - defense
mag damage = magic - resistance

speed >= 4 
2*damage

critical = skill/2
critical evade = luck
critical chance = critical - crit evade

critical damage = damage*3
*/

character_t tournament_match(character_t fighter1, character_t fighter2)
{
    
	int fighter1Hit = fighter1.stats[SKILL]*2 + fighter1.stats[LUCK]/2 - fighter2.stats[SPEED]*2 + fighter2.stats[LUCK];
	int fighter2Hit = fighter2.stats[SKILL]*2 + fighter2.stats[LUCK]/2 - fighter1.stats[SPEED]*2 + fighter1.stats[LUCK];

    int fighter1Dmg = 0;
	int fighter2Dmg = 0;
        
    int attack_type = rand()%2; //Randomly choose attack type this turn
    if(attack_type) // if 1, physical attack. if 0, magical attack
    {
        fighter1Dmg = fighter1.stats[STRENGTH] - fighter2.stats[DEFENSE];
        fighter2Dmg = fighter2.stats[STRENGTH] - fighter1.stats[DEFENSE];
    }
    else
    {
        fighter1Dmg = fighter1.stats[MAGIC] - fighter2.stats[RESISTANCE];
        fighter2Dmg = fighter2.stats[MAGIC] - fighter1.stats[RESISTANCE];
    }
    
    int fighter1Crit = fighter1.stats[SKILL]/2 - fighter2.stats[LUCK];
    int fighter2Crit = fighter2.stats[SKILL]/2 - fighter1.stats[LUCK];
		
    //fight to death
    while( (fighter1.health > 0) && (fighter2.health > 0) )
	{
		
        //The faster of two will attack first
        if(fighter1.stats[SPEED]>fighter2.stats[SPEED]) //fighter 1 will attack first
        {
            //fighter1
                //roll a chance to hit
            if(rand()%100 <= fighter1Hit)
            {
                //attack will go through, need to attack
                if(rand()%100 <= fighter1Crit) // triples damage if critical
                    fighter1Dmg *= 3;
            
                fighter2.health -= fighter1Dmg;
            }

            //fighter2
            if(rand()%100 <= fighter2Hit)
            {
                if(rand()%100 <= fighter2Crit)
            	    fighter2Dmg *= 3; 

                fighter1.health -= fighter1Dmg;                                       
            }
        }
        else
        {
            //fighter2 attacks first
            if(rand()%100 <= fighter2Hit)
            {
                if(rand()%100 <= fighter2Crit)
            	    fighter2Dmg *= 3; 

                fighter1.health -= fighter1Dmg;                                       
            }                

            //fighter1 if(rand()%100 <= fighter2hit)
            if(rand()%100 <= fighter1Hit)
            {
                //attack will go through, need to attack
                if(rand()%100 <= fighter1Crit) // triples damage if critical
                    fighter1Dmg *= 3;
            
                fighter2.health -= fighter1Dmg;
            }
        }            
    }  

    //someone dies
    if(fighter1.health <= 0)
        return fighter2;
    else
	   	return fighter1;
}

/*typedef struct bracket_match{
    
}

void tournament()
{
    
}*/


double evalFitness(std::vector<double> v)
{
    return sin(30);
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
	
	for(int x : dims)
	{
		printf("N = %d\n--------------\n", x);
		bacterialOptimization(x);
		printf("\n");
	}
	
    

    return 0;
}
