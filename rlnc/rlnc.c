#include<stdlib.h>
#include<stdio.h>
#include<math.h>
#include<glib.h>
#include<gsl/gsl_rng.h>
#include<gsl/gsl_randist.h>
#include"../TCPchecklist/TCPchecklist.h"
#include"rlnc.h"

#define FLOW_1 0
#define FLOW_2 1

static gsl_rng *rng;
static const gsl_rng_type * T;
static GQueue *queues[2];
static long int TCPchecklist[2];

static void evac(GQueue *queue, long int TCPchecklist_id, int numel){
	long int i,id;

	for (i=0;i<numel;i++){ 
		id = GPOINTER_TO_INT(g_queue_pop_head(queue)); 
		if (id>0) {TCPchecklist_add(TCPchecklist_id,id); }
		else { TCPchecklist_add(TCPchecklist_id,-id); }
	}
}

static void print_queues(){
	int i;
	int q[2];

	/* Rename variables for easier reference. */
	q[FLOW_1] = g_queue_get_length(queues[FLOW_1]); q[FLOW_2] = g_queue_get_length(queues[FLOW_2]);

	printf("****** flow1 queue ********\n");
	for (i=0;i<q[0];i++){
		printf("%d\t",GPOINTER_TO_INT(g_queue_peek_nth(queues[FLOW_1],i)));
	}
	putchar('\n');

	printf("****** flow2 good queue ********\n");
	for (i=0;i<q[1];i++){
		printf("%d\t",GPOINTER_TO_INT(g_queue_peek_nth(queues[FLOW_2],i)));
	}
	putchar('\n');

}

void rlnc_init(){

	/* Initialize random number generator. */
	gsl_rng_env_setup();
	T = gsl_rng_default;
	rng = gsl_rng_alloc(T);

	/* Create queues. */
	queues[FLOW_1] = g_queue_new();
	queues[FLOW_2] = g_queue_new();


	/* Initialize queues. */
	g_queue_init(queues[FLOW_1]);
	g_queue_init(queues[FLOW_2]);
}

void rlnc_finit(){

	/* Free random number generator. */
	gsl_rng_free(rng);

	/* Free queues. */
	g_queue_free(queues[FLOW_1]);
	g_queue_free(queues[FLOW_2]);
}

void rlnc_1(long int sim_time, int service_rates[2], long int *num_arrivals[2], gboolean *overheard[2], long int total_arrivals[2], long int *max_ID_ack[2], long int thres, gboolean *stable, long int gen_size){

	long int i,j,k,l;
	int c[2];
	long int lastID[2] = {0,0};
	long int g[2],q[2],p[2],r[2];
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
	counter = 0;
	for (i = 0; i<sim_time; i++){
		 
		/* Get new arrivals */
		for (k = 0; k<2; k++){ /* flow loop */
			for (j = 0; j<num_arrivals[k][i]; j++){ /* arrival loop */
				lastID[k]++;
				ID = lastID[k];

				/* If it is a good packet, enqueue ID, else enqueue -ID. */
				if (overheard[k][ID-1]){
					g_queue_push_tail(queues[k],GINT_TO_POINTER(ID));
				}
				else {
					g_queue_push_tail(queues[k],GINT_TO_POINTER(-ID));
				}
			}
		}
		if(debug){
		printf("slot %ld\n",i);
		printf("after arrivals\n");
		print_queues();
		printf("************************************************\n");}

		/* Rename variables for easier reference. */
		q[FLOW_1] = g_queue_get_length(queues[FLOW_1]); q[FLOW_2] = g_queue_get_length(queues[FLOW_2]);

		/* Take controls */
		if( (q[FLOW_1] > gen_size || q[FLOW_2] > gen_size) && counter <=0) {
			
			g[FLOW_1]=0; g[FLOW_2]=0;

			/* Count the good packets of flow 1. */
			for (k = 0; k < q[FLOW_1]; k++){
				(GPOINTER_TO_INT(g_queue_peek_nth(queues[FLOW_1],k))>0) ? (g[FLOW_1]++) : 1 ;
			}

			/* Count the good packets of flow 2. */
			for (k = 0; k < q[FLOW_2]; k++){
				(GPOINTER_TO_INT(g_queue_peek_nth(queues[FLOW_2],k))>0) ? (g[FLOW_2]++) : 1 ;
			}

			/* Determine the number of slots needed for evacuation. */
			counter = ceil ((double) (q[FLOW_1]+q[FLOW_2] - MIN(g[FLOW_1],g[FLOW_2])) / r[SLOW] );

			/* Save a pointer for future reference to evacuate the packets. */
			p[0] = q[FLOW_1]; p[1] = q[FLOW_2];
			
		}
		counter--;

		/* If the counter is zero, then transmissions have ended and it's time to evacuate packets */
		if (counter == 0 ){
			evac(queues[FLOW_1],TCPchecklist[FLOW_1],p[0]+1);
			evac(queues[FLOW_2],TCPchecklist[FLOW_2],p[1]+1);
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
	g_queue_clear(queues[FLOW_1]);
	g_queue_clear(queues[FLOW_2]);
}
