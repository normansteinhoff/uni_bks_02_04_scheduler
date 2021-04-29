/*
 * Autor: Thomas Tegethoff
 * Idee & Vorlage: Heiko Will
 */

#include <inttypes.h>
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#define WRAPPER
#include "scheduler.h"

#ifndef CONFIG_PROCS
#define CONFIG_PROCS                                                           \
	{                                                                      \
		{                                                              \
			.atime = 0,                                            \
			.rtime = 3,                                            \
		},                                                             \
			{                                                      \
				.atime = 2,                                    \
				.rtime = 7,                                    \
			},                                                     \
			{                                                      \
				.atime = 4,                                    \
				.rtime = 1,                                    \
			},                                                     \
		{                                                              \
			.atime = 6, .rtime = 5,                                \
		}                                                              \
	}
#endif

/* Processes' arrival and runtime */
static const struct {
	unsigned atime;
	unsigned rtime;
} processes[] = CONFIG_PROCS;
#define NUM_PROCESSES (sizeof(processes) / sizeof(processes[0]))
#undef CONFIG_PROCS

/* Implemented algorithms */
static const struct {
	const char *const name;
	void (*const func)(struct process *);
} algos[] = {
	{
		.name = "Round Robin",
		.func = rr,
	},
	{
		.name = "First Come First Serve",
		.func = fcfs,
	},
	{
		.name = "Shortest Process Next",
		.func = spn,
	},
	{
		.name = "Shortest Remaining Time Next",
		.func = srt,
	},
	{
		.name = "Highest Response Ration Next",
		.func = hrrn,
	},
};
#define NUM_ALGOS (sizeof(algos) / sizeof(algos[0]))

/* Define empty fallback functions */
static void empty(struct process *unused)
{
	fprintf(stderr, "Warning: Scheduler unimplemented!\n");
	(void)unused;
}

__attribute__((weak, alias("empty"))) void rr();
__attribute__((weak, alias("empty"))) void fcfs();
__attribute__((weak, alias("empty"))) void spn();
__attribute__((weak, alias("empty"))) void srt();
__attribute__((weak, alias("empty"))) void hrrn();

/* Enqueue process with process index/pid */
static void enqueue(struct process *const HEAD, unsigned pid)
{
	struct process *const new = malloc(sizeof(*new));
	if (!new) {
		perror("malloc");
		fprintf(stderr, "Fatal error, bailing\n");
		exit(1);
	}
	new->pid = pid;

	new->cycles_done = new->cycles_waited = 0;
	new->cycles_todo = processes[pid].rtime;
	new->state = PS_READY;

	new->prev = HEAD->prev;
	new->next = HEAD;
	HEAD->prev->next = new;
	HEAD->prev = new;
}

static bool run_and_print_queue(struct process *const HEAD)
{
	static const char *state_name[] = {
		[PS_DEAD] = "-",
		[PS_RUNNING] = "RUNNING",
		[PS_READY] = "ready",
	};

	size_t nrunning = 0;
	size_t nready = 0;

	assert(HEAD->state == PS_DEAD);
	for (struct process *current = HEAD->next; current != HEAD;
	     current = current->next) {
		/* Simulate tick */
		switch (current->state) {
		case PS_RUNNING:
			nrunning++;
			assert(current->cycles_done + current->cycles_todo <=
			       processes[current->pid].rtime);
			current->cycles_done++;
			assert(current->cycles_todo > 0);
			current->cycles_todo--;
			break;
		case PS_READY:
			nready++;
			assert(current->cycles_todo > 0);
			current->cycles_waited++;
			break;
		case PS_DEAD:
			assert(current->cycles_todo == 0);
			assert(current->cycles_done ==
			       processes[current->pid].rtime);
			/* nothing */
			break;
		}

		printf("%u (R: %u/%u, W: %u): %8s\t", current->pid,
		       current->cycles_done,
		       current->cycles_done + current->cycles_todo,
		       current->cycles_waited, state_name[current->state]);
	}

	/* You shall not have any other processes beside me */
	assert(nrunning <= 1);

	if (nrunning == 0 && nready > 0) {
		fprintf(stderr, "Warning: No RUNNING process selected, despite "
				"ready processes being available!\n");
	}

	return nrunning == 0;
}

int main(void)
{
	/* doubly-linked circular list containing processes */
	static struct process _head = {
		.next = &_head,
		.prev = &_head,
		.state = PS_DEAD,
	};
	static struct process *const HEAD = &_head;

	printf("PID (R: done/(done+todo), W: waited): STATE\n\n");

	for (size_t ai = 0; ai < NUM_ALGOS; ai++) {
		printf("Simulation for %s:\n", algos[ai].name);

		for (size_t tick = 0;; tick++) {
			// insert new process if arrival time has come
			for (size_t pi = 0; pi < NUM_PROCESSES; pi++) {
				if (processes[pi].atime == tick) {
					enqueue(HEAD, pi);
				}
			}

			/* actually select new RUNNING process */
			algos[ai].func(HEAD);

			/* run processes and print stats */
			printf("Tick %2zu: ", tick);
			bool empty_run_queue = run_and_print_queue(HEAD);
			printf("\n");

			if (empty_run_queue) { break; }
		}

		/* user prompt */
		if (ai != NUM_ALGOS - 1) {
			printf("\n"
			       "Press any key for next algorithm.\n"
			       "\n");
			getchar();
		}

		/*
                 * Free queue.
                 * Different cases:
                 * 1. [ HEAD ]: current = HEAD, current->prev = HEAD.
                 * 2. [ HEAD a ]: current = HEAD, current->prev = a.
                 * 3. [ HEAD a b] and by induction [ HEAD a b ... z ]
                 */
		assert(HEAD->next && HEAD->next->next &&
		       HEAD->next->next->prev);
		for (const struct process *current = HEAD->next->next;
		     current->prev != HEAD; current = current->next) {
			assert(current->next);
			assert(current->prev);
			free(current->prev);
		}
		HEAD->next = HEAD->prev = HEAD;
	}

	return 0;
}
