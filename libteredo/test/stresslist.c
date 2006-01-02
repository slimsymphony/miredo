/*
 * stresslist.c - Libteredo peer list stress tests
 * $Id$
 */

/***********************************************************************
 *  Copyright (C) 2005-2006 Remi Denis-Courmont.                       *
 *  This program is free software; you can redistribute and/or modify  *
 *  it under the terms of the GNU General Public License as published  *
 *  by the Free Software Foundation; version 2 of the license.         *
 *                                                                     *
 *  This program is distributed in the hope that it will be useful,    *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of     *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.               *
 *  See the GNU General Public License for more details.               *
 *                                                                     *
 *  You should have received a copy of the GNU General Public License  *
 *  along with this program; if not, you can get it from:              *
 *  http://www.gnu.org/copyleft/gpl.html                               *
 ***********************************************************************/

#if HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdbool.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <signal.h>

#if HAVE_STDINT_H
# include <stdint.h> /* Mac OS X needs that */
#endif
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>

#include "teredo.h"
#include "peerlist.h"

static void make_address (struct in6_addr *addr)
{
	unsigned i;

	for (i = 0; i < 16; i += sizeof (int))
		*((int *)(addr->s6_addr + i)) = rand ();
}


static volatile int count = 0;

static void alarm_handler (int sig)
{
	count++;
	signal (sig, alarm_handler);
	alarm (1);
	write (2, ".", 1);
}

#define DELAY 10

int main (void)
{
	teredo_peerlist *l;
	struct in6_addr addr = { };
	unsigned long i, j;
	time_t seed;
	clock_t t;

	time (&seed);

	l = teredo_list_create (UINT_MAX, 1000000);
	if (l == NULL)
		return -1;

	// Insertion stress test
	srand ((unsigned int)seed);
	signal (SIGALRM, alarm_handler);
	alarm (1);
	for (i = 0; count < DELAY; i++)
	{
		teredo_peer *p;
		bool create;

		make_address (&addr);
		p = teredo_list_lookup (l, seed, &addr, &create);
		if ((!create) || (p == NULL))
			return -1;
		teredo_list_release (l);
	}

	printf ("\n%lu insertions per second\n", i / DELAY);

	// Lookup stress test
	srand ((unsigned int)seed);
	seed += 10;
	t = clock ();
	for (j = 0; j < i; j++)
	{
		teredo_peer *p;

		make_address (&addr);
		p = teredo_list_lookup (l, seed, &addr, NULL);
		if (p == NULL)
			return -1;
		teredo_list_release (l);
	}
	t = clock () - t;

	printf ("\n%lu lookups per second\n",
	        (unsigned long)((float)j * CLOCKS_PER_SEC / t));

	teredo_list_destroy (l);
	return 0;
}
