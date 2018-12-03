#include <cstdio>
#include <vector>
#include <cstdlib>
#include <cmath>
#include <random>
#include <algorithm>
#include <numeric>
#include <iostream>
#include <ctime>

#include "character.hpp"

#ifndef NUM_TOURNAMENTS
#define NUM_TOURNAMENTS 100
#endif


/*
battle calculations
based on FE7 from https://fireemblem.fandom.com/wiki/Battle_Formulas

accuracy = 2*skill + 0.5*luck
dodge = 2*speed + luck
chance to hit = accuracy - dodge

phys damage = strength - defenseX_X 8
mag damage = magic - resistance

speed >= 4 
2*damage

critical = skill/2
critical evade = luck
critical chance = critical - crit evade

critical damage = damage*3
*/

char tournament_match(character_t &fighter1, character_t &fighter2)
{
    
	double fighter1Hit = fighter1.stats[SKILL]*2.0f + fighter1.stats[LUCK]/2.0f - fighter2.stats[SPEED]*2.0f + fighter2.stats[LUCK];
	double fighter2Hit = fighter2.stats[SKILL]*2.0f + fighter2.stats[LUCK]/2.0f - fighter1.stats[SPEED]*2.0f + fighter1.stats[LUCK];

    double fighter1Dmg = 0;
	double fighter2Dmg = 0;
        
    int attack_type = rand()%2; //Randomly choose attack type this turn
    if(attack_type) // if 1, physical attack. if 0, magical attack
    {
        fighter1Dmg = std::max<double>(fighter1.stats[STRENGTH] - fighter2.stats[DEFENSE], 0.1f); // a minimum of 0.1
        fighter2Dmg = std::max<double>(fighter2.stats[STRENGTH] - fighter1.stats[DEFENSE], 0.1f);
    }
    else
    {
        fighter1Dmg = std::max<double>(fighter1.stats[MAGIC] - fighter2.stats[RESISTANCE], 0.1f);
        fighter2Dmg = std::max<double>(fighter2.stats[MAGIC] - fighter1.stats[RESISTANCE], 0.1f);
    }
    
    double fighter1Crit = std::max<double>(fighter1.stats[SKILL]/2 - fighter2.stats[LUCK], 0.1f);
    double fighter2Crit = std::max<double>(fighter2.stats[SKILL]/2 - fighter1.stats[LUCK], 0.1f);
		
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

        std::cout << "Player 1 health: " << fighter1.health << std::endl;
        std::cout << "Player 2 health: " << fighter2.health << std::endl;
    }  

    //someone dies
    if(fighter1.health <= 0)
		return 2;//fighter2;
	else
		return 1;//fighter1;
}

/*typedef struct bracket_match{
    int fighter1_id = -1;
    int fighter2_id = -1;

    bracket_match *up_match = NULL;
    bracket_match *down_match = NULL;

    bracket_match *next_match = NULL;
}*/


void tournament(std::vector<character_t> &roster, int character_index)
{
	for(int i = 0; i < roster.size(); ++i)
        for(int j = 0; j < roster.size(); ++j)
            tournament_match(roster[i], roster[j]);

    return;
}

int compete(std::vector<character_t> &roster, int character_index)
{
    // Reset everybody's number of wins before we run the tournament
    for (int i = 0; i < roster.size(); i++)
    {
        roster[i].numWins = 0;  //Reset wins before we evalute them 
    }
    
	for(int i = 0; i < NUM_TOURNAMENTS; ++i)
		tournament(roster, character_index);
	
    std::cout << "Before sort\n";
    std::sort(roster.begin(), roster.end(),
            [](character_t a, character_t b) {
                return a.numWins > b.numWins;
            }
    );

    std::cout << "After sort\n";

    /* The range of the number of wins */
    return abs(roster.front().numWins - roster.back().numWins);
}