#include <stdio.h>
#include <unistd.h>
#include <vector>
#include "character.hpp"

void printInitCSV(std::vector<character_t> population)
{
    FILE *f;
    if(!access("init_stats.csv", F_OK))
    {
        fprintf(stderr, "Starting stats file already exsists, please move or rename before running again\n");
        return;
    }
    else
    {
        f = fopen("init_stats.csv", "w");

        fprintf(f, "Fighter,MaxHP,Stength,Magic,Skill,Speed,Luck,Defense,Resistance\n");

        for(int i=0;i<population.size();++i)
        {
            fprintf(f, "%d,", i);
            for(int j=0;j<NUM_STATS;++j)
                fprintf(f, "%f,", population.at(i).stats[j]);
            fprintf(f, "\n");
        }

        fclose(f);
        return;
    }

}

void printResultsCSV(std::vector<character_t> population)
{
    FILE *f;
    if(!access("results.csv", F_OK))
    {
        fprintf(stderr, "Results file already exsists, please move or rename before running again\n");
        fprintf(stderr, "NOTE: No results will be saved to disk this run\n");
        return;
    }
    else
    {
        f = fopen("results.csv", "w");

        fprintf(f, "Fighter,fitnessVal,MaxHP,Stength,Magic,Skill,Speed,Luck,Defense,Resistance\n");

        for(int i=0;i<population.size();++i)
        {
            fprintf(f, "%d,%f,", i, population.at(i).fitness);
            for(int j=0;j<NUM_STATS;++j)
                fprintf(f, "%f,", population.at(i).stats[j]);
            fprintf(f, "\n");
        }

        fclose(f);
        return;
    }

}

