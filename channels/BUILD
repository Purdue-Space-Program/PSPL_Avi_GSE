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
    includes = [
        "external/synnax+/client/cpp",
    ],
    deps = [
        "@nlohmann_json//:json",
        "@synnax//client/cpp:synnax",
    ],
    visibility = ["//visibility:public"],
)
