#include <stdint.h>
#include <fstream>

extern "C"{
    #include "../../neat/neat_internal.h"
}


#define max_optc 50

#define MAX_NEAT_FLOWS 100

// Neat options string and parameters
struct neatParserParams{
	int optc;
	char* neatOpt[max_optc];
    char* addr;
	intptr_t flowid;
    int log;
};


inline char* get_flow_protocol(neat_flow* flow){
	
	char* proto; 
    
	switch(flow->socket->stack){
	case NEAT_STACK_UDP:
        proto = "NEAT -> UDP";
        break;
    case NEAT_STACK_TCP:
        proto = "NEAT -> TCP";
        break;
    case NEAT_STACK_SCTP:
        proto = "NEAT -> SCTP";
        break;
    case NEAT_STACK_SCTP_UDP:
        proto = "NEAT -> SCTP/UDP";
        break;
    case NEAT_STACK_UDPLITE:
        proto = "NEAT -> UDPLite";
        break;
    default:
        proto = "?";
        break;
	}

	return proto;
}


