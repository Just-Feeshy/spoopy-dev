/*
 * This file is written by Claude AI, don't mad at me.
 * It's a multi-threaded test application for the Spoopy framework.
 * I was too lazy to write try my own test case and claude AI did it for me.
 * Honestly, it came up with sections I would't expect could break.
*/

#include <spoopy_api.h>

#define REPETITIONS 10000
#define THREAD_COUNT 16

typedef struct thread_args {
    int value;
    bool should;
    int thread_id;
    int work_multiplier;
} thread_args_t;

static void* worker_func(void* args) {
    thread_args_t* thread_args = (thread_args_t*)args;
    int value = thread_args->value;
    int thread_id = thread_args->thread_id;
    int work_amount = REPETITIONS * thread_args->work_multiplier;

    thread_args_t* result = spoopy_heap_alloc(sizeof(thread_args_t));

    SPOOPY_LOG_INFO("Worker thread %d started with value: %d", thread_id, value);
    SPOOPY_LOG_INFO("Thread %d: Should we for no reason?: %s", thread_id, thread_args->should ? "yes" : "no");
    SPOOPY_LOG_INFO("Thread %d: Will do %d iterations", thread_id, work_amount);

    // Do some work - vary the amount based on work_multiplier
    int i = 0;
    while(i < work_amount) {
        i++;
        // Every thread does slightly different work
        if (i % 1000 == 0 && thread_id % 3 == 0) {
            // Some threads do extra computation
            volatile int dummy = i * thread_id;
            (void)dummy; // Avoid unused variable warning
        }
    }

    result->value = value + thread_id;
    result->should = !thread_args->should; // Flip the boolean
    result->thread_id = thread_id;
    result->work_multiplier = thread_args->work_multiplier;

    SPOOPY_LOG_SUCCESS("Worker thread %d completed with value: %d", thread_id, result->value);
    SPOOPY_LOG_SUCCESS("Thread %d: Should we for a reason?: %s", thread_id, result->should ? "yes" : "no");

    return result;
}

int main(int argc, char** argv) {
    SPOOPY_LOG_INFO("Multi-Thread Parallel Test Application - Creating %d threads", THREAD_COUNT);

    // Initialize thread system
    spoopy_sdl_thread_init();

    assert(spoopy_get_main_id() != 0);
    assert(spoopy_sdl_current_is_main());

    SPOOPY_LOG_INFO("Thread system initialized successfully");

    // Arrays to hold thread data
    spoopy_sdl_thread_t* threads[THREAD_COUNT];
    thread_args_t thread_args[THREAD_COUNT];
    spoopy_core_thread_data_t core_data[THREAD_COUNT];

    // Create all threads with different parameters
    SPOOPY_LOG_INFO("Creating %d worker threads...", THREAD_COUNT);

    for (int i = 0; i < THREAD_COUNT; i++) {
        // Set up arguments for each thread
        thread_args[i].value = 10 + (i * 5);  // Different starting values
        thread_args[i].should = (i % 2 == 0); // Alternate true/false
        thread_args[i].thread_id = i;
        thread_args[i].work_multiplier = 1 + (i % 3); // Vary work amount: 1x, 2x, or 3x

        // Determine thread priority based on thread ID
        spoopy_thread_priority_t priority;
        if (i < 4) {
            priority = SPOOPY_THREAD_PRIO_HIGH;
        } else if (i < 8) {
            priority = SPOOPY_THREAD_PRIO_NORMAL;
        } else if (i < 12) {
            priority = SPOOPY_THREAD_PRIO_LOW;
        } else {
            priority = SPOOPY_THREAD_PRIO_CRITICAL;
        }

        // Create core thread data
        spoopy_create_core_thread_data(
            &core_data[i],
            worker_func,
            &thread_args[i],
            priority
        );

        // Create thread with descriptive name
        char thread_name[32];
        const char* prio_names[] = {"LOW", "NORM", "HIGH", "CRIT"};
        snprintf(thread_name, sizeof(thread_name), "Worker-%d-%s", i, prio_names[priority]);

        threads[i] = spoopy_sdl_thread_create(thread_name, core_data[i]);

        if (!threads[i]) {
            SPOOPY_LOG_ERROR("Failed to create thread %d", i);
            // Clean up previously created threads
            for (int j = 0; j < i; j++) {
                spoopy_sdl_thread_wait(threads[j]);
            }
            spoopy_sdl_thread_shutdown();
            return -1;
        }

        SPOOPY_LOG_INFO("Created thread %d with priority %s, starting value %d, work multiplier %dx",
                       i, prio_names[priority], thread_args[i].value, thread_args[i].work_multiplier);
    }

    SPOOPY_LOG_INFO("All %d threads created successfully!", THREAD_COUNT);
    SPOOPY_LOG_INFO("Main thread doing some work while workers are running...");

    // Do some work in main thread while others are running
    int main_work = 0;
    for (int i = 0; i < 5; i++) {
        SPOOPY_LOG_INFO("Main thread working... iteration %d", i + 1);
        for (int j = 0; j < REPETITIONS / 2; j++) {
            main_work++;
        }
    }

    SPOOPY_LOG_INFO("Main thread finished its work, now checking thread status...");

    // Check which threads are finished (non-blocking)
    int finished_count = 0;
    for (int i = 0; i < THREAD_COUNT; i++) {
        void* result = NULL;
        bool is_finished = spoopy_sdl_thread_get_result(threads[i], &result);
        if (is_finished) {
            finished_count++;
            SPOOPY_LOG_INFO("Thread %d finished early!", i);
        }
    }

    SPOOPY_LOG_INFO("%d out of %d threads finished early", finished_count, THREAD_COUNT);

    // Wait for all threads to complete and collect results
    SPOOPY_LOG_INFO("Waiting for all threads to complete...");

    int total_result_sum = 0;
    int true_count = 0;
    int false_count = 0;

    for (int i = 0; i < THREAD_COUNT; i++) {
        SPOOPY_LOG_INFO("Waiting for thread %d...", i);

        void* result_ptr = spoopy_sdl_thread_wait(threads[i]);

        if (result_ptr) {
            thread_args_t* result = (thread_args_t*)result_ptr;

            SPOOPY_LOG_INFO("Thread %d completed: value=%d, should=%s",
                           result->thread_id, result->value, result->should ? "yes" : "no");

            total_result_sum += result->value;
            if (result->should) {
                true_count++;
            } else {
                false_count++;
            }

            spoopy_heap_free(result);
        } else {
            SPOOPY_LOG_ERROR("Thread %d did not return a valid result", i);
        }
    }

    // Print summary
    SPOOPY_LOG_SUCCESS("=== SUMMARY ===");
    SPOOPY_LOG_SUCCESS("All %d threads completed successfully!", THREAD_COUNT);
    SPOOPY_LOG_SUCCESS("Total sum of all thread results: %d", total_result_sum);
    SPOOPY_LOG_SUCCESS("Threads returning 'should=true': %d", true_count);
    SPOOPY_LOG_SUCCESS("Threads returning 'should=false': %d", false_count);
    SPOOPY_LOG_SUCCESS("Main thread work counter: %d", main_work);

    // Verify we're still on main thread
    if (spoopy_sdl_current_is_main()) {
        SPOOPY_LOG_SUCCESS("Confirmed: Still running on main thread");
    } else {
        SPOOPY_LOG_ERROR("ERROR: Not on main thread anymore!");
    }

    // Shutdown thread system
    spoopy_sdl_thread_shutdown();

    SPOOPY_LOG_SUCCESS("Multi-Thread Parallel Test Application completed successfully");

    return 0;
}
