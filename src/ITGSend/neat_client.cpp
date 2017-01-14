#include "neat_client.h"
#include "../common/ITG.h"
#include "../common/util.h"


extern "C"{
    #include "../../NEAT/neat.h"
    #include "../../NEAT/neat_internal.h"
    #include <stdlib.h>
    #include <uv.h>
    #include <unistd.h>
}

struct neatFlowDescriptor{
    int nt_ret = pthread_cond_init(&nt_cv, &nt_cattr);
    pthread_cond_t nt_cv = PTHREAD_COND_INITIALIZER;
    pthread_mutex_t nt_mp = PTHREAD_MUTEX_INITIALIZER;
    pthread_condattr_t nt_cattr;
    uint16_t config_timeout = 0;
    char *config_primary_dest_addr = NULL;
    char *config_property = "{\n\
        \"transport\": [\n\
            {\n\
                \"value\": \"SCTP\",\n\
                \"precedence\": 1\n\
            },\n\
            {\n\
                \"value\": \"TCP\",\n\
                \"precedence\": 1\n\
            }\n\
        ]\n\
    }";

    struct std_buffer {
        unsigned char* buffer;
        uint32_t buffer_filled;
    };

    struct std_buffer in_buffer;
    unsigned char* buffer_rcv = NULL;
    unsigned char* buffer_snd = NULL;

    struct neat_ctx* ctx = NULL;
    struct neat_flow* flow = NULL;
    struct neat_flow_operations ops;

    uv_pipe_t pipe1;
    uv_connect_t contet;
    
};

extern int optind;

uint16_t config_log_level = 0;
uint32_t config_snd_buffer_size = 128;
uint32_t config_rcv_buffer_size = 256;


static neatFlowDescriptor nfd[MAX_NEAT_FLOWS];


void pipe_read(uv_stream_t *stream, ssize_t bytes_read, const uv_buf_t *buffer);

void pipe_alloc(uv_handle_t *handle, size_t suggested, uv_buf_t *buf);

static neat_error_code on_all_written(struct neat_flow_operations *opCB);



int& get_nt_ret(int itgFlowId){return nfd[itgFlowId].nt_ret;};

pthread_cond_t& get_nt_cv(int itgFlowId){return nfd[itgFlowId].nt_cv;};

pthread_mutex_t& get_nt_mp(int itgFlowId){return nfd[itgFlowId].nt_mp;};



neat_flow* get_flow(int itgFlowId){ return nfd[itgFlowId].flow; }



static neat_error_code on_error(struct neat_flow_operations* opCB){
    if(config_log_level >=2) fprintf(stderr, "%s()\n", __func__);
    return NEAT_OK;
}


static neat_error_code on_abort(struct neat_flow_operations* opCB){
    if(config_log_level >=2) fprintf(stderr, "%s()\n", __func__);
    return NEAT_OK;
}


static neat_error_code on_network_changed(struct neat_flow_operations* opCB){
    if(config_log_level >=2) fprintf(stderr, "%s()\n", __func__);
    return NEAT_OK;
}


static neat_error_code on_timeout(struct neat_flow_operations* opCB){
    if(config_log_level >=2) fprintf(stderr, "%s()\n", __func__);
    return NEAT_OK;
}


static neat_error_code on_readable(struct neat_flow_operations* opCB){
    if(config_log_level >=2) fprintf(stderr, "%s()\n", __func__);
    return NEAT_OK;
}


static neat_error_code on_writable(struct neat_flow_operations* opCB){
    if(config_log_level >=2) fprintf(stderr, "%s()\n", __func__);

    intptr_t flowId = (intptr_t)opCB->userData;

    if (config_log_level >= 1) fprintf(stderr, "%s - sent %d bytes\n", __func__, nfd[flowId].in_buffer.buffer_filled);

    // stop writing
    nfd[flowId].ops.on_writable = NULL;
    neat_set_operations(nfd[flowId].ctx, nfd[flowId].flow, &nfd[flowId].ops);
    return NEAT_OK;
}


static neat_error_code on_all_written(struct neat_flow_operations* opCB){
    if(config_log_level >=2) fprintf(stderr, "%s()\n", __func__);
    // data sent completely - continue reading
    
    intptr_t flowId = (intptr_t)opCB->userData;

    uv_read_start((uv_stream_t*) &nfd[flowId].pipe1, pipe_alloc, pipe_read);
    return NEAT_OK;
}


static neat_error_code on_connected(struct neat_flow_operations* opCB){
    if(config_log_level >=2) fprintf(stderr, "%s()\n", __func__);

    int rc; 

    intptr_t flowId = (intptr_t)opCB->userData;
    


    uv_pipe_init(nfd[flowId].ctx->loop, &nfd[flowId].pipe1, 0);

    uv_pipe_connect(&nfd[flowId].contet, &nfd[flowId].pipe1, "testname", NULL);


    nfd[flowId].ops.on_readable = on_readable;
    neat_set_operations(nfd[flowId].ctx, nfd[flowId].flow, &nfd[flowId].ops);

    if (nfd[flowId].config_primary_dest_addr != NULL) {
        rc = neat_set_primary_dest(nfd[flowId].ctx, nfd[flowId].flow, nfd[flowId].config_primary_dest_addr);
        if (rc) {
            fprintf(stderr, "Failed to set primary dest. addr.: %u\n", rc);
        } else {
            if (config_log_level > 1)
                fprintf(stderr, "Primary dest. addr. set to: '%s'.\n",
            nfd[flowId].config_primary_dest_addr);
        }
    }

    if (nfd[flowId].config_timeout)
        neat_change_timeout(nfd[flowId].ctx, nfd[flowId].flow, nfd[flowId].config_timeout);
    
    // Signal D-ITG client that we are ready to send.
    nfd[flowId].nt_ret = 1;
    pthread_cond_broadcast(&nfd[flowId].nt_cv);
    pthread_mutex_unlock(&nfd[flowId].nt_mp);

    return NEAT_OK;
}

static neat_error_code on_close(struct neat_flow_operations* opCB){
    if(config_log_level >=2) fprintf(stderr, "%s()\n", __func__);

    return NEAT_OK;
}


void pipe_alloc(uv_handle_t *handle, size_t suggested, uv_buf_t *buffer){
    if(config_log_level >=2) fprintf(stderr, "%s()\n", __func__);

    buffer->len = config_rcv_buffer_size;
    buffer->base = (char*)malloc(config_rcv_buffer_size);
    
}


static void on_pipe_write(uv_write_t* write, int status){
   if(config_log_level >=2) fprintf(stderr, "%s()\n", __func__);

   printf("Write status: %d\n", status);
}


void neat_send(unsigned char* payload, int size, int itgFlowId){
    if(config_log_level >=2) fprintf(stderr, "%s()\n", __func__);


    neat_write(nfd[itgFlowId].ctx, nfd[itgFlowId].flow, payload, size, NULL, 0);

}

void neat_stop_event_loop(int itgFlowId){
    neat_stop_event_loop(nfd[itgFlowId].ctx);
}


void* neat_client_start(void* param){
  
 if(config_log_level >=2) fprintf(stderr, "%s()\n", __func__);

 
    
    int arg, result;
    char *arg_property = NULL;

    neatParserParams para = *(neatParserParams *)param;

    pthread_mutex_lock(&nfd[para.flowid].nt_mp);

    uint64_t prop;
    result = EXIT_SUCCESS;

    memset(&nfd[para.flowid].ops, 0, sizeof(nfd[para.flowid].ops));
    memset(&nfd[para.flowid].in_buffer, 0, sizeof(nfd[para.flowid].in_buffer));
    nfd[para.flowid].config_primary_dest_addr = para.addr;

    // Reset optind so we can make getopt work multiple times during the same execution.
    optind = 1;

    while ((arg = getopt(para.optc, para.neatOpt, "P:R:S:T:Jv:A:")) != -1) {
        switch(arg) {
        case 'P':
            //arg_property = optarg;
            if (read_file(optarg, &arg_property) < 0) {
                fprintf(stderr, "Unable to read properties from %s: %s",
                        optarg, strerror(errno));
                result = EXIT_FAILURE;
                goto cleanup;
            }
            if (config_log_level >= 1) {
                fprintf(stderr, "%s - option - properties: %s\n", __func__, arg_property);
            }
            break;
        case 'R':
            config_rcv_buffer_size = atoi(optarg);
            if (config_log_level >= 1) {
                fprintf(stderr, "%s - option - receive buffer size: %d\n", __func__, config_rcv_buffer_size);
            }
            break;
        case 'S':
            config_snd_buffer_size = atoi(optarg);
            if (config_log_level >= 1) {
                fprintf(stderr, "%s - option - send buffer size: %d\n", __func__, config_snd_buffer_size);
            }
            break;
        case 'J':
            if (config_log_level >= 1) {
                fprintf(stderr, "%s - option - json stats on send enabled\n", __func__);
            }
            break;
        case 'v':
            config_log_level = atoi(optarg);
            if (config_log_level >= 1) {
                fprintf(stderr, "%s - option - log level: %d\n", __func__, config_log_level);
            }
            break;
        case 'T':
            nfd[para.flowid].config_timeout = atoi(optarg);
            if (config_log_level >= 1) {
                fprintf(stderr, "%s - option - timeout: %d seconds\n", __func__, nfd[para.flowid].config_timeout);
            }
            break;
        case 'A':
            nfd[para.flowid].config_primary_dest_addr = optarg;
            if (config_log_level >= 1) {
                fprintf(stderr, "%s - option - primary dest. address: %s\n", __func__, nfd[para.flowid].config_primary_dest_addr);
            }
            break;
        default:
            goto cleanup;
            break;
        }
    }
    

    if((nfd[para.flowid].buffer_rcv = (unsigned char*)malloc(config_snd_buffer_size)) == NULL){
        fprintf(stderr, "%s - error: could not allocate receive buffer\n", __func__);
        result = EXIT_FAILURE;
        goto cleanup;
    }

    if((nfd[para.flowid].buffer_snd = (unsigned char*)malloc(config_snd_buffer_size)) == NULL){
        fprintf(stderr, "%s - error: could not allocate send buffer\n", __func__);
        result = EXIT_FAILURE;
        goto cleanup;
    }

    if((nfd[para.flowid].in_buffer.buffer = (unsigned char*)malloc(config_snd_buffer_size)) == NULL){
        fprintf(stderr, "%s - error: could not allocate in buffer\n", __func__);
        result = EXIT_FAILURE;
        goto cleanup;
    }
    
    if ((nfd[para.flowid].ctx = neat_init_ctx()) == NULL) {
        fprintf(stderr, "%s - error: could not initialize context\n", __func__);
        result = EXIT_FAILURE;
        goto cleanup;
    }

    if ((nfd[para.flowid].flow = neat_new_flow(nfd[para.flowid].ctx)) == NULL) {
        fprintf(stderr, "%s - error: could not create new neat flow\n", __func__);
        result = EXIT_FAILURE;
        goto cleanup;
    }
    

   if (neat_set_property(nfd[para.flowid].ctx, nfd[para.flowid].flow,  arg_property ? arg_property : nfd[para.flowid].config_property)) {
        fprintf(stderr, "%s - error: neat_set_property\n", __func__);
        result = EXIT_FAILURE;
        goto cleanup;
    }


    nfd[para.flowid].ops.on_connected = on_connected;
    nfd[para.flowid].ops.on_error = on_error;
    nfd[para.flowid].ops.on_close = on_close;
    nfd[para.flowid].ops.on_aborted = on_abort;
    nfd[para.flowid].ops.on_network_status_changed = on_network_changed;
    nfd[para.flowid].ops.on_timeout = on_timeout;
    nfd[para.flowid].ops.userData = (void*)para.flowid;


    if (neat_set_operations(nfd[para.flowid].ctx, nfd[para.flowid].flow, &nfd[para.flowid].ops)) {
        fprintf(stderr, "%s - error: neat_set_operations\n", __func__);
        result = EXIT_FAILURE;
        goto cleanup;
    }

    // wait for on_connected or on_error to be invoked
    if (neat_open(nfd[para.flowid].ctx, nfd[para.flowid].flow, nfd[para.flowid].config_primary_dest_addr ?
             nfd[para.flowid].config_primary_dest_addr : "localhost", 8080, NULL, 0) == NEAT_OK) {

        neat_start_event_loop(nfd[para.flowid].ctx, NEAT_RUN_DEFAULT);

    } else {
        fprintf(stderr, "%s - error: neat_open\n", __func__);
        result = EXIT_FAILURE;
        goto cleanup;
    }


    // Cleanup

cleanup:
    if(nfd[para.flowid].buffer_rcv != NULL)
        free(nfd[para.flowid].buffer_rcv);
    if(nfd[para.flowid].buffer_snd != NULL)
        free(nfd[para.flowid].buffer_snd);
    if(nfd[para.flowid].in_buffer.buffer != NULL)
        free(nfd[para.flowid].in_buffer.buffer);

    if (arg_property) {
        free(arg_property);
    }
    if (nfd[para.flowid].ctx != NULL) {
        neat_free_ctx(nfd[para.flowid].ctx);
    }

    pthread_exit(0);
    return NULL;
}

