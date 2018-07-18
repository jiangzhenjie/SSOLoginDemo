cc_binary(
    name = "user_client",
    srcs = [
        "ssologin_client.cc",
    ],
    deps = [
    	"//common:ssologin_crypto",
    	"//proto:ssologin",
      	"@com_github_grpc_grpc//:grpc++",
    ],
    data = ["//key:public.pem"],
)