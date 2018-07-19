cc_binary(
    name = "user_server",
    srcs = [
        "ssologin_server.cc",
    ],
    deps = [
        "//common:ssologin_crypto",
        "//proto:ssologin",
        "@com_github_grpc_grpc//:grpc++",
        "//third_party/mysql:mysqlclient",
        "//third_party/mysql-helper:mysqlhelper"
    ],
    data = ["//key:private.pem"],
)