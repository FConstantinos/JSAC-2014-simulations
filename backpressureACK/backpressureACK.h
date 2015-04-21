void backpressureACK_init();

void backpressureACK_finit();

void backpressureACK_1(long int sim_time, int service_rates[2], long int *num_arrivals[2], gboolean *overheard[2], long int total_arrivals[2], long int *max_ID_ack[2], long int thres, gboolean *stable);
