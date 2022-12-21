#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "location.h"

char* result(char* option, char cities[][4], location cities_location[], int city_count);
void given(char cities_rearrange[][4], char cities[][4], int city_count);
void farthest(char cities_rearrange[][4], char cities[][4], location cities_location[], int city_count);
int* rank_distance(double distance[], int city_count);


int main(int argc, char *argv[]){
    //check filename
    if(argc == 1){
        printf("TSP: missing filename\n");
        return 1;
    }
    //check other argv
    if(argc == 2){
        printf("TSP: invalid algorithm arguments 1\n");
        return 4;
    }
    int count_exchange = 0;
    for(int i = 2; i < argc; i++){
        if(     strcmp("-given", argv[i])
                 && strcmp("-farthest", argv[i])
                 && strcmp("-exchange", argv[i])
                 && strcmp("adjacent", argv[i])
                 && strcmp("any", argv[i]))
        {
            printf("TSP: invalid algorithm arguments\n");
            return 4;
        }
        if(!strcmp("-exchange", argv[i])){
            count_exchange++;
            if(i + 1 > argc){ //if there is no further option for exchange
                printf("TSP: invalid algorithm arguments\n");
                return 4;
            }
        }
    }
    //write options into array
    int total_option = argc - 2 - count_exchange;
    char option[total_option][19];
    for(int i = 2, j = 0; i < argc; i++, j++){
        //given or farthest
        if(!strcmp("-given", argv[i]) || !strcmp("-farthest", argv[i])){
            strcpy(option[j], argv[i]);
        }
        //exchange
        else if(!strcmp("-exchange", argv[i])){
            i++;
            if(!strcmp("adjacent", argv[i])){
                strcpy(option[j], "-exchange adjacent");
            }
            else if(!strcmp("any", argv[i])){
                strcpy(option[j], "-exchange any");
            }
            else{
                printf("TSP: invalid algorithm arguments\n");
                return 4;
            }
        }
        //other
        else{
            printf("TSP: invalid algorithm arguments\n");
            return 4;
        }
    }

    //open file, check if file exist
    FILE* cityfile = fopen (argv[1], "r");
    if(cityfile == NULL){
        printf("TSP: could not open %s\n", argv[1]);
        return 2;
    }
    // check the number of the city is not less than 2
    int city_count;
    fscanf(cityfile, "%d", &city_count);
    if(city_count < 2){
        printf("TSP: too few cities\n");
        return 3;
    }

    //read cities into array
    char cities[city_count][4];
    for(int i = 0; i < city_count; i++){
        cities[i][4] = '\0';
        fscanf(cityfile, "%s ", cities[i]);
        //printf("%s\n", cities[i]);
    }

    //read location of cities into array
    location cities_location[city_count];
    for(int i = 0; i < city_count; i++){
        fscanf(cityfile, "%lf %lf", &cities_location[i].lat, &cities_location[i].lon);
    }


    //calculate and print out result
    for(int i = 0; i < total_option; i++){
        //calculate different options
        char cities_rearrange[city_count][4];
        if(!strcmp("-given", option[i])){
            given(cities_rearrange, cities, city_count);
        }
        else if(!strcmp("-farthest", option[i])){
            farthest(cities_rearrange, cities, cities_location, city_count);
        }
        else if(!strcmp("-exchange adjacent", option[i])){
            given(cities_rearrange, cities, city_count);
        }
        else if(!strcmp("-exchange any", option[i])){
            given(cities_rearrange, cities, city_count);
        }
        else{
            printf("TSP: invalid algorithm arguments\n");
            return 4; 
        }

        char* result_sentence = result(option[i], cities_rearrange, cities_location, city_count);
        printf("%s\n", result_sentence);
        free(result_sentence);
    }
    return 0;
}



void given(char cities_rearrange[][4], char cities[][4], int city_count){
    for(int i = 0; i < city_count; i++){
        strcpy(cities_rearrange[i], cities[i]);
    }
}

void farthest(char cities_rearrange[][4], char cities[][4], location cities_location[], int city_count){
    double distance[city_count - 1];
    for(int i = 1; i < city_count; i++){
        distance[i-1] = location_distance(&cities_location[0], &cities_location[i]);
    }
    int distance_count = city_count - 1;
    int* rank = rank_distance(distance, distance_count);
    // for(int i = 0; i < city_count -1; i++){
    //     printf("i=%d, %d %lf\n", i, rank[i], distance[i]);
    // }
    //copy the value acording to the ranking index to new array.
    strcpy(cities_rearrange[0], cities[0]);
    if(city_count % 2 == 0){
        //printf("total is even");
        strcpy(cities_rearrange[city_count/2], cities[1 + rank[city_count/2-1]]);
    }
    for(int i = 0, j = 0; i < distance_count / 2; i++, j += 2){
        strcpy(cities_rearrange[city_count-1-i], cities[1 + rank[j]]);
        strcpy(cities_rearrange[i+1], cities[1 + rank[j+1]]);
    }
    free(rank);
}

int* rank_distance(double distance[], int count){
    int* rank = malloc(count * sizeof(int));
    double max = -1;
    double last_max = 100000;
    for(int i = 0; i < count; i++){
        rank[i] = 0;
    }
    for(int i = 0; i < count; i++)
    {
        for(int j = 0; j < count; j++)
        {
            if(distance[j] > max && (distance[j] < last_max))
            {
                max = distance[j];
                rank[i] = j;
            }
        }
        last_max = max;
        max = -1;
    }
    return rank;
}



char* result(char* option, char cities[][4], location cities_location[], int city_count){
    
    //calculate spaces in the option part
    int spaces = 19 - strlen(option);
    char option_spaces[spaces];
    strcpy(option_spaces, "");
    for(int i = 0; i < spaces; i++){
        strcat(option_spaces, " ");
    } 

    //calculate total_distance
    double total_distance = 0;
    for(int i = 0; i < city_count; i++){
        if(!(i + 1 == city_count)){
            total_distance += location_distance(&cities_location[i], &cities_location[i+1]);
        }
        else{
            total_distance += location_distance(&cities_location[i], &cities_location[0]);
        }
    }

    //print out city_list
    char city_list[4 * city_count];
    strcpy(city_list, "");
    for(int i = 0; i < city_count; i++){
        //printf("%s", cities[i]);
        strcpy(city_list + 4 * i, cities[i]);
        strcpy(city_list + i * 4 + 3, " ");
    }
    city_list[4 * city_count - 1] = '\0';
    
    //malloc a space for result
    char *result_sentence = malloc(1 + strlen(option) + strlen(option_spaces) + 10 + strlen(city_list));
    if (result_sentence == NULL)
        {
        return NULL;
        }
    //combine all parts into one sentence
    sprintf(result_sentence,"%s%s: %.2f %s" ,option, option_spaces, total_distance, city_list);
    return result_sentence;
}
