#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <cstring>

void client_rr(int domainid, const std::string & service_name);
void server_rr(int domainid, const std::string & service_name);

void client_func(int domainid, const std::string & service_name);
void server_func(int domainid, const std::string & service_name);

void usage()
{
  printf("Usage: robot_test domainid [client|server]\n");
}

int main(int argc, char *argv[])
{
  int domainid = 65;
  std::string service_name = "RobotControl";
  
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
