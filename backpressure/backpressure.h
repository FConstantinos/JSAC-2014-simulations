void backpressure_init();

void backpressure_finit();

void backpressure_1(long int sim_time, int service_rates[2], double probs[2], long int *num_arrivals[2], gboolean *overheard[2], long int total_arrivals[2], long int *max_ID_ack[2], long int thres, gboolean *stable);
