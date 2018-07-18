licenses(["notice"])

package(default_visibility = ["//visibility:public"])

cc_library(
	name = "mysqlclient",
    srcs = ["lib/libmysqlclient.a", "lib/libmysqlservices.a", "lib/libcrypto.dylib", "lib/libssl.dylib"],
    hdrs = [
    	"include/mysql.h", 
    	"include/binary_log_types.h",
    	"include/my_list.h",
    	"include/mysql_com.h",
    	"include/my_command.h",
    	"include/mysql/udf_registration_types.h",
    	"include/mysql/client_plugin.h",
    	"include/mysql/plugin_auth_common.h",
    	"include/mysql_version.h",
    	"include/mysql_time.h",
    	"include/errmsg.h"
    	],
)