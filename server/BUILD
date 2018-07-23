cc_binary(
    name = "user_server",
    srcs = [
        "ssologin_server.cc",
    ],
    deps = [
        "//third_party/bcrypt:bcrypt",
        "//proto:ssologin",
        "@com_github_grpc_grpc//:grpc++",
        "//common:ssologin_crypto",
	   "//third_party/sqlite:sqlitecpp",
    ],
    data = ["//key:private.pem", "//database:ssologin.db3"],
)