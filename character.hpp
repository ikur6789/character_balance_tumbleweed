#include <cstdio>
#include <vector>
#include <cstdlib>
#include <cmath>
#include <random>
#include <algorithm>
#include <numeric>
#include <iostream>
#include <ctime>

#ifndef CHARACTER_H
#define CHARACTER_H


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
            	stats.push_back(50.0); //random stat from 20-100 for every stat

            //Creating non changeable stats
            //first non-changable stat
            constValIndices[0] = rand() % NUM_STATS;

			health = stats[MAX_HP]; //Starting health is their max health stat

                //second non-changable stat, if they have one
            if(rand()%2) //"If an odd number, true, if an even number, false"
            {
                do
                {
                    constValIndices[1] = rand() % NUM_STATS;
                }
                while(constValIndices[1]!=constValIndices[0]); //Ensures two unique unchangable stats
                
            }
        }

        void generateRandomStats()
        {
            for(int s=0;s<NUM_STATS;++s)
                if(s!=constValIndices[0] && s!=constValIndices[1])
            	    stats.at(s)=((double)(rand() % MAX_STAT_VAL)+20); //random stat from 20-100 for every stat
			
			health = stats[MAX_HP]; //Starting health is their max health stat

            return;
        }

        //Data members:
    	std::vector<double> stats;
    	double              fitness;
    	double              health;         // character health for tournaments
        double              bacteriaHealth; // bacteria health for bacterial foraging algorithm
    	int                 constValIndices[2] = {-1,-1 };
    	double              popularity = 0; //All characters have 0 popularity, they gain more by winning tournaments
        unsigned            numWins = 0;


};
#endif
