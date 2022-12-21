#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h> 

#include "track.h"

typedef struct _node
{
    trackpoint *point;
    struct _node *next;
    struct _node *prev;
} node;

struct _track
{
    node head;
    node tail;
    size_t size;
};

void track_initialize(track *l);
void track_add_node_end(track *t, node *to_add);
void track_remove_node(track *t, node *n);
void node_destroy(node *n);
track *track_copy(const track *t);

/**
 * Creates an empty GPS track.
 * 
 * @return a pointer to the track, or NULL if allocation was unsuccessful
 */
track *track_create(){
    track *new = malloc(sizeof(*new));
    if (new != NULL){
        track_initialize(new);
        return new;
    }
    else return NULL;
}

void track_initialize(track *l){
    l->head.point = NULL;
    l->tail.point = NULL;
    l->head.next = &l->tail;
    l->tail.prev = &l->head;
    l->head.prev = NULL;
    l->tail.next = NULL;
    l->size = 0;
}


/**
 * Destroys the given track.  The track is invalid after it has been destroyed.
 * There is no effect if the parameter is NULL.
 *
 * @param l a pointer to a track
 */
void track_destroy(track *l){
    if (l != NULL){
        node *curr = l->head.next;
        while(curr != &l->tail){
            node *old_next = curr->next;
            node_destroy(curr);
            curr = old_next;
        }
        l->size = 0;
        free(l);
    }
}

void node_destroy(node *n){
    trackpoint_destroy(n->point);
    free(n);
}

/**
 * Returns the number of trackpoints on the given track.
 *
 * @param t a pointer to a valid track, non-NULL
 * @return the number of points in t
 */
size_t track_size(const track *t){
    size_t num = t->size;
    return num;
}


/**
 * Add a the given trackpoint to the end of the given track.  The
 * add is successful if the time of the new trackpoint is strictly
 * after the time of the last trackpoint in the track (or that track
 * was empty) and there were no allocation errors.  If the add was
 * successful, then ownership of the trackpoint is transferred
 * to the track.
 *
 * @param t a pointer to a track, non-NULL
 * @param p a pointer to a trackpoint, non-NULL
 * @return true if and only if the point was successfully added
 */
bool track_add_point(track *t, trackpoint *p){
    if (t == NULL || p == NULL) return false;
    if (track_size(t) != 0){
        if (trackpoint_get_time(p) <= (trackpoint_get_time(t->tail.prev->point))){
            return false;
        }
    }
    node *added = malloc(sizeof(*added));
    if (added != NULL){
        added->point = p;
        track_add_node_end(t, added);
        return true;
    }
    else return false;
}

void track_add_node_end(track *t, node *to_add){
    to_add->next = &t->tail;
    to_add->prev = t->tail.prev;
    to_add->next->prev = to_add;
    to_add->prev->next = to_add;
    t->size++;
}

/**
 * Returns the time between the first trackpoint and the last trackpoint in
 * the given track, in seconds.  Returns zero if the track is empty.
 *
 * @param t a pointer to a track, non-NULL
 * @return the time between the first and last trackpoints on that track
 */
double track_length(const track *t){
    if (track_size(t) >= 2){
        double time_diff = trackpoint_get_time(t->head.next->point) - trackpoint_get_time(t->tail.prev->point);
        if (time_diff < 0) return (0 - time_diff);
        return time_diff;
    }
    return 0;
}


/**
 * Returns a copy of the location of the trackpoint on this track with
 * the given time.  If there is no such trackpoint then the returned
 * location is determined by linear interpolation between the first
 * trackpoints before and after the given time.  If the track is empty
 * or if the given time is strictly before the time of the first
 * trackpoint or strictly after the time of the last trackpoint, then
 * the returned value is NULL.  If there is an error allocating the
 * location to return then the returned value is NULL.  Otherwise, it
 * is the caller's responsibility to destroy the returned location.
 *
 * @param t a pointer to a track, non-NULL
 * @param time a double
 * @return a copy of the trackpoint with the given time, or a new interpolated
 * trackpoint, or NULL
 */
location *track_get(const track *t, double time){
    if (t == NULL
    || time < trackpoint_get_time(t->head.next->point)
    || time > trackpoint_get_time(t->tail.prev->point)
    ) return NULL;
    //loop to find or find prev&next;
    location *loc_copy = NULL;
    node *curr = t->head.next;
    for (int i = 0; i < t->size; i++){
        double curr_time = trackpoint_get_time(curr->point);
        if (time == curr_time){
            const location *loc = trackpoint_get_location(curr->point);
            double lat = location_get_latitude(loc);
            double lon = location_get_longitude(loc);
            loc_copy = location_create(lat, lon);
            break;
        }
        else if (time < curr_time){
            location const *start = trackpoint_get_location(curr->prev->point);
            location const *end = trackpoint_get_location(curr->point);
            //calculate the weight
            double time_prev = trackpoint_get_time(curr->prev->point);
            double time_next = curr_time;
            double weight = (time - time_prev) / (time_next - time_prev);
            if(weight < 0) weight = (0 - weight);
            loc_copy = location_interpolate(start, end, weight);
            break;
        }
        curr = curr->next;
    }
    return loc_copy;
}


/**
 * Passes each point in the given track to the given function.  Points
 * are passed to the function in increasing chronological order.
 *
 * @param t a pointer to a valid track, non-NULL
 * @param f the function to call for each point
 * @param arg an extra argument to pass to the function
 */
void track_for_each(const track *t, void (*f)(const trackpoint *, void *), void *arg){
    node *curr = t->head.next;
    while (curr != &t->tail){
        f(curr->point, arg);
        curr = curr->next;
    }
}


/**
 * Merges the second tracks into the first track.  All points from the
 * second track are added to the first track, except that a trackpoint
 * already present in the first track is not duplicated if also
 * present in the second track, and conflicting trackpoints (two trackpoints,
 * one from the first track and one from the second, with the same
 * timestamp but different locations) are removed from the merged
 * track and destroyed.  The second track is destroyed by
 * this operation, with all of its trackpoints either transferred to the
 * first track or destroyed.  The resulting first track has all trackpoints
 * in strictly increasing chronological order.
 *
 * @param dest a pointer to a track, non-NULL
 * @param src a pointer to a track, non-NULL
 */

void track_merge(track *dest, track *src){
    node *n1 = dest->head.next;
    node *n2 = src->head.next;
    //if dest have no point
    if(track_length(dest) == 0){
        track *temp = dest;
        dest = src;
        src = temp;
    }
    //if src have no point
    if(track_length(src) == 0) return;
    //reverse if dest is not before src
    if(trackpoint_compare(n2->point, n1->point) < 0){
        // track *temp = dest;
        // dest = src;
        // src = temp;
        // track_merge(dest, src);
        // return;

        //exchange size
        int temp = dest->size;
        dest->size = src->size;
        src->size = temp;
        //exchange elements
        node *dest_head = dest->head.next;
        node *dest_tail = dest->tail.prev;
        dest->head.next = src->head.next;
        dest->tail.prev = src->tail.prev;
        src->head.next = dest_head;
        src->tail.prev = dest_tail;
        //reset n1 n2 pointer
        n1 = dest->head.next;
        n2 = src->head.next;
    } 
    
    //find the overlap start
    //slice dest
    track *t3 = track_create();
    n1 = dest->tail.prev;
    while(n1->point != NULL && n2->point != NULL
                    && trackpoint_compare(n2->point, n1->point) <= 0){
        node *curr = n1;
        n1 = n1->prev;
        track_remove_node(dest, curr);
        track_add_node_end(t3, curr);
    }
    
    //insert in overlap area
    node *n3 = t3->tail.prev;
    while(src->size > 0 || t3->size > 0){
        //find the overlap end, add all others onto the dest
        if((t3->size == 0) && (src->size == 0)){
            break;
        }
        else if(t3->size == 0){//if slice part end but remain some src array
            dest->size = dest->size + src->size;
            dest->tail.prev->next = src->head.next;
            src->tail.prev->next = &dest->tail;
            dest->tail.prev = src->tail.prev;
            src->head.next = &src->tail;
            src->tail.prev = &src->head;
            src->size = 0;
            break;
        }
        else if(src->size == 0){
            node *curr = n3;
            n3 = n3->prev;
            track_remove_node(t3, curr);
            track_add_node_end(dest, curr);
        }
        else{ //insert in overlap area
            int cmp = trackpoint_compare(n3->point,n2->point);
            if(cmp == 0 && (location_compare(trackpoint_get_location(n3->point), trackpoint_get_location(n2->point)) != 0)){
                //if conflict
                printf("%lf %lf %lf\n",trackpoint_get_latitude(n3->point), trackpoint_get_longitude(n3->point), trackpoint_get_time(n3->point));
                printf("%lf %lf %lf\n",trackpoint_get_latitude(n2->point), trackpoint_get_longitude(n2->point), trackpoint_get_time(n2->point));
                node *curr = n3;
                n3 = n3->prev;
                track_remove_node(t3, curr);
                node_destroy(curr);

                curr = n2;
                n2 = n2->next;
                track_remove_node(src, curr);
                node_destroy(curr);
            }
            else if (cmp <= 0){ //insert slice or duplicated
                node *curr = n3;
                n3 = n3->prev;
                track_remove_node(t3, curr);
                track_add_node_end(dest, curr);
                if(cmp == 0){//duplicate
                    curr = n2;
                    n2 = n2->next;
                    track_remove_node(src, curr);
                    node_destroy(curr);
                }
            }
            else { //insert src
                node *curr = n2;
                n2 = n2->next;
                track_remove_node(src, curr);
                track_add_node_end(dest, curr);
            }
        }
    }
    track_destroy(t3);
    track_destroy(src);
    return;
}

void track_remove_node(track *t, node *n){
    node *before = n->prev;
    node *after = n->next;
    n->next = NULL;
    n->prev = NULL;
    before->next = after;
    after->prev = before;
    t->size --;
}

/**
 * Returns an approximation of the closest approach of the two tracks.
 * The approximation is obtained by a process equivalent in result
 * to the following:
 * 1) get the timestamps of all trackpoints in both tracks combined
 * 2) eliminate those timestamps not between the start and end of both tracks
 * 3) determine the location at each remaining time on both tracks,
 *    where if one track has no trackpoint with a matching timestamp,
 *    the location on that track at that time is determined by
 *    linear interpolation on the latitude and longitude (separately)
 *    of the next earliest and next latest trackpoints
 * 4) for each pair of locations corresponding to match times,
 *    compute the distance between those locations using location_distance
 * 5) return the minimum of all the distances from the previous step,
 *    or INFINITY if there were no remaining timestamps after step 2
 * 
 * @param dest a pointer to a track, non-NULL
 * @param src a pointer to a track, non-NULL
 * @return a double giving the distance in km of the closest approach,
 * or INFINITY
 */
double max(double a, double b){
    if(a > b) return a;
    else return b;
}
double min(double a, double b){
    if(a > b) return b;
    else return a;
}
double track_closest_approach(const track *track1, const track *track2){
    //make copies of 2 tracks
    track *t1 = track_copy(track1);
    track *t2 = track_copy(track2);
    //merge
    track_merge(t1, t2);
    track *t = t1;
    if(track_size(t1) == 0) t = t2;
    //get the timelist
    double overlap_head = max(trackpoint_get_time(track1->head.next->point),
                            trackpoint_get_time(track2->head.next->point));
    double overlap_tail = min(trackpoint_get_time(track1->tail.prev->point),
                            trackpoint_get_time(track2->tail.prev->point));
    double min_dist = INFINITY;
    node* curr = t->head.next;
    while(curr != &t->tail){
        double time = trackpoint_get_time(curr->point);
        if(time > overlap_head && time < overlap_tail){
            location* loc1 = track_get(track1, time);
            location* loc2 = track_get(track2, time);
            //compute distance
            double distance = location_distance(loc1, loc2);
            //write max
            if(distance < min_dist) min_dist = distance;
            //printf("%lf, min:%lf\n", distance, min_dist);
            location_destroy(loc1);
            location_destroy(loc2);
        }
        curr = curr->next;
    }
    track_destroy(t);
    return min_dist;
}

track *track_copy(const track *t){
    track *new = track_create();
    node* curr = t->head.next;
    while(curr != &t->tail){
        double lat = trackpoint_get_latitude(curr->point);
        double lon = trackpoint_get_longitude(curr->point);
        double time = trackpoint_get_time(curr->point);
        location *loc = location_create(lat, lon);
        trackpoint *p = trackpoint_create(loc, time);
        track_add_point(new, p);
        curr = curr->next;
    }
    return new;
}