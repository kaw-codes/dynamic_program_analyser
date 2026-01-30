#include <catch2/catch_test_macros.hpp> // TEST_CASE
extern "C" {
    #include "libdpa.h"
}

#include <signal.h> // kill
#include <fstream> // status_file

TEST_CASE("tests for launch", "[launch]")
{
    // name of software to analyse
    char *path = const_cast<char*>("./build/tests/targets/hello_world");

    // test of existence of the process
    process_t *proc = NULL;
    int status = launch(path, true, &proc);
    REQUIRE(status == 0);
    REQUIRE(proc != nullptr);
    REQUIRE(kill(proc->pid, 0) == 0); // verif proc exists

    // get process status
    std::string proc_status_path = "/proc/" + std::to_string(proc->pid) + "/status";
    std::ifstream status_file(proc_status_path);
    REQUIRE(status_file.is_open());

    // verify that proc is traced
    std::string line;
    bool is_traced = false;
    int tracer_pid = 0;
    while(std::getline(status_file, line))
    {
        if (line.find("TracerPid:") == 0)
        {
            tracer_pid = std::stoi(line.substr(11));
            INFO("TracerPid found: " << tracer_pid);
            if (tracer_pid != 0)
            {
                is_traced = true;
            }
            break;
        }
    }
    REQUIRE(is_traced);

    // end
    status_file.close();
    if (proc != nullptr)
    {
        kill(proc->pid, SIGKILL);
        free(proc);
        proc = NULL;
    }
}
