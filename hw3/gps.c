#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h> 
#include "track.h"
#include "trackpoint.h"


int check_input(int argc, char *argv[]);
int store_track(track *t, FILE *file);
void print_track(const trackpoint *pt, void *t);

int main(int argc, char *argv[]){

    //check args
    int check_return = check_input(argc, argv);
    if (check_return != 0) return check_return;
    //check file exist
    FILE *file1 = fopen (argv[2], "r");
    FILE *file2 = fopen (argv[3], "r");
    if(file1 == NULL || file2 == NULL){
        if(file1 == NULL && file2 == NULL);
        else if(file1 == NULL) fclose(file2);
        else if(file2 == NULL) fclose(file1);
        fprintf(stderr, "file invalid\n");
        return 3;
    }
    //read and write tracks;
    track *t1 = track_create();
    track *t2 = track_create();
    if(store_track(t1, file1) != 0) return 4;
    if(store_track(t2, file2) != 0) return 4;

    //calculte options
    if(strcmp(argv[1], "-combine") == 0){
        track_merge(t1, t2);
        track *t = t1;
        if(track_size(t1) == 0) t = t2;
        track_for_each(t, print_track, NULL);
        track_destroy(t);
    }
    else if(strcmp(argv[1], "-closest") == 0){
        int min_dist = round(track_closest_approach(t1, t2));
        printf("%d\n", min_dist);
        track_destroy(t1);
        track_destroy(t2);
    }
    else{
        track_destroy(t1);
        track_destroy(t2);
        fprintf(stderr, "Unknown error\n");
        return 4;
    }

    fclose(file1);
    fclose(file2);
    //printf("success\n");
    return 0;
}

int check_input(int argc, char *argv[]){
    if(argc != 4){
        fprintf(stderr, "arg count invalid\n");
        return 1;
    }
    if(strcmp(argv[1], "-combine") != 0
        && strcmp(argv[1], "-closest") != 0){
        fprintf(stderr, "option invalid\n");
        return 2;
    }
    return 0;
}

int store_track(track *t, FILE *file){
    double lat, lon, time = 0;
    double time_prev = -1;
    double temp;
    int i = 0;
    while(fscanf(file, "%lf", &temp) != EOF){
        if(i % 3 == 0) lat = temp;
        else if(i % 3 == 1) lon = temp;
        else{
            time = temp;
            //check input
            if(time <= time_prev){
                fprintf(stderr, "timestamp invalid\n");
                return 1;
            }
            if(lat > 90 || lat < -90 || lon > 180 || lon < -180){
                fprintf(stderr, "lat/lon invalid\n");
                return 1;
            }
            time_prev = time;
            location *loc = location_create(lat, lon);
            trackpoint *p = trackpoint_create(loc, time);
            track_add_point(t, p);
        }
        i++;
    }
    return 0;
}

void print_track(const trackpoint *pt, void *t){
    double lat, lon, time;
    lat = trackpoint_get_latitude(pt);
    lon = trackpoint_get_longitude(pt);
    time = trackpoint_get_time(pt);
    printf("%lf %lf %lf\n", lat, lon, time);
}