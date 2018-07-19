licenses(["notice"])

package(default_visibility = ["//visibility:public"])

cc_library(
	name = "bcrypt",
    srcs = ["bcrypt.c", 
    		"crypt_blowfish.c", 
    		"crypt_gensalt.c", 
    		"wrapper.c",
    		"x86.S"],
    hdrs = ["bcrypt.h",
    		"BCrypt.hpp",
    		"crypt_blowfish.h",
    		"crypt_gensalt.h",
    		"crypt.h",
    		"ow-crypt.h"],
)