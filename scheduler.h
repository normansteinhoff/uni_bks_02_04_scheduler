/*
 * Autor: Thomas Tegethoff
 * Idee & Vorlage: Heiko Will
 * Modifiziert: Leonard König
 * 
 * Hinweise:
 * - die Prozessliste ist doppelt verkettet, d.h. jedes Element kennt seinen
 *   Vorgänger und Nachfolger
 * - das erste Element / der Kopf (HEAD) der Prozessliste ist ein Dummy, d.h.
 *   es enthält keine Daten
 * - die Prozessliste ist im Kreis verkettet, d.h. das erste Element (HEAD)
 *   zeigt auf das zweite etc., und das letzte Element zeigt wieder auf das
 *   erste (HEAD)
 * - die Funktionen sollen ausschließlich
 *   1. den state des Prozesses, der als nächstes laufen soll, zu RUNNING ändern
 *   2. den state des Prozesses, der bisher gelaufen ist, zu READY (wenn
 *       cycles_todo ungleich 0) ist, bzw. zu DEAD (wenn cycles_todo gleich 0)
 *       verändern.
 * - Sie sollen keine anderen Werte der Prozesse verändern!
 */

#ifndef SCHEDULER_H
#define SCHEDULER_H

enum process_state {
	PS_DEAD = 0,	// Prozess fertig (tot)
	PS_RUNNING = 1,	// zuletzt gelaufener Prozess
	PS_READY = 2	// Prozess bereit
};

struct process {
#ifndef WRAPPER
	const unsigned pid;
	const unsigned cycles_done;
	const unsigned cycles_waited;
	const unsigned cycles_todo;
	struct process *const next;
	struct process *const prev;
#else
	unsigned pid;
	unsigned cycles_done;	// wie viele Cyclen gelaufen
	unsigned cycles_waited;	// wie viele Cyclen gewartet
	unsigned cycles_todo;	// wie viele Cyclen noch zu laufen
	struct process *next;	// nächster Prozess in der Liste
	struct process *prev;	// vorheriger Prozess in der Liste
#endif

	enum process_state state;
};

/* Round Robin */
void rr(struct process *head);
/* First Come First Serve */
void fcfs(struct process *head);
/* Shortest Process Next */
void spn(struct process *head);
/* Shortest Remaining Time */
void srt(struct process *head);
/* Highest Response Ration Next */
void hrrn(struct process *head);

#endif
