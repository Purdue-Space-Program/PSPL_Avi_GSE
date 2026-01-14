cc_library(
    name = "channels",
    srcs = [
        "generator.cpp",
        "synnax.cpp",
        "commands.cpp",
        "telem.cpp",
    ],
    hdrs = [
        "generator.hpp",
        "synnax.hpp",
        "commands.hpp",
        "telem.hpp",
    ],
    deps = [
        "@synnax//client/cpp:synnax",
        "@nlohmann_json//:json",
    ],
    visibility = ["//visibility:public"],
    includes = ["external/synnax+"],
)
