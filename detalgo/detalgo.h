void detalgo_init();

void detalgo_finit();

void detalgo_1(long int sim_time, int service_rates[2], long int *num_arrivals[2], gboolean *overheard[2], long int total_arrivals[2], long int *max_ID_ack[2], long int *backlog[2], long int thres, gboolean *stable);
