cc_library(
	name = "chess",
	srcs = glob(["**/*.cpp"]),
	hdrs = glob(["**/*.h"]),
	#copts = ["-O4"],
	deps = [
		"//types:types",
		"//util:util",
	],
	visibility = [
		"//perft:__pkg__",
		"//ut:__pkg__",
	],
)
