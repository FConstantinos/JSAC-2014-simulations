#include<stdlib.h>
#include<stdio.h>
#include<math.h>
#include<glib.h>
#include<gsl/gsl_rng.h>
#include<gsl/gsl_randist.h>
#include"../TCPchecklist/TCPchecklist.h"
#include"detalgo.h"

#define FLOW_1 0
#define FLOW_2 1
#define BAD 0
#define GOOD 1

static gsl_rng *rng;
static const gsl_rng_type * T;
static GQueue *queues[2][2];
static long int TCPchecklist[2];

static void evac (GQueue *queue, long int TCPchecklist_id, int numel){
	long int i,id;

	for (i=0;i<numel;i++){ 
		id = GPOINTER_TO_INT(g_queue_pop_head(queue)); 
		TCPchecklist_add(TCPchecklist_id,id); 
	}
}

static void print_queues(){
	int i;
	int g[2],b[2];

	/* Rename variables for easier reference. */
	g[0] = g_queue_get_length(queues[0][GOOD]); g[1] = g_queue_get_length(queues[1][GOOD]);
	b[0] = g_queue_get_length(queues[0][BAD]); b[1] = g_queue_get_length(queues[1][BAD]);

	printf("****** flow1 good queue ********\n");
	for (i=0;i<g[0];i++){
		printf("%d\t",GPOINTER_TO_INT(g_queue_peek_nth(queues[FLOW_1][GOOD],i)));
	}
	putchar('\n');

	printf("****** flow1 bad queue ********\n");
	for (i=0;i<b[0];i++){
		printf("%d\t",GPOINTER_TO_INT(g_queue_peek_nth(queues[FLOW_1][BAD],i)));
	}
	putchar('\n');

	printf("****** flow2 good queue ********\n");
	for (i=0;i<g[1];i++){
		printf("%d\t",GPOINTER_TO_INT(g_queue_peek_nth(queues[FLOW_2][GOOD],i)));
	}
	putchar('\n');

	printf("****** flow2 bad queue ********\n");
	for (i=0;i<b[1];i++){
		printf("%d\t",GPOINTER_TO_INT(g_queue_peek_nth(queues[FLOW_2][BAD],i)));
	}
	putchar('\n');
	
}

void detalgo_init(){

	/* Initialize random number generator. */
	gsl_rng_env_setup();
	T = gsl_rng_default;
	rng = gsl_rng_alloc(T);

	/* Create queues. */
	queues[FLOW_1][GOOD] = g_queue_new();
	queues[FLOW_1][BAD] = g_queue_new();
	queues[FLOW_2][GOOD] = g_queue_new();
	queues[FLOW_2][BAD] = g_queue_new();

	/* Initialize queues. */
	g_queue_init(queues[FLOW_1][GOOD]);
	g_queue_init(queues[FLOW_1][BAD]);
	g_queue_init(queues[FLOW_2][GOOD]);
	g_queue_init(queues[FLOW_2][BAD]);
}

void detalgo_finit(){

	/* Free random number generator. */
	gsl_rng_free(rng);

	/* Free queues. */
	g_queue_free(queues[FLOW_1][GOOD]);
	g_queue_free(queues[FLOW_1][BAD]);
	g_queue_free(queues[FLOW_2][GOOD]);
	g_queue_free(queues[FLOW_2][BAD]);
}

void detalgo_1(long int sim_time, int service_rates[2], long int *num_arrivals[2], gboolean *overheard[2], long int total_arrivals[2], long int *max_ID_ack[2], long int *backlog[2], long int thres, gboolean *stable){

	long int i,j,k;
	int c[2];
	long int lastID[2] = {0,0};
	long int g[2],b[2],r[2];
	int FAST,SLOW;
	long int ID;
	int debug = 0;
	int mg,Mg;
	int counter;
	int flow;

	/* Initializations */
	r[0] = service_rates[0];  r[1] = service_rates[1];
	FAST = service_rates[FLOW_1] <= service_rates[FLOW_2]?FLOW_2:FLOW_1;
	SLOW = fmod(FAST+1,2);
	if (stable!=NULL) {*stable = TRUE; }

	/* Initialize TCPchecklist library. */
	TCPchecklist_init();

	/* Create TCP checklists */
	TCPchecklist[FLOW_1] = TCPchecklist_create(total_arrivals[FLOW_1]);
	TCPchecklist[FLOW_2] = TCPchecklist_create(total_arrivals[FLOW_2]);

	/* Begin simulation */
	for (i = 0; i<sim_time; i++){
		 
		/* Get new arrivals */
		for (k = 0; k<2; k++){ /* flow loop */
			for (j = 0; j<num_arrivals[k][i]; j++){ /* arrival loop */
				lastID[k]++;
				ID = lastID[k];
				if (overheard[k][ID-1]){
					g_queue_push_tail(queues[k][GOOD],GINT_TO_POINTER(ID));
				}
				else {
					g_queue_push_tail(queues[k][BAD],GINT_TO_POINTER(ID));
				}
			}
		}
		if(debug){
		printf("slot %ld\n",i);
		printf("after arrivals\n");
		print_queues();
		printf("************************************************\n");

		}

		/* Rename variables for easier reference. */
		g[0] = g_queue_get_length(queues[0][GOOD]); g[1] = g_queue_get_length(queues[1][GOOD]);
		b[0] = g_queue_get_length(queues[0][BAD]); b[1] = g_queue_get_length(queues[1][BAD]);


		/* Take controls */
		if (g[FAST] >= r[SLOW] && g[SLOW] >= r[SLOW]){
			evac( queues[FAST][GOOD],TCPchecklist[FAST],r[SLOW]);
			evac( queues[SLOW][GOOD],TCPchecklist[SLOW],r[SLOW]);
		}

		else if (b[FAST] >= r[FAST] && b[SLOW] >= r[SLOW]){
			k = (GPOINTER_TO_INT(g_queue_peek_head(queues[FAST][BAD])) < GPOINTER_TO_INT(g_queue_peek_head(queues[SLOW][BAD]))) ? FAST : SLOW;
			evac( queues[k][BAD],TCPchecklist[k],r[k]);		
		}

		else if (b[SLOW] >= r[SLOW]){
			evac( queues[SLOW][BAD],TCPchecklist[SLOW],r[SLOW]);		
		}

		else if (b[FAST] >= r[FAST]){
			evac( queues[FAST][BAD],TCPchecklist[FAST],r[FAST]);		
		}

		else if (b[SLOW] >= r[SLOW]){
			evac( queues[SLOW][BAD],TCPchecklist[SLOW],r[SLOW]);		
		}

		else if (g[SLOW] >= r[SLOW]){
			evac( queues[SLOW][GOOD],TCPchecklist[SLOW],r[SLOW]);		
		}
		else if (g[FAST] >= r[FAST]){
			evac( queues[FAST][GOOD],TCPchecklist[FAST],r[FAST]);		
		}

		/* anti-idleness controls */
		else{

			/* Determine what flow can send the most packets. */
			mg = MIN(g[SLOW],g[FAST]);
			Mg = MAX(g[SLOW],g[FAST]);

			counter=0;
			if ( MIN(r[SLOW],g[SLOW]+g[FAST] + b[SLOW] + b[FAST] ) >= MIN(r[FAST],g[FAST]+b[FAST]) ) {
				while(counter<r[SLOW]){
					g[0] = g_queue_get_length(queues[0][GOOD]); g[1] = g_queue_get_length(queues[1][GOOD]);
					b[0] = g_queue_get_length(queues[0][BAD]); b[1] = g_queue_get_length(queues[1][BAD]);

					/* Evacuate the g+g. */
					if (g[FLOW_1] && g[FLOW_2]){
						evac( queues[FAST][GOOD],TCPchecklist[FAST],1);
						evac( queues[SLOW][GOOD],TCPchecklist[SLOW],1);
						counter++;
						continue;
					}

					/* Evacuate any bad packets. */
					if (b[FAST] && b[SLOW] ){

						/* Choose the queue with the largest backlog. */
						flow = (MAX(b[FAST],b[SLOW]) == b[SLOW])?SLOW:FAST;

						evac( queues[flow][BAD],TCPchecklist[flow],1);	
						counter++;
						continue;
					}

					if (b[SLOW]){
						evac( queues[SLOW][BAD],TCPchecklist[SLOW],1);		
						counter++;
						continue;
					}

					if (b[FAST]){	
						evac( queues[FAST][BAD],TCPchecklist[FAST],1);
						counter++;
						continue;
					}

					/* g controls */
					if (g[SLOW]){
						evac( queues[SLOW][GOOD],TCPchecklist[SLOW],1);
						counter++;
						continue;	
					}
					if (g[FAST]){
						evac( queues[FAST][GOOD],TCPchecklist[FAST],1);		
						counter++;
						continue;
					}
					break;
				}
			}
			else {
				while(counter<r[FAST]){
					g[FAST] = g_queue_get_length(queues[FAST][GOOD]);
					b[FAST] = g_queue_get_length(queues[FAST][BAD]);
	
					/* First evacuate the fast bad packets. */
					if( b[FAST]){
						evac(queues[FAST][BAD],TCPchecklist[FAST],1);
						counter++;
						continue;
					}


					/* Then evacuate the fast good packets. */
					if(g[FAST]){
						evac(queues[FAST][GOOD],TCPchecklist[FAST],1);
						counter++;
						continue;
					}
					break;
				}
			}
		
		}
		if(debug){
		printf("after control\n");
		print_queues();
		printf("************************************************\n");}

		/* Get greatest ACKed ID for this slot. */
		max_ID_ack[FAST][i] = TCPchecklist_get_largest_consecutive_ACK_id(TCPchecklist[FAST]);
		max_ID_ack[SLOW][i] = TCPchecklist_get_largest_consecutive_ACK_id(TCPchecklist[SLOW]);

		/* Check if system is stable. Otherwise stop*/
		if (stable!=NULL && (lastID[FLOW_1]-max_ID_ack[FLOW_1][i] > thres || lastID[FLOW_2]-max_ID_ack[FLOW_2][i] > thres) ){
			*stable = FALSE;
			break;
		}

		/* Rename variables for easier reference. */
		g[0] = g_queue_get_length(queues[0][GOOD]); g[1] = g_queue_get_length(queues[1][GOOD]);
		b[0] = g_queue_get_length(queues[0][BAD]); b[1] = g_queue_get_length(queues[1][BAD]);

		/* Compute current backlog. */
		backlog[FLOW_1][i] = g[FLOW_1]+ b[FLOW_1];
		backlog[FLOW_2][i] = g[FLOW_2]+ b[FLOW_2];

		if(debug){
		printf("checklists largest ACK\n");
		printf("flow 1: %ld\nflow 2: %ld\n",max_ID_ack[FLOW_1][i],max_ID_ack[FLOW_2][i]);
		printf("************************************************\n");	
		printf("give values\n"); 
		scanf(" %ld ",&k);}
	}

	/* Finalize TCPchecklist. */ 
	TCPchecklist_finit(); 

	/* Evacuate the queue remnants. */
	g_queue_clear(queues[FLOW_1][GOOD]);
	g_queue_clear(queues[FLOW_1][BAD]);
	g_queue_clear(queues[FLOW_2][GOOD]);
	g_queue_clear(queues[FLOW_2][BAD]);
}
