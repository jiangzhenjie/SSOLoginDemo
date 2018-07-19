licenses(["notice"])

package(default_visibility = ["//visibility:public"])

cc_library(
	name = "mysqlhelper",
    srcs = glob(["*.cpp", "*.hpp"]),
    hdrs = glob(["*.hpp"]),
    deps = [
    	"//third_party/mysql:mysqlclient",
    	"@boost//:lexical_cast",
    ]
)