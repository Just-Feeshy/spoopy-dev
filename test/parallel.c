#include <spoopy.h>
#include <spoopy_thread.h>

typedef struct thread_args{
    int value = 23;
    bool should_false = false;
} thread_args_t;

static void* simple_thread_func(void* args) {
    thread_args_t* thread_args = (thread_args_t*)args;
    SPOOPY_LOG_INFO("Thread started with value: %d", thread_args->value);

    if(thread_args->should_false) {
        SPOOPY_LOG_ERROR("Thread encountered an error, should_false is true");
        return NULL;
    }

    return (void*)(intptr_t)(thread_args->value * 2);
}

int main(int argc, char** argv) {
    SPOOPY_LOG_INFO("Parallel Test Application");
    spoopy_sdl_thread_init();

    assert(spoopy_get_main_id() != 0);
    assert(spoopy_sdl_current_is_main());

    spoopy_core_thread_data_t core_data;
    spoopy_create_core_thread_data(&core_data,

    spoopy_sdl_thread_shutdown();

    SPOOPY_LOG_SUCCESS("Parallel Test Application completed successfully");
    return 0;
}
