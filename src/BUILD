cc_library(
	name = "chess",
	srcs = glob(["**/*.cpp"]),
	hdrs = glob(["**/*.h"]),
	#copts = ["-O4"], #32 or 64-bit?
	deps = [
		"//abort:abort",
		"//io_tools:Fd",
		"//types:types",
		"//util:util",
	],
	visibility = [
		"//perft:__pkg__",
		"//ut:__pkg__",
		"//bench:__pkg__",
        "//exec:__pkg__",
	],
)
