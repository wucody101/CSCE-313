#include <threading.h>

void t_init()
{
    //initalize contexts to invalid state
    for (int i = 0; i < NUM_CTX; i++) {
        contexts[i].state = INVALID;
    }

    //set up first context
    contexts[0].state = VALID;
    //initalize current_context_idx
    current_context_idx = 0;

}

int32_t t_create(fptr foo, int32_t arg1, int32_t arg2)
{
    //find the first empty spot
    int tracker_idx;
    int empty_spot = 0;

    for (uint8_t i = 0; i < NUM_CTX; i++) {
        if (contexts[i].state == INVALID) {
            tracker_idx = i;
            empty_spot = 1;
            break;
        }
    }

    //no empty spots
    if (empty_spot == 0) {
        return 1;
    }

    //initalize using getcontext
    getcontext(&contexts[tracker_idx].context);
    
    //make context
    contexts[tracker_idx].context.uc_stack.ss_sp = malloc(STK_SZ);
    contexts[tracker_idx].context.uc_stack.ss_size = STK_SZ;
    contexts[tracker_idx].context.uc_stack.ss_flags = 0;
    contexts[tracker_idx].context.uc_link = NULL;
    makecontext(&contexts[tracker_idx].context, (void (*)())foo, 2, arg1, arg2);

    //tracker_idx full
    contexts[tracker_idx].state = VALID;
    return 0;
}

int32_t t_yield()
{
    uint8_t saved_idx = current_context_idx;
    int count = 0;

    int temp = -1;
    uint8_t next_idx;

    int first = -1;
    uint8_t first_idx;

    for (uint8_t i = 0; i < NUM_CTX; i++) {
        if (contexts[i].state == VALID) {
            count++;

            //first used context
            if (first == -1) {
                first_idx = i;
                first = 0;
            }

            //next used context
            else if (i > current_context_idx && temp == -1) {
                next_idx = i;
                temp = 0;
            }
        }
    }

    if (first == -1) {
        next_idx = first_idx;
    }

    if (next_idx == current_context_idx) {
        return count - 1;
    }

    current_context_idx = next_idx;

    //set up swapcontext
    ucontext_t* prev = &contexts[saved_idx].context;
    ucontext_t* next = &contexts[next_idx].context;
    swapcontext(prev, next);

    //compute number of valid contexts
    count = 0;
    for (uint8_t i = 0; i < NUM_CTX; i++) {
        if (contexts[i].state == VALID) {
                count++;
        }
    }

    return count - 1;
}

void t_finish()
{
    //free stack
    free(contexts[current_context_idx].context.uc_stack.ss_sp);

    //reset context entry
    memset(&contexts[current_context_idx], 0, sizeof(struct worker_context));
    contexts[current_context_idx].state = DONE;

    int temp = -1;
    uint8_t next_idx;

    int first = -1;
    uint8_t first_idx;

    for (uint8_t i = 0; i < NUM_CTX; i++) {
        if (contexts[i].state == VALID) {

            //first used context
            if (first == -1) {
                first_idx = i;
                first = 0;
            }

            //next used context
            else if (i > current_context_idx && temp == -1) {
                next_idx = i;
                temp = 0;
            }
        }
    }

    if (first == -1) {
        next_idx = first_idx;
    }

    if (next_idx != current_context_idx) {
        current_context_idx = next_idx;
        setcontext(&contexts[next_idx].context);
    }

}
