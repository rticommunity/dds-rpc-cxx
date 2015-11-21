#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <cstring>

#include <ndds/ndds_cpp.h>

void client_rr(int domainid, const std::string & service_name);
void server_rr(int domainid, const std::string & service_name);

void client_func(int domainid, const std::string & service_name);
void server_func(int domainid, const std::string & service_name);

void usage()
{
  printf("Usage: robot_test domainid [client_rr|client_func|server_rr|server_func]\n");
}

int main(int argc, char *argv[])
{
  int domainid = 65;
  std::string service_name = "RobotControl";
  
  /*NDDSConfigLogger::get_instance()->set_verbosity_by_category(
	  NDDS_CONFIG_LOG_CATEGORY_API,
	  NDDS_CONFIG_LOG_VERBOSITY_STATUS_ALL);
  */

  if (argc == 3)
  {
    domainid = atoi(argv[1]);

    if (strcmp(argv[2], "client_rr") == 0)
      client_rr(domainid, service_name);
    else if (strcmp(argv[2], "server_rr") == 0)
      server_rr(domainid, service_name);
    else if (strcmp(argv[2], "client_func") == 0)
      client_func(domainid, service_name);
    else if (strcmp(argv[2], "server_func") == 0)
      server_func(domainid, service_name);
    else
      usage();
  }
  else
    usage();
  
  return 0;
}
