load("@rules_cc//cc:defs.bzl", "cc_binary")

cc_binary(
    name = "gse_main",
    srcs = ["main.cpp"],
    deps = [
        "//channels",
        "//net",
    ],
)

load("@hedron_compile_commands//:refresh_compile_commands.bzl", "refresh_compile_commands")

refresh_compile_commands(
    name = "refresh_compile_commands",
    exclude_headers = "all", 
    targets = {
        "//:gse_main": "", 
    },
)
