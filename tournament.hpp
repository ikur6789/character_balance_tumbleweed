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
#define NUM_TOURNAMENTS 1
#endif


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

char tournament_match(character_t fighter1, character_t fighter2)
{
	int fighter1Hit = fighter1.stats[SKILL]*2 + fighter1.stats[LUCK]/2 - fighter2.stats[SPEED]*2 + fighter2.stats[LUCK];
	int fighter2Hit = fighter2.stats[SKILL]*2 + fighter2.stats[LUCK]/2 - fighter1.stats[SPEED]*2 + fighter1.stats[LUCK];

	if(fighter1Hit <= 10)
		fighter1Hit = 10;
	if(fighter2Hit <= 10)
		fighter2Hit = 10;

    int fighter1Dmg = 0;
	int fighter2Dmg = 0;
        
    int attack_type = rand()%2; //Randomly choose attack type this turn
    if(attack_type) // if 1, physical attack. if 0, magical attack
    {
        fighter1Dmg = std::max<int>(fighter1.stats[STRENGTH] - fighter2.stats[DEFENSE], 1);
        fighter2Dmg = std::max<int>(fighter2.stats[STRENGTH] - fighter1.stats[DEFENSE], 1);
	}
    else
    {
        fighter1Dmg = std::max<int>(fighter1.stats[MAGIC] - fighter2.stats[RESISTANCE], 1);
        fighter2Dmg = std::max<int>(fighter2.stats[MAGIC] - fighter1.stats[RESISTANCE], 1);
    }

	if(fighter1Dmg <= 0)
		fighter1Dmg = 10;
	if(fighter2Dmg <= 0)
		fighter2Dmg = 10;
    
    int fighter1Crit = std::max<int>(fighter1.stats[SKILL]/2 - fighter2.stats[LUCK], 1);
    int fighter2Crit = std::max<int>(fighter2.stats[SKILL]/2 - fighter1.stats[LUCK], 1);
		
    //fight to the death, 1v1 final destination no items fox only
    while( (fighter1.health > 0) && (fighter2.health > 0) )
	{
		
        //The faster of two will attack first
        if(fighter1.stats[SPEED]>fighter2.stats[SPEED]) //fighter 1 will attack first
        {
            //fighter1
            if(rand()%50 <= fighter1Hit) //roll a chance to hit
            {
                //attack will go through, need to attack
                if(rand()%100 <= fighter1Crit) // triples damage if critical
					fighter1Dmg *= 3;

                fighter2.health -= fighter1Dmg;
            }

            //fighter2
            if(rand()%50 <= fighter2Hit)
            {
                if(rand()%100 <= fighter2Crit)
				    fighter2Dmg *= 3; 

                fighter1.health -= fighter1Dmg;                                       
            }
        }
        else
        {
            //fighter2 attacks first
            if(rand()%50 <= fighter2Hit)
            {
                if(rand()%100 <= fighter2Crit)
            	    fighter2Dmg *= 3; 
			
                fighter1.health -= fighter1Dmg;                                       
            }                

            //fighter1 if(rand()%100 <= fighter2hit)
            if(rand()%50 <= fighter1Hit)
            {
                //attack will go through, need to attack
                if(rand()%100 <= fighter1Crit) // triples damage if critical
            	    fighter1Dmg *= 3;
			
                fighter2.health -= fighter1Dmg;
            }
        }
    }  

    //someone dies, break loop and return winner
    if(fighter1.health <= 0)
		return 2;//fighter2;
	else
		return 1;//fighter1;
}


void tournament(std::vector<character_t> &roster, character_t &character, int character_index)
{
	char winner;    //winner of tournament
	for(int i = 0; i < roster.size(); ++i)
        for(int j = 0; j < roster.size(); ++j)
			if(i != j) //if they're not the same person fighting
	        {
                if (i == character_index)      
                    winner = tournament_match(character, roster[j]);
                else
                    if (j == character_index) 
                        winner = tournament_match(roster[i], character);
                else
                    winner = tournament_match(roster[i], roster[j]);

				switch(winner)  //adds to character's number of wins record
				{
					case 1:
                        if (i == character_index) 
                            character.numWins++;
						else                      
                            roster[i].numWins++;
						break;
					case 2:
                        if (j == character_index) 
                            character.numWins++;
						else
                            roster[j].numWins++;
						break;

					default:
						break;
				}
			}

    return;
}

int compete(std::vector<character_t> &roster, character_t &character, int character_index)
{
    // Reset everybody's number of wins before we run the tournament
    for (int i = 0; i < roster.size(); i++)
        roster[i].numWins = 0;  //Reset wins before we evalute them 

    character.numWins = 0;
	
    for(int i = 0; i < NUM_TOURNAMENTS; ++i)
		tournament(roster, character, character_index);
	
    /* We need to exclude the fighter at character_index since he will
     * always be zero and this will throw off the range */
    std::vector<character_t> temp_roster;
    for (int i = 0; i < roster.size(); ++i) {
        if (i != character_index) {
            temp_roster.push_back(roster.at(i));
        }
    }

    //sorts by number of wins
    std::sort(temp_roster.begin(), temp_roster.end(),
            [](character_t a, character_t b) {
                return a.numWins > b.numWins;
            }
    );

    /* The range of the number of wins */
    return abs(temp_roster.front().numWins - temp_roster.back().numWins);
}