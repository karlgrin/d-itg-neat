#include <pthread.h>
#include "../common/neat.h"



void* neat_client_start(void* param);

void neat_send(unsigned char* payload, int size, int itgFlowId);

void neat_stop_event_loop(int itgFlowId);

neat_flow* get_flow(int itgFlowId);

int& get_nt_ret(int itgFlowId);

pthread_cond_t& get_nt_cv(int itgFlowId);

pthread_mutex_t& get_nt_mp(int itgFlowId);

