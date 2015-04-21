#include<stdlib.h>
#include<stdio.h>
#include<math.h>
#include<glib.h>
#include<gsl/gsl_rng.h>
#include<gsl/gsl_randist.h>
#include"../TCPchecklist/TCPchecklist.h"
#include"stochalgo.h"

#define FLOW_1 0
#define FLOW_2 1
#define BAD 0
#define GOOD 1
#define UNKNOWN 2

static gsl_rng *rng;
static const gsl_rng_type * T;
static GQueue *queues[2][3];
static long int TCPchecklist[2];

static void evac (GQueue *queue, long int TCPchecklist_id, int numel){
	long int i,id;

	for (i=0;i<numel;i++){ 
		id = GPOINTER_TO_INT(g_queue_pop_head(queue)); 
		if (id>0) {TCPchecklist_add(TCPchecklist_id,id); }
		else { TCPchecklist_add(TCPchecklist_id,-id); }
	}
}

static void print_queues(){
	int i;
	int g[2],b[2],u[2];

	/* Rename variables for easier reference. */
	g[0] = g_queue_get_length(queues[0][GOOD]); g[1] = g_queue_get_length(queues[1][GOOD]);
	b[0] = g_queue_get_length(queues[0][BAD]); b[1] = g_queue_get_length(queues[1][BAD]);
	u[0] = g_queue_get_length(queues[0][UNKNOWN]); u[1] = g_queue_get_length(queues[1][UNKNOWN]);

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

	printf("****** flow1 unknown queue ********\n");
	for (i=0;i<u[0];i++){
		printf("%d\t",GPOINTER_TO_INT(g_queue_peek_nth(queues[FLOW_1][UNKNOWN],i)));
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

	printf("****** flow2 unknown queue ********\n");
	for (i=0;i<u[1];i++){
		printf("%d\t",GPOINTER_TO_INT(g_queue_peek_nth(queues[FLOW_2][UNKNOWN],i)));
	}
	putchar('\n');
	
}

void stochalgo_init(){

	/* Initialize random number generator. */
	gsl_rng_env_setup();
	T = gsl_rng_default;
	rng = gsl_rng_alloc(T);

	/* Create queues. */
	queues[FLOW_1][GOOD] = g_queue_new();
	queues[FLOW_1][BAD] = g_queue_new();
	queues[FLOW_1][UNKNOWN] = g_queue_new();
	queues[FLOW_2][GOOD] = g_queue_new();
	queues[FLOW_2][BAD] = g_queue_new();
	queues[FLOW_2][UNKNOWN] = g_queue_new();


	/* Initialize queues. */
	g_queue_init(queues[FLOW_1][GOOD]);
	g_queue_init(queues[FLOW_1][BAD]);
	g_queue_init(queues[FLOW_1][UNKNOWN]);
	g_queue_init(queues[FLOW_2][GOOD]);
	g_queue_init(queues[FLOW_2][BAD]);
	g_queue_init(queues[FLOW_2][UNKNOWN]);
}

void stochalgo_finit(){

	/* Free random number generator. */
	gsl_rng_free(rng);

	/* Free queues. */
	g_queue_free(queues[FLOW_1][GOOD]);
	g_queue_free(queues[FLOW_1][BAD]);
	g_queue_free(queues[FLOW_1][UNKNOWN]);
	g_queue_free(queues[FLOW_2][GOOD]);
	g_queue_free(queues[FLOW_2][BAD]);
	g_queue_free(queues[FLOW_2][UNKNOWN]);
}

void stochalgo_1(long int sim_time, int service_rates[2], long int *num_arrivals[2], gboolean *overheard[2], long int total_arrivals[2], long int *max_ID_ack[2], long int *backlog[2], long int thres, gboolean *stable){

	long int i,j,k,l;
	int c[2];
	long int lastID[2] = {0,0};
	long int g[2],b[2],r[2],u[2];
	int FAST,SLOW;
	long int ID;
	int debug = 0;
	int mg,Mg,mu,Mu,mgu;
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

				/* If it is a good packet, enqueue ID, else enqueue -ID. */
				if (overheard[k][ID-1]){
					g_queue_push_tail(queues[k][UNKNOWN],GINT_TO_POINTER(ID));
				}
				else {
					g_queue_push_tail(queues[k][UNKNOWN],GINT_TO_POINTER(-ID));
				}
			}
		}
		if(debug){
		printf("slot %ld\n",i);
		printf("after arrivals\n");
		print_queues();
		printf("************************************************\n");}

		/* Rename variables for easier reference. */
		g[0] = g_queue_get_length(queues[0][GOOD]); g[1] = g_queue_get_length(queues[1][GOOD]);
		b[0] = g_queue_get_length(queues[0][BAD]); b[1] = g_queue_get_length(queues[1][BAD]);
		u[0] = g_queue_get_length(queues[0][UNKNOWN]); u[1] = g_queue_get_length(queues[1][UNKNOWN]);

		/* Take controls */

		/* g + g controls */
		if (g[FAST] >= r[SLOW] && g[SLOW] >= r[SLOW]){
			evac( queues[FAST][GOOD],TCPchecklist[FAST],r[SLOW]);
			evac( queues[SLOW][GOOD],TCPchecklist[SLOW],r[SLOW]);
		}


		/* b controls */
		else if (b[SLOW] >= r[SLOW]){
			evac( queues[SLOW][BAD],TCPchecklist[SLOW],r[SLOW]);		
			evac( queues[FAST][BAD],TCPchecklist[FAST],r[SLOW]);
		}
		/* g + u controls */
		else if ( (u[SLOW] >= r[SLOW] && g[FAST] >= r[SLOW] ) && ( u[FAST] >= r[SLOW] && g[SLOW] >= r[SLOW] ) ){
			/* Make random choice */
			c[0] = SLOW; c[1] = FAST;
			gsl_ran_sample(rng,&k,1,c,2,sizeof(int));
			l = fmod(k+1,2);		

			for (j=0;j<r[SLOW];j++){
				if(GPOINTER_TO_INT(g_queue_peek_head( queues[k][UNKNOWN])) >0){
					evac( queues[k][UNKNOWN],TCPchecklist[k],1);
					evac( queues[l][GOOD],TCPchecklist[l],1);
				}
				else{
					evac( queues[k][UNKNOWN],TCPchecklist[k],1);
				}
			}
		}
		else if (u[SLOW] >= r[SLOW] && g[FAST] >= r[SLOW] ) {

			k = SLOW;
			l = FAST;
			for (j=0;j<r[SLOW];j++){
				if(GPOINTER_TO_INT(g_queue_peek_head( queues[k][UNKNOWN])) >0){
					evac( queues[k][UNKNOWN],TCPchecklist[k],1);
					evac( queues[l][GOOD],TCPchecklist[l],1);
				}
				else{
					evac( queues[k][UNKNOWN],TCPchecklist[k],1);
				}
			}
		}

		else if (u[FAST] >= r[SLOW] && g[SLOW] >= r[SLOW] ){
			k = FAST;
			l = SLOW;
			for (j=0;j<r[SLOW];j++){
				if(GPOINTER_TO_INT(g_queue_peek_head( queues[k][UNKNOWN])) >0){
					evac( queues[k][UNKNOWN],TCPchecklist[k],1);
					evac( queues[l][GOOD],TCPchecklist[l],1);
				}
				else{
					evac( queues[k][UNKNOWN],TCPchecklist[k],1);
				}
			}
		}

		/* u + u controls */
		else if (u[FAST] >= r[SLOW] && u[SLOW] >= r[SLOW] ){

			for(j=0;j<r[SLOW];j++){
				k = GPOINTER_TO_INT(g_queue_peek_head( queues[FAST][UNKNOWN] ));
				l = GPOINTER_TO_INT(g_queue_peek_head( queues[SLOW][UNKNOWN] ));
				if ( k > 0 && l > 0 ){
					evac( queues[FAST][UNKNOWN],TCPchecklist[FAST],1);
					evac( queues[SLOW][UNKNOWN],TCPchecklist[SLOW],1);	
				}
				else if ( k > 0 && l < 0 ){
					evac( queues[SLOW][UNKNOWN],TCPchecklist[SLOW],1);
					g_queue_push_tail(queues[FAST][GOOD],GINT_TO_POINTER(g_queue_pop_head(queues[FAST][UNKNOWN])) );
				}
				else if ( k < 0 && l > 0 ){
					evac( queues[FAST][UNKNOWN],TCPchecklist[FAST],1);
					g_queue_push_tail(queues[SLOW][GOOD],GINT_TO_POINTER(g_queue_pop_head(queues[SLOW][UNKNOWN])) );
				}
				else if ( k < 0 && l < 0 ){
					g_queue_push_tail(queues[FAST][BAD],GINT_TO_POINTER(g_queue_pop_head(queues[FAST][UNKNOWN])) );
					g_queue_push_tail(queues[SLOW][BAD],GINT_TO_POINTER(g_queue_pop_head(queues[SLOW][UNKNOWN])) );					
				} 
			}
			
		}

		/* u controls */
		else if (u[FAST] >= r[FAST]){
			evac( queues[FAST][UNKNOWN],TCPchecklist[FAST],r[FAST]);		
		}
		else if (u[SLOW] >= r[SLOW]){
			evac( queues[SLOW][UNKNOWN],TCPchecklist[SLOW],r[SLOW]);		
		}
		
		/* g controls */
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
			if ( MIN(r[SLOW],g[SLOW]+g[FAST] + 2*b[SLOW] + u[FAST] + u[SLOW] ) >= MIN(r[FAST],g[FAST]+u[FAST]) ) {
				while(counter<r[SLOW]){
					g[0] = g_queue_get_length(queues[0][GOOD]); g[1] = g_queue_get_length(queues[1][GOOD]);
					b[0] = g_queue_get_length(queues[0][BAD]); b[1] = g_queue_get_length(queues[1][BAD]);
					u[0] = g_queue_get_length(queues[0][UNKNOWN]); u[1] = g_queue_get_length(queues[1][UNKNOWN]);

					/* Evacuate any bad packets. */
					if (b[SLOW]){
						evac( queues[SLOW][BAD],TCPchecklist[SLOW],1);		
						evac( queues[FAST][BAD],TCPchecklist[FAST],1);
						counter++;
						continue;
					}

					/* Evacuate the g+g. */
					if (g[FLOW_1] && g[FLOW_2]){
						evac( queues[FAST][GOOD],TCPchecklist[FAST],1);
						evac( queues[SLOW][GOOD],TCPchecklist[SLOW],1);
						counter++;
						continue;
					}

					/* g + u controls */
					if (u[SLOW] && g[FAST]  ) {

						k = SLOW;
						l = FAST;
						if(GPOINTER_TO_INT(g_queue_peek_head( queues[k][UNKNOWN])) >0){
							evac( queues[k][UNKNOWN],TCPchecklist[k],1);
							evac( queues[l][GOOD],TCPchecklist[l],1);
						}
						else{
							evac( queues[k][UNKNOWN],TCPchecklist[k],1);
						}
						counter++;
						continue;
					}

					if (u[FAST] && g[SLOW] ){
						k = FAST;
						l = SLOW;
						if(GPOINTER_TO_INT(g_queue_peek_head( queues[k][UNKNOWN])) >0){
							evac( queues[k][UNKNOWN],TCPchecklist[k],1);
							evac( queues[l][GOOD],TCPchecklist[l],1);
						}
						else{
							evac( queues[k][UNKNOWN],TCPchecklist[k],1);
						}
						counter++;
						continue;
					}


					/* u + u controls */
					if (u[FAST] && u[SLOW]){
						k = GPOINTER_TO_INT(g_queue_peek_head( queues[FAST][UNKNOWN] ));
						l = GPOINTER_TO_INT(g_queue_peek_head( queues[SLOW][UNKNOWN] ));

						if ( k > 0 && l > 0 ){
							evac( queues[FAST][UNKNOWN],TCPchecklist[FAST],1);
							evac( queues[SLOW][UNKNOWN],TCPchecklist[SLOW],1);	
						}
						else if ( k > 0 && l < 0 ){
							evac( queues[SLOW][UNKNOWN],TCPchecklist[SLOW],1);
							g_queue_push_tail(queues[FAST][GOOD],GINT_TO_POINTER(g_queue_pop_head(queues[FAST][UNKNOWN])) );
						}
						else if ( k < 0 && l > 0 ){
							evac( queues[FAST][UNKNOWN],TCPchecklist[FAST],1);
							g_queue_push_tail(queues[SLOW][GOOD],GINT_TO_POINTER(g_queue_pop_head(queues[SLOW][UNKNOWN])) );
						}
						else if ( k < 0 && l < 0 ){
							g_queue_push_tail(queues[FAST][BAD],GINT_TO_POINTER(g_queue_pop_head(queues[FAST][UNKNOWN])) );
							g_queue_push_tail(queues[SLOW][BAD],GINT_TO_POINTER(g_queue_pop_head(queues[SLOW][UNKNOWN])) );					
						} 
						counter++;
						continue;		
					}

					/* u controls */
					if (u[FAST]){
						evac( queues[FAST][UNKNOWN],TCPchecklist[FAST],1);
						counter++;
						continue;		
					}
					if (u[SLOW]){
						evac( queues[SLOW][UNKNOWN],TCPchecklist[SLOW],1);
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
					u[FAST] = g_queue_get_length(queues[FAST][UNKNOWN]);
	
					/* First evacuate the fast unknown packets. */
					if( u[FAST]){
						evac(queues[FAST][UNKNOWN],TCPchecklist[FAST],1);
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
		u[0] = g_queue_get_length(queues[0][UNKNOWN]); u[1] = g_queue_get_length(queues[1][UNKNOWN]);

		/* Compute current backlog. */
		backlog[FLOW_1][i] = g[FLOW_1]+ b[FLOW_1] + u[FLOW_1];
		backlog[FLOW_2][i] = g[FLOW_2]+ b[FLOW_2] + u[FLOW_2];

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
	g_queue_clear(queues[FLOW_1][UNKNOWN]);
	g_queue_clear(queues[FLOW_2][GOOD]);
	g_queue_clear(queues[FLOW_2][BAD]);
	g_queue_clear(queues[FLOW_2][UNKNOWN]);
}
