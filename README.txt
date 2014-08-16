README for the dds-rpc-cxx project.

rtiddsgen robot.idl -D BASIC -D REDEFINE_DDS_TYPES -namespace -language C++

rtiddsgen rpc_types.idl -D BASIC -D REDEFINE_DDS_TYPES -namespace -language C++

rm -fr robot.cxx robot.h robotSupport.* robotPlugin.* rpc_types.cxx rpc_types.h rpc_typesPlugin.* rpc_typesSupport.* rpc_types_publisher* robot_publisher* rpc_types_subscriber* robot_subscriber* rpc_types-vs2012.sln 
