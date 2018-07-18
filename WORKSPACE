local_repository(
  name = "com_github_grpc_grpc",
  path = "third_party/grpc",
)

git_repository(
    name = "boringssl",
    commit = "2de09aef116ad6ab1a438467ec6f9005d9eb9d12",
    remote = "https://github.com/google/boringssl.git",
)

load("@com_github_grpc_grpc//bazel:grpc_deps.bzl", "grpc_deps")
grpc_deps()