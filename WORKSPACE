local_repository(
  name = "com_github_grpc_grpc",
  path = "third_party/grpc",
)

git_repository(
    name = "boringssl",
    commit = "2de09aef116ad6ab1a438467ec6f9005d9eb9d12",
    remote = "https://github.com/google/boringssl.git",
)

git_repository(
    name = "com_github_nelhage_rules_boost",
    commit = "239ce40e42ab0e3fe7ce84c2e9303ff8a277c41a",
    remote = "https://github.com/nelhage/rules_boost",
)

load("@com_github_grpc_grpc//bazel:grpc_deps.bzl", "grpc_deps")
grpc_deps()

load("@com_github_nelhage_rules_boost//:boost/boost.bzl", "boost_deps")
boost_deps()