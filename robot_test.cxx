#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <cstring>

#include <ndds/ndds_cpp.h>

#include "common.h"

void client_rr(const std::string & service_name);
void server_rr(const std::string & service_name);

void client_func(const std::string & service_name, const std::string & testname);
void server_func(const std::string & service_name);

void usage()
{
  printf("Usage: robot_test domainid [client_rr|client_func|server_rr|server_func]\n");
}

int main(int argc, char *argv[])
{
    try {
        int domainid = 65;
        std::string service_name = "RobotControl";

        /*NDDSConfigLogger::get_instance()->set_verbosity_by_category(
          NDDS_CONFIG_LOG_CATEGORY_API,
          NDDS_CONFIG_LOG_VERBOSITY_STATUS_ALL);
        */

        if (argc >= 3)
        {
            domainid = atoi(argv[1]);
            DDSDomainParticipant * default_participant = 
              dds::rpc::details::DefaultDomainParticipant::singleton().set_domainid(domainid).get();

            std::string testname;
            if (argc == 4)
              testname = argv[3];

            if (strcmp(argv[2], "client_rr") == 0)
                client_rr(service_name);
            else if (strcmp(argv[2], "server_rr") == 0)
                server_rr(service_name);
            else if (strcmp(argv[2], "client_func") == 0)
                client_func(service_name, testname);
            else if (strcmp(argv[2], "server_func") == 0)
                server_func(service_name);
            else
                usage();
        }
        else
            usage();

        return 0;
    }
    catch (std::exception & ex)
    {
        printf("Exception in main: %s\n", ex.what());
    }
    catch (...)
    {
        printf("Unknown exception in main\n");
    }
}
