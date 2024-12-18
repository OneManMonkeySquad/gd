
import <windows.h>;
import foundation;
import std;

namespace
{
    foundation::api_registry api_registry;
}

void job1_B_X(void* data)
{
    foundation::println("       job1_B_X");
}

void job1_B_Y(void* data)
{
    foundation::println("       job1_B_Y");
}

void job1_B_Z(void* data)
{
    foundation::println("       job1_B_Z");
}

void job1_A(void* data)
{
    foundation::println("   job1_A");
}

void job1_B(void* data)
{
    foundation::println("   job1_B");

    auto js = api_registry.get<foundation::job_system_t>();
    auto handle = js->run_jobs({ foundation::jobdecl_t{ &job1_B_X }, foundation::jobdecl_t{ &job1_B_Y }, foundation::jobdecl_t{ &job1_B_Z } });
    js->wait_for_counter(handle, 0);
}

void job1(void* data)
{
    foundation::println("job1");

    auto js = api_registry.get<foundation::job_system_t>();
    auto handle = js->run_jobs({ foundation::jobdecl_t{ &job1_A }, foundation::jobdecl_t{ &job1_B } });
    js->wait_for_counter(handle, 0);
}

void job2(void* data)
{
    foundation::println("job2");
}

void job3(void* data)
{
    foundation::println("job3");
}

int main()
{
    foundation::install_global_exception_handler();

    foundation::clear_log();

    foundation::println("");
    foundation::println("*-*-* INIT *-*-*");

#ifdef _WIN32
    std::setlocale(2, ".UTF8");
#endif

    auto game = api_registry.get<foundation::game_t>();

    foundation::plugin_manager_t plugin_manager{ api_registry };
    plugin_manager.init();

    foundation::println("");
    foundation::println("*-*-* RUN *-*-*");

    auto js = api_registry.get<foundation::job_system_t>();
    js->init();

    {
        auto handle = js->run_jobs({ foundation::jobdecl_t{ &job1 }, foundation::jobdecl_t{ &job2 }, foundation::jobdecl_t{ &job3 } });
        js->wait_for_counter(handle, 0);

        foundation::println("main after wait");
    }


    game->start();

    while (true)
    {
        if (!game->run_once())
            break;

        plugin_manager.update();
    }

    foundation::println("");
    foundation::println("*-*-* EXIT *-*-*");

    game->exit();

    js->deinit();

    plugin_manager.exit();

    return 0;
}