import <windows.h>;
import foundation;
import std;

#define assert(expression) (void)(                                                       \
            (!!(expression)) ||                                                              \
            (_wassert(_CRT_WIDE(#expression), _CRT_WIDE(__FILE__), (unsigned)(__LINE__)), 0) \
        )

namespace
{
    using fiber_t = void*;

    std::mutex glob;

    std::vector<std::thread> threads;

    std::vector<fiber_t> fiber_pool;
    std::vector<size_t> free_fiber_indices;

    std::array<std::atomic_uint32_t, 32> counters;
    std::vector<size_t> free_counter_indicies;

    std::queue<std::pair<foundation::jobdecl_t, std::atomic_uint32_t*>> job_queue;

    std::vector<std::pair<std::atomic_uint32_t*, fiber_t>> sleeping_fiber_queue;

    std::atomic_bool exit_workers;

    struct fiber_data_t
    {
        size_t fiber_idx;
    };

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

                            // put current fiber into pool
                            assert(fiber_data.fiber_idx >= 0 && fiber_data.fiber_idx < 160);
                            free_fiber_indices.push_back(fiber_data.fiber_idx);

                            // switch to sleeping fiber
                            switchToFiber = pair.second;

                            break;
                        }
                    }
                }

                if (switchToFiber != nullptr)
                {
                    ::SwitchToFiber(switchToFiber);
                    continue;
                }
            }

            // can we start a new task?
            if (!job_queue.empty())
            {
                std::pair<foundation::jobdecl_t, std::atomic_uint32_t*> job;
                {
                    std::scoped_lock lock{ glob };

                    job = job_queue.front();
                    job_queue.pop();
                }


                job.first.task(job.first.data);

                *job.second -= 1;
                continue;
            }

            // nothing to do
            // free_fiber_indices.push_back(fiber_data.fiber_idx);
             // ::SwitchToFiber(fiber_data.parent_fiber);
        }
    }

    void worker_thread()
    {
        auto this_fiber = ::ConvertThreadToFiber(nullptr);

        while (!exit_workers)
        {
            size_t fiber_idx;
            {
                std::scoped_lock lock{ glob };

                fiber_idx = free_fiber_indices.back();
                free_fiber_indices.pop_back();
            }

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
            free_fiber_indices.push_back(i);
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

    foundation::jobs_handle_t run_jobs(std::initializer_list<foundation::jobdecl_t> jobs)
    {
        foundation::println("JS run_jobs");

        std::scoped_lock lock{ glob };

        auto counter_idx = free_counter_indicies.back();
        free_counter_indicies.pop_back();

        auto counter = &counters[counter_idx];
        *counter = jobs.size();

        for (auto job : jobs)
        {
            job_queue.push(std::make_pair(job, counter));
        }

        return counter;
    }

    void wait_for_counter(foundation::jobs_handle_t handle, std::uint32_t value)
    {
        foundation::println("JS wait_for_counter");

        auto counter = (std::atomic_uint32_t*)handle;
        if (*counter == 0)
            return;

        size_t free_fiber_idx;
        {
            std::scoped_lock lock{ glob };

            sleeping_fiber_queue.push_back(std::make_pair(counter, ::GetCurrentFiber()));

            free_fiber_idx = free_fiber_indices.back();
            free_fiber_indices.pop_back();
        }

        ::SwitchToFiber(fiber_pool[free_fiber_idx]);
    }

    void wait_for_counter_no_fiber(foundation::jobs_handle_t handle, std::uint32_t value)
    {
    }
}

extern "C" __declspec(dllexport) void plugin_load(foundation::api_registry& api, bool reload)
{
    foundation::job_system_t js;
    js.init = &init;
    js.deinit = &deinit;
    js.run_jobs = &run_jobs;
    js.wait_for_counter = &wait_for_counter;
    js.wait_for_counter_no_fiber = &wait_for_counter_no_fiber;

    api.set(js);
}

extern "C" __declspec(dllexport) void plugin_unload(foundation::api_registry& api, bool reload)
{
    // api.remove(js);
   // delete js;
}