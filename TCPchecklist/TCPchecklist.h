void TCPchecklist_init();

void TCPchecklist_finit();

long int TCPchecklist_create(long int nelem);

void TCPchecklist_destroy(long int id);

long int TCPchecklist_get_largest_consecutive_ACK_id(long int id);

void TCPchecklist_add(long int id, long int element);


