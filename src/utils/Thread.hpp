#pragma once

#include <iostream>
#include <thread>
#include <pthread.h>

#ifdef __APPLE__
    #include <mach/mach.h>
    #include <mach/thread_policy.h>
#else
    #include <sched.h>
#endif

namespace Hyperion::Utils {
    inline void pin_thread(int cpu_id) {
        #ifdef __APPLE__
            thread_extended_policy_data_t policy;
            policy.timeshare = 0;

            kern_return_t ret = thread_policy_set(
                pthread_mach_thread_np(pthread_self()),
                THREAD_EXTENDED_POLICY,
                (thread_policy_t)& policy,
                THREAD_EXTENDED_POLICY_COUNT
            );

            if (ret != KERN_SUCCESS) {
                std::cerr << "WARNING: Failed to set macOS thread policy: " << ret << "\n";
            }

            (void) cpu_id;
        #else
            cpu_set_t cpuset;
            CPU_ZERO(&cpuset);
            CPU_SET(cpu_id, &cpuset);

            int ret = pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset);

            if (ret != 0) {
                std::cerr << "WARNING: Failed to pin thread to Core " << cpu_id << "\n";
            }
        #endif
    }
}