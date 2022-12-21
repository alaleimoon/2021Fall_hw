#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "location.h"


typedef struct _city
{
    char name[4];
    location loc;
} city;


int check_input(int argc, char *argv[], int* total_option);
int w_options(int argc, char *argv[], char** options);
void rw_input(int argc, char *argv[], city **cities, int city_count, FILE* cityfile);
void result(char* option, city** cities, int new_count);
void given(city** cities_newlist, city** cities, int new_count);
void farthest(city** cities_newlist, city** cities_copy2, city** cities, int city_count);
void farthest_reorder(city** cities_newlist, city** cities_copy2, city** cities, int new_count);
void copy_city(city* des, city* src);
void adjacent(city** cities_newlist, city** cities_copy2, city** cities, int city_count);
double ext_dist(city** cities, int a, int b, int new_count, int adj0_or_any1);
void adjacent_reorder(city** cities_newlist, city** cities_copy2, city** cities, int city_count);
void print_list(city** list, int count);
double total_dist(city** cities, int new_count);
void swap(city** cities_copy3, int pair1, int pair2, int new_count);
void any(city** cities_newlist, city** cities_copy2, city** cities, int city_count);
void free_cities(city** cities, int new_count);
int index_count(city** cities, char* last, char* second, int city_count);
city** create_cities(city** cities, int city_count);
void any_copy(city** cities_newlist, city** cities, int new_count, int a, int b);
void free_option(char** cities, int new_count);

int main(int argc, char *argv[]){

    //check input and store options
    int total_option;
    int check_return = check_input(argc, argv, &total_option);
    if (!check_return == 0){
        return check_return;
    }

    //initialize array to store options
    char** options = NULL;
    options = malloc(total_option * sizeof(*options));
    for (int i = 0; i < total_option; i++){
        options[i] = malloc(20 * sizeof(char));
    }

    check_return = w_options(argc, argv, options);
    if (!check_return == 0){
        free_option(options, total_option);
        return check_return;
    }

    //open file, check if file exist
    FILE* cityfile = fopen (argv[1], "r");
    if(cityfile == NULL){
        fclose(cityfile);
        free_option(options, total_option);
        fprintf(stderr, "TSP: could not open %s\n", argv[1]);
        return 2;
    }
    
    // check the number of the city is not less than 2
    int city_count;
    fscanf(cityfile, "%d", &city_count);
    if(city_count < 2){
        fprintf(stderr, "TSP: too few cities\n");
        fclose(cityfile);
        free_option(options, total_option);
        return 3;
    }

    //create arr to store inf of cities
    city **cities = NULL;
    cities = malloc((city_count + 1) * sizeof(city*));
    for(int i = 0; i < city_count + 1; i++){
        cities[i] = malloc(sizeof(city));
        cities[i]->name[4] = '\0';
    }

    //read and write name and location into arry
    rw_input(argc, argv, cities, city_count, cityfile);
    copy_city(cities[city_count], cities[0]);
    fclose(cityfile);
    
    //calculate and print out result
    for(int i = 0; i < total_option; i++){
        //initialize newlist after sorting
        city** cities_newlist = create_cities(cities, city_count);
        city** cities_copy2 = create_cities(cities, city_count);
        city** result_cities;

        //calculate different options
        if(!strcmp("-given", options[i])){
            result_cities = cities;
            //given(cities_newlist, cities, city_count+1);
        }
        else if(!strcmp("-farthest", options[i])){
            given(cities_copy2, cities, city_count+1);
            farthest(cities_newlist, cities_copy2, cities, city_count);
            result_cities = cities_newlist;
        }
        else if(!strcmp("-exchange adjacent", options[i])){
            given(cities_copy2, cities, city_count+1);
            adjacent(cities_newlist, cities_copy2, cities, city_count);
            result_cities = cities_newlist;
        }
        else if(!strcmp("-exchange any", options[i])){
            given(cities_copy2, cities, city_count+1);
            any(cities_newlist, cities_copy2, cities, city_count);
            result_cities = cities_newlist;
        }
        else{
            fprintf(stderr, "TSP: invalid algorithm arguments\n");
            return 4; 
        }

        result(options[i], result_cities, city_count+1);
        free(options[i]);
        free_cities(cities_newlist, city_count+1);
        free_cities(cities_copy2, city_count+1);
    }

    free(options);
    free_cities(cities, city_count+1);
    return 0;
}


int check_input(int argc, char *argv[], int* total_option){
    //check filename
    if(argc == 1){
        fprintf(stderr, "TSP: missing filename\n");
        return 1;
    }
    //check other argv
    if(argc == 2){
        fprintf(stderr, "TSP: invalid algorithm arguments\n");
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
            fprintf(stderr, "TSP: invalid algorithm arguments\n");
            return 4;
        }
        if(!strcmp("-exchange", argv[i])){
            count_exchange++;
            if(i + 1 > argc){ //if there is no further option for exchange
                fprintf(stderr, "TSP: invalid algorithm arguments\n");
                return 4;
            }
        }
    }
    *total_option = argc - 2 - count_exchange;
    return 0;
}


int w_options(int argc, char *argv[], char** options){
    //write options into array
    for(int i = 2, j = 0; i < argc; i++, j++){
        //given or farthest
        if(!strcmp("-given", argv[i]) || !strcmp("-farthest", argv[i])){
            strcpy(options[j], argv[i]);
        }
        //exchange
        else if(!strcmp("-exchange", argv[i])){
            i++;
            if(i + 1 > argc){
                fprintf(stderr, "TSP: invalid algorithm arguments\n");
                return 4;
            }
            if(!strcmp("adjacent", argv[i])){
                strcpy(options[j], "-exchange adjacent");
            }
            else if(!strcmp("any", argv[i])){
                strcpy(options[j], "-exchange any");
            }
            else{
                fprintf(stderr, "TSP: invalid algorithm arguments\n");
                return 4;
            }
        }
        //other
        else{
            fprintf(stderr, "TSP: invalid algorithm arguments\n");
            return 4;
        }
        //printf("%s\n",options[j]);
    }
    return 0;
}


void rw_input(int argc, char *argv[], city **cities, int city_count, FILE* cityfile){
    
    //read city names into array
    for(int i = 0; i < city_count; i++){
        fscanf(cityfile, "%s ", cities[i]->name);
        //printf("%s\n", cities[i]->name);
    }

    //read location of cities into array
    for(int i = 0; i < city_count; i++){
        fscanf(cityfile, "%lf %lf", &(cities[i]->loc.lat), &(cities[i]->loc.lon));
        //printf("%lf %lf\n", cities[i]->loc.lat, cities[i]->loc.lon);
    }
}

void result(char* option, city** cities, int new_count){
    
    printf("%s" ,option);

    //calculate spaces in the option part
    int spaces = 18 - strlen(option);
    for(int i = 0; i < spaces; i++){
        printf(" ");
    } 
    printf(":");

    //calculate total_distance
    double total_distance = total_dist(cities, new_count);

    //calculate spaces in the distance part
    char *dist = malloc(16 * sizeof(char));
    sprintf(dist,"%.2f", total_distance);
    int spaces2 = 14 - strlen(dist) - 1;
    for(int i = 0; i < spaces2; i++){
        printf(" ");
    } 
    printf("%.2f ",total_distance);

    //print out city_list
    for(int i = 0; i < new_count; i++){
        printf("%s", cities[i]->name);
        if(!(i * 4 + 3 == 4 * new_count - 1)){
            printf(" ");
        }
    }
    free(dist);
    printf("\n");
}



void given(city** cities_newlist, city** cities, int new_count){
    for(int i = 0; i < new_count-1; i++){
        copy_city(cities_newlist[i], cities[i]);
    }
    copy_city(cities_newlist[new_count-1], cities[0]);
}

void farthest(city** cities_newlist, city** cities_copy2, city** cities, int city_count){
    // sorting max distance
    for(int i = 0; i < city_count - 1; i++){
        int max_index = i;
        double max_dis = 0;
        double dis = 0;
        for(int j = i + 1; j < city_count; j++){ 
            dis = location_distance(&(cities_copy2[i]->loc), &(cities_copy2[j]->loc));
            if(dis > max_dis){
                max_dis = dis;
                max_index = j;
            }
        }
        city* temp = malloc(sizeof(city));
        copy_city(temp, cities_copy2[i+1]);
        if(!(i+1 == max_index)){
            copy_city(cities_copy2[i+1], cities_copy2[max_index]);
        }
        copy_city(cities_copy2[max_index], temp);
        free(temp);
    }
    farthest_reorder(cities_newlist, cities_copy2, cities, city_count + 1);
}

void copy_city(city* des, city* src){
    strcpy(des->name, src->name);
    des->loc = src->loc;
}

void farthest_reorder(city** cities_newlist, city** cities_copy2, city** cities, int new_count){
    for(int i = 1, j = 1; i < new_count - 1; i += 2, j++){
        copy_city(cities_newlist[new_count-j-1], cities_copy2[i]);
        if(!(i + 1 == new_count - 1)){
            copy_city(cities_newlist[j], cities_copy2[i+1]);
        }
    }
    copy_city(cities_newlist[0], cities[0]);
    copy_city(cities_newlist[new_count-1], cities[0]);
}

void adjacent(city** cities_newlist, city** cities_copy2, city** cities, int city_count){
    int new_count = city_count + 1;
    // sorting max distance
    while(1){
        int pair1 = 0;
        int pair2 = 0;
        double max_dis = 0;
        double dis = 0;

        for(int j = 0; j < city_count; j++){
            //initialize copy3
            city** cities_copy3 = create_cities(cities, city_count);
            given(cities_copy3, cities_copy2, city_count+1);

            //calculate decreased distance
            swap(cities_copy3, j, j+1, new_count);
            dis = ext_dist(cities_copy2, j, j+1, new_count, 0)
                - ext_dist(cities_copy3, j, j+1, new_count, 0);

            free_cities(cities_copy3, city_count+1);
            //find the max distance and write
            if(dis > max_dis){
                max_dis = dis;
                pair1 = j;
                pair2 = j + 1;
            }
            //printf("%lf\n", max_dis);
        }
        //end the loop
        if(max_dis == 0){
            break;
        }
        //swap a pair of cities
        swap(cities_copy2, pair1, pair2, city_count+1);
    }
    adjacent_reorder(cities_newlist, cities_copy2, cities, city_count);
}

int check_num(int i, int city_count){
    if(i > city_count - 1){
        i -= city_count;
    }
    if(i < 0){
        i += city_count;
    }
    return i;
}

void swap(city** cities, int pair1, int pair2, int new_count){
    int city_count = new_count - 1;
    pair1 = check_num(pair1, city_count);
    pair2 = check_num(pair2, city_count);

    city* temp = malloc(sizeof(city));
    copy_city(temp, cities[pair1]);
    copy_city(cities[pair1], cities[pair2]);
    copy_city(cities[pair2], temp);
    free(temp);
    copy_city(cities[new_count-1], cities[0]);
}



void adjacent_reorder(city** cities_newlist, city** cities_copy2, city** cities, int city_count){
    //find the first city -> determine the copy start
    int firse_index = 0;
    for(int i = 0; i < city_count; i++){
        if(!strcmp(cities[0]->name, cities_copy2[i]->name)){
            firse_index = i;
            //printf("%d", i);
        }
    }
    //find the second city-> determine the copy direction
    int last = firse_index-1;
    int second = firse_index+1;
    if(last<0){
        last = city_count - 1;
    }
    if(second > city_count - 1){
        second = 0;
    }
    int check_reverse = index_count(cities, cities_copy2[last]->name, cities_copy2[second]->name, city_count);
    int reverse = 1;
    //printf("%d",check_reverse);
    if(check_reverse > 0){
         reverse = -1;
     }

    //loop to reorder the list
    for(int i = 0, j = firse_index; i < city_count; i++, j += reverse){
        if(j > city_count - 1){
            j = 0;
        }
        else if(j < 0){
            j = city_count - 1;
        }
        copy_city(cities_newlist[i], cities_copy2[j]);
        //print_list(cities_copy2, city_count+1);
    }
    copy_city(cities_newlist[city_count], cities[0]);
}

void print_list(city** list, int count){
    for(int i = 0; i < count; i++){
        printf("%s,", list[i]->name);
    }
    printf("\n");
}

double total_dist(city** cities, int new_count){
    double total_distance = 0;
    for(int i = 0; i < new_count - 1; i++){
        total_distance += location_distance(&(cities[i]->loc), &(cities[i+1]->loc));
    }
    return total_distance;
}

double ext_dist(city** cities, int a, int b, int new_count, int adj0_or_any1){
    double adj_dist = 0;
    int a_former = check_num(a-1, new_count-1);
    int b_later = check_num(b+1, new_count-1);
    adj_dist += location_distance(&(cities[a_former]->loc), &(cities[a]->loc));
    adj_dist += location_distance(&(cities[b]->loc), &(cities[b_later]->loc));
    if(adj0_or_any1){
        int a_later = check_num(a+1, new_count-1);
        int b_former = check_num(b-1, new_count-1);
        adj_dist += location_distance(&(cities[a]->loc), &(cities[a_later]->loc));
        adj_dist += location_distance(&(cities[b_former]->loc), &(cities[b]->loc));
    }
    return adj_dist;
}







void any(city** cities_newlist, city** cities_copy2, city** cities, int city_count){
    int new_count = city_count + 1;
    // sorting max distance
    while(1){
        int pair1 = 0;
        int pair2 = 0;
        double max_dis = 0;
        double dis = 0;
        for(int i = 0; i < city_count-1; i++){
            for(int j = i+1; j < city_count; j++){
                //initialize copy3
                city** cities_copy3 = create_cities(cities, city_count);
                any_copy(cities_copy3, cities_copy2, city_count+1, i, j);
                //calculate decreased distance
                swap(cities_copy3, i, j, new_count);
                dis = ext_dist(cities_copy2, i, j, new_count, 1)
                    - ext_dist(cities_copy3, i, j, new_count, 1);

                free_cities(cities_copy3, new_count);
                //find the max distance and write
                if(dis > max_dis){
                    max_dis = dis;
                    pair1 = i;
                    pair2 = j;
                }
            }
        }

        //end the loop
        if(max_dis == 0){
            break;
        }
        //swap a pair of cities
        swap(cities_copy2, pair1, pair2, city_count+1);
    }

    adjacent_reorder(cities_newlist, cities_copy2, cities, city_count);
}

void any_copy(city** cities_newlist, city** cities, int new_count, int a, int b){
    int a_former = check_num(a-1, new_count-1);
    int b_later = check_num(b+1, new_count-1);
    int a_later = check_num(a+1, new_count-1);
    int b_former = check_num(b-1, new_count-1);
    copy_city(cities_newlist[a], cities[a]);
    copy_city(cities_newlist[b], cities[b]);
    copy_city(cities_newlist[a_former], cities[a_former]);
    copy_city(cities_newlist[b_former], cities[b_former]);
    copy_city(cities_newlist[a_later], cities[a_later]);
    copy_city(cities_newlist[b_later], cities[b_later]);
}


void free_option(char** cities, int new_count){
    for(int i = 0; i < new_count; i++){
        //free(cities[i]->name);
        free(cities[i]);
    }
    free(cities);
}


void free_cities(city** cities, int new_count){
    for(int i = 0; i < new_count; i++){
        //free(cities[i]->name);
        free(cities[i]);
    }
    free(cities);
}

int index_count(city** cities, char* last, char* second, int city_count){
    int last_input;
    int second_input;
    for(int i = 0; i < city_count; i++){
        if(!strcmp(cities[i]->name, last)){
            last_input = i;
            //printf("last_input %d\n",i);
        }
        if(!strcmp(cities[i]->name, second)){
            second_input = i;
            //printf("second_input %d\n",i);
        }
    }
    return(second_input - last_input);
}

city** create_cities(city** cities, int city_count){
    city** cities_copy =  malloc((city_count+1) * sizeof(city*));
    for(int i = 0; i < city_count+1; i++){
        cities_copy[i] = malloc(sizeof(city));
        strcpy(cities_copy[i]->name, "");
    }
    return cities_copy;
}