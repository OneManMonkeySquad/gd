#include "mpmc_bounded_queue.h"
#include "foundation/foundation.h"
#include "foundation/print.h"
#include "foundation/api_registry.h"
#include <windows.h>

import std;

#define assert(expression) (void)(                                                       \
            (!!(expression)) ||                                                              \
            (_wassert(_CRT_WIDE(#expression), _CRT_WIDE(__FILE__), (unsigned)(__LINE__)), 0) \
        )

namespace
{
    using fiber_t = void*;

    struct fiber_data_t
    {
        size_t fiber_idx;
    };

    std::mutex glob;
    std::vector<std::thread> threads;
    std::atomic_bool exit_workers;

    std::vector<fiber_t> fiber_pool;
    mpmc_bounded_queue<size_t> free_fiber_indices{ 256 }; // actually 160

    std::array<std::atomic_uint32_t, 32> counters;
    std::vector<size_t> free_counter_indicies;

    mpmc_bounded_queue<std::pair<fd::jobdecl_t, std::atomic_uint32_t*>> job_queue{ 64 };

    std::vector<std::pair<std::atomic_uint32_t*, fiber_t>> sleeping_fiber_queue;

    void worker_fiber(void* data)
    {
        auto fiber_data = *(fiber_data_t*)data;
        assert(fiber_data.fiber_idx >= 0 && fiber_data.fiber_idx < 160);

        while (true)
        {
            // can we wake up any sleeping fiber?
            if (!sleeping_fiber_queue.empty())
            {
                fiber_t switchToFiber = nullptr;
                {
                    std::scoped_lock lock{ glob };

                    for (int i = 0; i < sleeping_fiber_queue.size(); ++i)
                    {
                        auto& pair = sleeping_fiber_queue[i];
                        if (*pair.first == 0)
                        {
                            // remove sleeping fiber
                            if (sleeping_fiber_queue.size() > 1)
                            {
                                sleeping_fiber_queue[i] = sleeping_fiber_queue[sleeping_fiber_queue.size() - 1];
                            }
                            sleeping_fiber_queue.pop_back();

                            // switch to sleeping fiber
                            switchToFiber = pair.second;
                            break;
                        }
                    }
                }

                if (switchToFiber != nullptr)
                {
                    // put current fiber into pool
                    assert(fiber_data.fiber_idx >= 0 && fiber_data.fiber_idx < 160);
                    free_fiber_indices.enqueue(fiber_data.fiber_idx);

                    ::SwitchToFiber(switchToFiber);
                    continue;
                }
            }

            // can we start a new task?

            std::pair<fd::jobdecl_t, std::atomic_uint32_t*> job;
            if (job_queue.dequeue(job))
            {
                job.first.task(job.first.data);

                *job.second -= 1;
                continue;
            }

            // nothing to do
            // free_fiber_indices.push_back(fiber_data.fiber_idx);
             // ::SwitchToFiber(fiber_data.parent_fiber);
        }
    }

    void translate_seh_to_cpp(unsigned int code, EXCEPTION_POINTERS* ep)
    {
        throw std::runtime_error("SEH exception occurred");
    }

    void worker_thread()
    {
        _set_se_translator(translate_seh_to_cpp);

        auto this_fiber = ::ConvertThreadToFiber(nullptr);

        while (!exit_workers)
        {
            size_t fiber_idx;
            free_fiber_indices.dequeue(fiber_idx);

            ::SwitchToFiber(fiber_pool[fiber_idx]);
        }
    }

    void init()
    {
        exit_workers = false;

        auto num_cpus = 0;
        for (int i = 0; i < num_cpus; ++i)
        {
            threads.emplace_back([i]() { worker_thread(); });
        }

        for (int i = 0; i < 32; ++i)
        {
            free_counter_indicies.push_back(i);
        }

        auto this_fiber = ::ConvertThreadToFiber(nullptr);
        for (int i = 0; i < 160; ++i)
        {
            fiber_data_t* data = new fiber_data_t();
            data->fiber_idx = i;

            auto fiber = ::CreateFiber(0, &worker_fiber, data);
            fiber_pool.push_back(fiber);
            free_fiber_indices.enqueue(i);
        }
    }

    void deinit()
    {
        exit_workers = true;

        for (auto& thread : threads)
        {
            thread.join();
        }
    }

    fd::jobs_handle_t run_jobs(std::initializer_list<fd::jobdecl_t> jobs)
    {
        // fd::println("JS run_jobs");

        std::scoped_lock lock{ glob };

        auto counter_idx = free_counter_indicies.back();
        free_counter_indicies.pop_back();

        auto counter = &counters[counter_idx];
        *counter = jobs.size();

        for (auto job : jobs)
        {
            while (!job_queue.enqueue(std::make_pair(job, counter)))
            {
                fd::println("fixme");
            }
        }

        return counter;
    }

    void wait_for_counter(fd::jobs_handle_t handle, std::uint32_t value)
    {
        // fd::println("JS wait_for_counter");

        auto counter = (std::atomic_uint32_t*)handle;
        if (*counter == 0)
            return;


        {
            std::scoped_lock lock{ glob };
            sleeping_fiber_queue.push_back(std::make_pair(counter, ::GetCurrentFiber()));
        }

        size_t free_fiber_idx;
        free_fiber_indices.dequeue(free_fiber_idx);

        ::SwitchToFiber(fiber_pool[free_fiber_idx]);
    }

    void wait_for_counter_no_fiber(fd::jobs_handle_t handle, std::uint32_t value)
    {
    }
}

extern "C" __declspec(dllexport) void load_plugin(fd::api_registry_t& api, bool reload, void* old_dll)
{
    fd::job_system_t js;
    js.init = &init;
    js.deinit = &deinit;
    js.run_jobs = &run_jobs;
    js.wait_for_counter = &wait_for_counter;
    js.wait_for_counter_no_fiber = &wait_for_counter_no_fiber;

    api.set(js);
}

extern "C" __declspec(dllexport) void unload_plugin(fd::api_registry_t& api, bool reload)
{
    // api.remove(js);
   // delete js;
}