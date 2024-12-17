/**
 * savvy_scheduler
 * CS 341 - Fall 2024
 */
#include "libpriqueue/libpriqueue.h"
#include "libscheduler.h"

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "print_functions.h"

/**
 * The struct to hold the information about a given job
 */
typedef struct _job_info {
    int id;

    /* TODO: Add any other information and bookkeeping you need into this
     * struct. */
    double arrival_time;
    double first_arrival_time;
    double remaining_time;
    double start_time;
    double fist_start_time;
    scheduler_info stats;
} job_info;

double total_waiting_time;
double total_turnaround_time;
double total_response_time;
int number_of_jobs;

void scheduler_start_up(scheme_t s) {
    switch (s) {
    case FCFS:
        comparision_func = comparer_fcfs;
        break;
    case PRI:
        comparision_func = comparer_pri;
        break;
    case PPRI:
        comparision_func = comparer_ppri;
        break;
    case PSRTF:
        comparision_func = comparer_psrtf;
        break;
    case RR:
        comparision_func = comparer_rr;
        break;
    case SJF:
        comparision_func = comparer_sjf;
        break;
    default:
        printf("Did not recognize scheme\n");
        exit(1);
    }
    priqueue_init(&pqueue, comparision_func);
    pqueue_scheme = s;
    // Put any additional set up code you may need here
    total_response_time = 0;
    total_turnaround_time = 0;
    total_waiting_time = 0;
    number_of_jobs = 0;
}

static int break_tie(const void *a, const void *b) {
    return comparer_fcfs(a, b);
}

int comparer_fcfs(const void *a, const void *b) {
    // TODO: Implement me!
    job_info *job_a = (job_info*)(((job*)a)->metadata);
    job_info *job_b = (job_info*)(((job*)b)->metadata);
    if (job_a->first_arrival_time < job_b->first_arrival_time) {
        return -1;
    } else if (job_a->first_arrival_time > job_b->first_arrival_time) {
        return 1;
    }
    return 0;
}

int comparer_ppri(const void *a, const void *b) {
    // Complete as is
    return comparer_pri(a, b);
}

int comparer_pri(const void *a, const void *b) {
    // TODO: Implement me!
    job_info *job_a = (job_info*)(((job*)a)->metadata);
    job_info *job_b = (job_info*)(((job*)b)->metadata);
    if (job_a->stats.priority < job_b->stats.priority) {
        return -1;
    } else if (job_a->stats.priority > job_b->stats.priority) {
        return 1;
    }
    return break_tie(a, b);
}

int comparer_psrtf(const void *a, const void *b) {
    // TODO: Implement me!
    job_info *job_a = (job_info*)(((job*)a)->metadata);
    job_info *job_b = (job_info*)(((job*)b)->metadata);
    if (job_a->remaining_time < job_b->remaining_time) {
        return -1;
    } else if (job_a->remaining_time > job_b->remaining_time) {
        return 1;
    }
    return break_tie(a, b);
}

int comparer_rr(const void *a, const void *b) {
    // TODO: Implement me!
    job_info *job_a = (job_info*)(((job*)a)->metadata);
    job_info *job_b = (job_info*)(((job*)b)->metadata);
    if (job_a->arrival_time < job_b->arrival_time) {
        return -1;
    } else if (job_a->arrival_time > job_b->arrival_time) {
        return 1;
    }
    return 0;
}

int comparer_sjf(const void *a, const void *b) {
    // TODO: Implement me!
    job_info *job_a = (job_info*)(((job*)a)->metadata);
    job_info *job_b = (job_info*)(((job*)b)->metadata);
    if (job_a->stats.running_time < job_b->stats.running_time) {
        return -1;
    } else if (job_a->stats.running_time > job_b->stats.running_time) {
        return 1;
    }
    return break_tie(a, b);
}

// Do not allocate stack space or initialize ctx. These will be overwritten by
// gtgo
void scheduler_new_job(job *newjob, int job_number, double time,
                       scheduler_info *sched_data) {
    // TODO: Implement me!
    job_info *job = malloc(sizeof(job_info));
    job->id = job_number;
    job->arrival_time = time;
    job->first_arrival_time = time;
    job->remaining_time = sched_data->running_time;
    job->start_time = -1;
    job->fist_start_time = -1;
    job->stats = *sched_data;
    newjob->metadata = job;
    number_of_jobs++;
    priqueue_offer(&pqueue, newjob);
}

job *scheduler_quantum_expired(job *job_evicted, double time) {
    // TODO: Implement me!
    if (priqueue_size(&pqueue) == 0) {
        return job_evicted;
    }
    if (job_evicted == NULL) {
        job* next_job = priqueue_poll(&pqueue);
        job_info *job_i = (job_info*)next_job->metadata;
        job_i->start_time = time;
        if (job_i->fist_start_time == -1) {
            job_i->fist_start_time = time;
        }
        return next_job;
    }
    if (pqueue_scheme == FCFS || pqueue_scheme == PRI || pqueue_scheme == SJF) {
        return job_evicted;
    }
    job_info *job_i = (job_info*)job_evicted->metadata;
    job_i->remaining_time -= time - job_i->start_time;
    job_i->arrival_time = time;
    priqueue_offer(&pqueue, job_evicted);
    job* next_job = priqueue_poll(&pqueue);
    job_i = (job_info*)next_job->metadata;
    job_i->start_time = time;
    if (job_i->fist_start_time == -1) {
        job_i->fist_start_time = time;
    }
    return next_job;
}

void scheduler_job_finished(job *job_done, double time) {
    // TODO: Implement me!
    job_info *job = (job_info*)job_done->metadata;
    total_waiting_time += time - job->first_arrival_time - job->stats.running_time;
    total_turnaround_time += time - job->first_arrival_time;
    total_response_time += job->fist_start_time - job->first_arrival_time; 
    free(((job_info*)job_done->metadata));
}

static void print_stats() {
    fprintf(stderr, "turnaround     %f\n", scheduler_average_turnaround_time());
    fprintf(stderr, "total_waiting  %f\n", scheduler_average_waiting_time());
    fprintf(stderr, "total_response %f\n", scheduler_average_response_time());
}

double scheduler_average_waiting_time() {
    return total_waiting_time / number_of_jobs;
}

double scheduler_average_turnaround_time() {
    return total_turnaround_time / number_of_jobs;
}

double scheduler_average_response_time() {
    return total_response_time / number_of_jobs;
}

void scheduler_show_queue() {
    // OPTIONAL: Implement this if you need it!
}

void scheduler_clean_up() {
    priqueue_destroy(&pqueue);
    print_stats();
}
