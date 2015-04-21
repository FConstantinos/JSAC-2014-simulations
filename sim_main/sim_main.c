#include<stdio.h>
#include<string.h>
#include<glib.h>
#include<gsl/gsl_rng.h>
#include<gsl/gsl_randist.h>
#include"../detalgo/detalgo.h"
#include"../stochalgo/stochalgo.h"
#include"../rlnc/rlnc.h"
#include"../backpressure/backpressure.h"
#include"../backpressureACK/backpressureACK.h"
#include"../stability_region/stability_region.h"
#include"../delay/delay.h"

#define TOLERANCE .001
#define RLNC_GEN 8

static gsl_rng *rng;
static const gsl_rng_type * T;
static long int sim_time;
static int service_rates[2];
static double probs[2];
static long int *num_arrivals[2]; 
static long int *ID_max[2];
static gboolean *overheard[2]; 
static long int total_arrivals[2];
static long int overheard_size[2];
static long int *max_ID_ack[2];
static long int *backlog[2];
static long int thres;
static double xmax;
static double ymax;
static double ratio;

void arrival_generator(gsl_rng *rng, long int sim_time, double arrival_rate, double prob, long int *num_arrivals, gboolean **overheard, long int *overheard_size, long int *total_arrivals){
	long int i;

	/* Set num_arrivals. */
	*total_arrivals = 0;
	for (i=0;i<sim_time;i++){
		num_arrivals[i] = gsl_ran_poisson(rng,arrival_rate);
		*total_arrivals += num_arrivals[i];
	}
			
	/* If the size of overheard matrix is not enough, malloc it. */
	if (*total_arrivals > *overheard_size){
		g_free(*overheard);
		*overheard = (gboolean *)g_malloc((*total_arrivals)*sizeof(gboolean));
		*overheard_size = *total_arrivals;
	}

	/* Set overheard. */
	for (i=0;i<(*total_arrivals);i++){
		(*overheard)[i]=(gboolean)gsl_ran_bernoulli(rng,prob);
	}
}

gboolean stability_funct_det(double arrival_rates[2]){
	long int i;
	gboolean stable;

	arrival_generator(rng, sim_time, arrival_rates[0], probs[0], num_arrivals[0], &overheard[0], &overheard_size[0], &total_arrivals[0]);
	arrival_generator(rng, sim_time, arrival_rates[1], probs[1], num_arrivals[1], &overheard[1], &overheard_size[1], &total_arrivals[1]);	

	/* Initialize ID_max. */
	ID_max[0][0] = num_arrivals[0][0];	ID_max[1][0] = num_arrivals[1][0];
	for (i=1;i<sim_time;i++){
		ID_max[0][i] = ID_max[0][i-1] + num_arrivals[0][i];
		ID_max[1][i] = ID_max[1][i-1] + num_arrivals[1][i];
	}
	/* Initialize backlog. */
	backlog[0] = (long int*)malloc(sim_time*sizeof(long int)); backlog[1] = (long int*)malloc(sim_time*sizeof(long int));

	/* Run the simulation with the values taken */
	detalgo_1(sim_time, service_rates, num_arrivals, overheard, total_arrivals, max_ID_ack, backlog, thres, &stable);

	return stable;

}

gboolean stability_funct_stoch(double arrival_rates[2]){
	long int i;
	gboolean stable;

	arrival_generator(rng, sim_time, arrival_rates[0], probs[0], num_arrivals[0], &overheard[0], &overheard_size[0], &total_arrivals[0]);
	arrival_generator(rng, sim_time, arrival_rates[1], probs[1], num_arrivals[1], &overheard[1], &overheard_size[1], &total_arrivals[1]);	

	/* Initialize ID_max. */
	ID_max[0][0] = num_arrivals[0][0];	ID_max[1][0] = num_arrivals[1][0];
	for (i=1;i<sim_time;i++){
		ID_max[0][i] = ID_max[0][i-1] + num_arrivals[0][i];
		ID_max[1][i] = ID_max[1][i-1] + num_arrivals[1][i];
	}

	/* Initialize backlog. */
	backlog[0] = (long int*)malloc(sim_time*sizeof(long int)); backlog[1] = (long int*)malloc(sim_time*sizeof(long int));

	/* Run the simulation with the values taken */
	stochalgo_1(sim_time, service_rates, num_arrivals, overheard, total_arrivals, max_ID_ack, backlog, thres,&stable);

	return stable;

}

double delay_funct_det(double coeff){
	long int i;
	double mean,mean_delay[2];

	arrival_generator(rng, sim_time, coeff, probs[0], num_arrivals[0], &overheard[0], &overheard_size[0], &total_arrivals[0]);
	arrival_generator(rng, sim_time, ratio*coeff, probs[1], num_arrivals[1], &overheard[1], &overheard_size[1], &total_arrivals[1]);	

	/* Initialize ID_max. */
	ID_max[0][0] = num_arrivals[0][0];	ID_max[1][0] = num_arrivals[1][0];
	for (i=1;i<sim_time;i++){
		ID_max[0][i] = ID_max[0][i-1] + num_arrivals[0][i];
		ID_max[1][i] = ID_max[1][i-1] + num_arrivals[1][i];
	}

	/* Initialize backlog. */
	backlog[0] = (long int*)malloc(sim_time*sizeof(long int)); backlog[1] = (long int*)malloc(sim_time*sizeof(long int));

	/* Run the simulation with the values taken */
	detalgo_1(sim_time, service_rates, num_arrivals, overheard, total_arrivals, max_ID_ack, backlog, -1, NULL);

	mean_delay[0] = 0;
	mean_delay[1] = 0;
	for(i=0;i<sim_time;i++){
		mean_delay[0]+= ID_max[0][i]-max_ID_ack[0][i];
		mean_delay[1]+= ID_max[1][i]-max_ID_ack[1][i];
	}
	mean_delay[0] = mean_delay[0]/(double)sim_time; 
	mean_delay[1] = mean_delay[1]/(double)sim_time; 
	mean = (mean_delay[0]+mean_delay[1])/2; 
	return mean;

}

double delay_funct_stoch(double coeff){
	long int i;
	double mean,mean_delay[2];

	arrival_generator(rng, sim_time, coeff, probs[0], num_arrivals[0], &overheard[0], &overheard_size[0], &total_arrivals[0]);
	arrival_generator(rng, sim_time, ratio*coeff, probs[1], num_arrivals[1], &overheard[1], &overheard_size[1], &total_arrivals[1]);	

	/* Initialize ID_max. */
	ID_max[0][0] = num_arrivals[0][0];	ID_max[1][0] = num_arrivals[1][0];
	for (i=1;i<sim_time;i++){
		ID_max[0][i] = ID_max[0][i-1] + num_arrivals[0][i];
		ID_max[1][i] = ID_max[1][i-1] + num_arrivals[1][i];
	}

	/* Initialize backlog. */
	backlog[0] = (long int*)malloc(sim_time*sizeof(long int)); backlog[1] = (long int*)malloc(sim_time*sizeof(long int));

	/* Run the simulation with the values taken */
	stochalgo_1(sim_time, service_rates, num_arrivals, overheard, total_arrivals, max_ID_ack, backlog, -1,NULL);

	mean_delay[0] = 0;
	mean_delay[1] = 0;
	for(i=0;i<sim_time;i++){
		mean_delay[0]+= ID_max[0][i]-max_ID_ack[0][i];
		mean_delay[1]+= ID_max[1][i]-max_ID_ack[1][i];
	}
	mean_delay[0] = mean_delay[0]/(double)sim_time;
	mean_delay[1] = mean_delay[1]/(double)sim_time;
	mean = (mean_delay[0]+mean_delay[1])/2;
	return mean;
}

double delay_funct_rlnc(double coeff){
	long int i;
	double mean,mean_delay[2];

	arrival_generator(rng, sim_time, coeff, probs[0], num_arrivals[0], &overheard[0], &overheard_size[0], &total_arrivals[0]);
	arrival_generator(rng, sim_time, ratio*coeff, probs[1], num_arrivals[1], &overheard[1], &overheard_size[1], &total_arrivals[1]);	

	/* Initialize ID_max. */
	ID_max[0][0] = num_arrivals[0][0];	ID_max[1][0] = num_arrivals[1][0];
	for (i=1;i<sim_time;i++){
		ID_max[0][i] = ID_max[0][i-1] + num_arrivals[0][i];
		ID_max[1][i] = ID_max[1][i-1] + num_arrivals[1][i];
	}

	/* Run the simulation with the values taken */
	rlnc_1(sim_time, service_rates, num_arrivals, overheard, total_arrivals, max_ID_ack,-1,NULL,RLNC_GEN);

	mean_delay[0] = 0;
	mean_delay[1] = 0;
	for(i=0;i<sim_time;i++){
		mean_delay[0]+= ID_max[0][i]-max_ID_ack[0][i];
		mean_delay[1]+= ID_max[1][i]-max_ID_ack[1][i];
	}
	mean_delay[0] = mean_delay[0]/(double)sim_time;
	mean_delay[1] = mean_delay[1]/(double)sim_time;
	mean = (mean_delay[0]+mean_delay[1])/2;
	return mean;
}

double delay_funct_backpressure(double coeff){
	long int i;
	double mean,mean_delay[2];

	arrival_generator(rng, sim_time, coeff, probs[0], num_arrivals[0], &overheard[0], &overheard_size[0], &total_arrivals[0]);
	arrival_generator(rng, sim_time, ratio*coeff, probs[1], num_arrivals[1], &overheard[1], &overheard_size[1], &total_arrivals[1]);	

	/* Initialize ID_max. */
	ID_max[0][0] = num_arrivals[0][0];	ID_max[1][0] = num_arrivals[1][0];
	for (i=1;i<sim_time;i++){
		ID_max[0][i] = ID_max[0][i-1] + num_arrivals[0][i];
		ID_max[1][i] = ID_max[1][i-1] + num_arrivals[1][i];
	}

	/* Run the simulation with the values taken */
	backpressure_1(sim_time, service_rates, probs, num_arrivals, overheard, total_arrivals, max_ID_ack,-1,NULL);

	mean_delay[0] = 0;
	mean_delay[1] = 0;
	for(i=0;i<sim_time;i++){
		mean_delay[0]+= ID_max[0][i]-max_ID_ack[0][i];
		mean_delay[1]+= ID_max[1][i]-max_ID_ack[1][i];
	}
	mean_delay[0] = mean_delay[0]/(double)sim_time;
	mean_delay[1] = mean_delay[1]/(double)sim_time;
	mean = (mean_delay[0]+mean_delay[1])/2;
	return mean;
}

double delay_funct_backpressureACK(double coeff){
	long int i;
	double mean,mean_delay[2];

	arrival_generator(rng, sim_time, coeff, probs[0], num_arrivals[0], &overheard[0], &overheard_size[0], &total_arrivals[0]);
	arrival_generator(rng, sim_time, ratio*coeff, probs[1], num_arrivals[1], &overheard[1], &overheard_size[1], &total_arrivals[1]);	

	/* Initialize ID_max. */
	ID_max[0][0] = num_arrivals[0][0];	ID_max[1][0] = num_arrivals[1][0];
	for (i=1;i<sim_time;i++){
		ID_max[0][i] = ID_max[0][i-1] + num_arrivals[0][i];
		ID_max[1][i] = ID_max[1][i-1] + num_arrivals[1][i];
	}

	/* Run the simulation with the values taken */
	backpressureACK_1(sim_time, service_rates, num_arrivals, overheard, total_arrivals, max_ID_ack,-1,NULL);

	mean_delay[0] = 0;
	mean_delay[1] = 0;
	for(i=0;i<sim_time;i++){
		mean_delay[0]+= ID_max[0][i]-max_ID_ack[0][i];
		mean_delay[1]+= ID_max[1][i]-max_ID_ack[1][i];
	}
	mean_delay[0] = mean_delay[0]/(double)sim_time;
	mean_delay[1] = mean_delay[1]/(double)sim_time;
	mean = (mean_delay[0]+mean_delay[1])/2;
	return mean;
}

int main( int argc, char * argv[]){

	/* Initialize random number generator. */
	gsl_rng_env_setup();
	T = gsl_rng_default;
	rng = gsl_rng_alloc(T);
	double arrival_rates[2];
	double **points_det, **points_stoch, **points_rlnc, **points_backpressure, **points_backpressureACK;
	double mean_delay[2];
	long int npoints;
	long int i;
	char str[10];

	/* Initialize algorithms. */
	detalgo_init();
	stochalgo_init();
	rlnc_init();
	backpressure_init();
	backpressureACK_init();

	/* Prepare the input. */
	scanf("%s",str);
	scanf("%ld ",&sim_time);
	scanf("%d %d ",&service_rates[0],&service_rates[1]);
	scanf("%lf %lf ",&probs[0],&probs[1]);
	if(!strcmp(str,"stability")){
		scanf("%ld ",&thres);
		scanf("%ld ",&npoints);
	}
	else if(!strcmp(str,"instance")){ 
		scanf("%lf %lf",&arrival_rates[0],&arrival_rates[1]);
	}
	else if(!strcmp(str,"delay")){ 
		scanf("%ld ",&npoints);
		scanf("%lf ",&xmax);
		scanf("%lf ",&ymax);
		scanf("%lf ",&ratio);
	}
	else { exit(0); }

	/* Initialize arrival generation variables. */
	num_arrivals[0] = (long int *)g_malloc(sim_time*sizeof(long int)); num_arrivals[1] = (long int *)g_malloc(sim_time*sizeof(long int));
	ID_max[0] = (long int *)g_malloc(sim_time*sizeof(long int)); ID_max[1] = (long int *)g_malloc(sim_time*sizeof(long int));
	overheard[0] = (gboolean *)g_malloc(sizeof(gboolean)); overheard[1] = (gboolean *)g_malloc(sizeof(gboolean));
	overheard_size[0] = 1; overheard_size[1] = 1;

	/* Initialize max_ID_ack. */
	max_ID_ack[0] = (long int *)g_malloc(sim_time*sizeof(long int)); 	max_ID_ack[1] = (long int *)g_malloc(sim_time*sizeof(long int));

	GTimer *timer = g_timer_new();
	g_timer_start(timer);

	if (!strcmp(str,"stability")) {
		/* Run simulation for stability */
		points_stoch = stability_region(&stability_funct_stoch,0,service_rates[0],0,service_rates[1],npoints,TOLERANCE);
		points_det = stability_region(&stability_funct_det,0,service_rates[0],0,service_rates[1],npoints,TOLERANCE);

		g_timer_stop(timer);
		printf("elapsed time: %f\n", g_timer_elapsed(timer,NULL));
		g_free(timer);

		/* Print the results. */

		FILE * f;
		f = fopen("plot_stability.out","w");
		fprintf(f,"#arrival rate of flow 1\tarrival rate of flow 2\n");
		for(i=0;i<npoints;i++){
			fprintf(f,"%lf\t%lf\t%lf\t%lf\n", points_det[i][0], points_det[i][1], points_stoch[i][0], points_stoch[i][1]);
		}
		fclose(f);
		system("gnuplot plot_cmd_stability -persist");

		/* Free points. */
		for(i=0;i<npoints;i++){
			free(points_det[i]);
			free(points_stoch[i]);
		}
		free(points_det);
		free(points_stoch);
	}
	else if (!strcmp(str,"delay")) {

		/* Run simulation for delay */ 
		points_stoch = delay(&delay_funct_stoch,0,xmax,ymax,npoints); 
		points_det = delay(&delay_funct_det,0,xmax,ymax,npoints);
		points_rlnc = delay(&delay_funct_rlnc,0,xmax,ymax,npoints);
		points_backpressure = delay(&delay_funct_backpressure,0,xmax,ymax,npoints);
		points_backpressureACK = delay(&delay_funct_backpressureACK,0,xmax,ymax,npoints);

		g_timer_stop(timer);
		printf("elapsed time: %f\n", g_timer_elapsed(timer,NULL));
		g_free(timer);

		/* Print the results. */
		FILE * f;
		f = fopen("plot_delay.out","w");
		fprintf(f,"#load scaling coefficitent\t mean delay \n");
		for(i=0;i<npoints;i++){
			fprintf(f,"%lf\t%lf\t%lf\t%lf\t%lf\t%lf\t%lf\t%lf\t%lf\t%lf\n", points_det[i][0], points_det[i][1], points_stoch[i][0], points_stoch[i][1], points_rlnc[i][0], points_rlnc[i][1], points_backpressure[i][0], points_backpressure[i][1],points_backpressureACK[i][0], points_backpressureACK[i][1]);
		}
		fclose(f);
		system("gnuplot plot_cmd_delay -persist");

		/* Free points. */
		for(i=0;i<npoints;i++){
			free(points_det[i]);
			free(points_stoch[i]);
			free(points_rlnc[i]);
			free(points_backpressure[i]);
			free(points_backpressureACK[i]);
		}
		free(points_det);
		free(points_stoch);
		free(points_rlnc);
		free(points_backpressure);
		free(points_backpressureACK);
	}
	else {
		/* Initialize arrivals. */
		arrival_generator(rng, sim_time, arrival_rates[0], probs[0], num_arrivals[0], &overheard[0], &overheard_size[0], &total_arrivals[0]);
		arrival_generator(rng, sim_time, arrival_rates[1], probs[1], num_arrivals[1], &overheard[1], &overheard_size[1], &total_arrivals[1]);	

		/* Initialize ID_max. */
		ID_max[0][0] = num_arrivals[0][0];	ID_max[1][0] = num_arrivals[1][0];
		for (i=1;i<sim_time;i++){
			ID_max[0][i] = ID_max[0][i-1] + num_arrivals[0][i];
			ID_max[1][i] = ID_max[1][i-1] + num_arrivals[1][i];
		}

		/* Initialize backlog. */
		backlog[0] = (long int*)malloc(sim_time*sizeof(long int)); backlog[1] = (long int*)malloc(sim_time*sizeof(long int));

		/* Run the simulation with the values taken for deterministic algorithm */
		detalgo_1(sim_time, service_rates, num_arrivals, overheard, total_arrivals, max_ID_ack, backlog, -1,NULL);

		g_timer_stop(timer);
		printf("elapsed time: %f\n", g_timer_elapsed(timer,NULL));

		/* Print the results. */
		FILE * f;
		f = fopen("plot_det.out","w");
		fprintf(f,"#timeslot\t max ID in flow 1-max ID ACKed in flow 1\t max ID in flow 2-max ID ACKed in flow 2\n");
		for(i=0;i<sim_time;i++){
			fprintf(f,"%ld\t%ld\t%ld\n",i+1,ID_max[0][i]-max_ID_ack[0][i],ID_max[1][i]-max_ID_ack[1][i]);
		}
		fclose(f);
		system("gnuplot plot_cmd_instance_det -persist");

		f = fopen("plot_det_backlog.out","w");
		fprintf(f,"#timeslot\t backlog in flow 1\t backlog in flow 2\n");
		for(i=0;i<sim_time;i++){
			fprintf(f,"%ld\t%ld\t%ld\n",i+1,backlog[0][i], backlog[1][i]);
		}
		fclose(f);
		system("gnuplot plot_cmd_instance_det_backlog -persist");

		mean_delay[0] = 0;
		mean_delay[1] = 0;
		for(i=0;i<sim_time;i++){
			mean_delay[0]+= ID_max[0][i]-max_ID_ack[0][i];
			mean_delay[1]+= ID_max[1][i]-max_ID_ack[1][i];
		}
		mean_delay[0] = mean_delay[0]/(double)sim_time;
		mean_delay[1] = mean_delay[1]/(double)sim_time;
		printf("deterministic system:\n");
		printf("mean delay of flow 1: %lf\n",mean_delay[0]);
		printf("mean delay of flow 2: %lf\n",mean_delay[1]);


		/* Run the simulation with the values taken for stochastic algorithm */
		g_timer_start(timer);

		stochalgo_1(sim_time, service_rates, num_arrivals, overheard, total_arrivals, max_ID_ack, backlog,-1,NULL);

		g_timer_stop(timer);
		printf("elapsed time: %f\n", g_timer_elapsed(timer,NULL));
		g_free(timer);

		/* Print the results. */
		f = fopen("plot_stoch.out","w");
		fprintf(f,"#timeslot\t max ID in flow 1-max ID ACKed in flow 1\t max ID in flow 2-max ID ACKed in flow 2\n");
		for(i=0;i<sim_time;i++){
			fprintf(f,"%ld\t%ld\t%ld\n",i+1,ID_max[0][i]-max_ID_ack[0][i],ID_max[1][i]-max_ID_ack[1][i]);
		}
		fclose(f);
		system("gnuplot plot_cmd_instance_stoch -persist");

		f = fopen("plot_stoch_backlog.out","w");
		fprintf(f,"#timeslot\t backlog in flow 1\t backlog in flow 2\n");
		for(i=0;i<sim_time;i++){
			fprintf(f,"%ld\t%ld\t%ld\n",i+1,backlog[0][i], backlog[1][i]);
		}
		fclose(f);
		system("gnuplot plot_cmd_instance_stoch_backlog -persist");

		mean_delay[0] = 0;
		mean_delay[1] = 0;
		for(i=0;i<sim_time;i++){
			mean_delay[0]+= ID_max[0][i]-max_ID_ack[0][i];
			mean_delay[1]+= ID_max[1][i]-max_ID_ack[1][i];
		}
		mean_delay[0] = mean_delay[0]/(double)sim_time;
		mean_delay[1] = mean_delay[1]/(double)sim_time;
		printf("stochastic system:\n");
		printf("mean delay of flow 1: %lf\n",mean_delay[0]);
		printf("mean delay of flow 2: %lf\n",mean_delay[1]);
	}

	/* Free arrival generation variables. */
	free(num_arrivals[0]); 	free(num_arrivals[1]);
	free(overheard[0]); 	free(overheard[1]);

	/* Free max_ID. */
	free(ID_max[0]); 	free(ID_max[1]);

	/* Free max_ID_ack. */
	free(max_ID_ack[0]); 	free(max_ID_ack[1]);

	/* Finalize algorithms. */ 
	detalgo_finit(); 
	stochalgo_finit();
	rlnc_finit();
	backpressure_finit();
	backpressureACK_finit();

	/* Free random number generator. */
	gsl_rng_free(rng);

	return 0;	
}


