cc_test(
	name  = "DataTables_ut",
	srcs  = ["ut_main.cpp", "DataTables_ut.cpp"],
	copts = [
		"-DDEBUG", "-Wall", "-DSAFE_BUFFER"
	],
	deps  = [
		"@googletest//:gtest_main",
		"//src:chess",
	],
)

cc_test(
	name  = "Position_ut",
	srcs  = ["ut_main.cpp", "Position_ut.cpp"],
	copts = [
		"-DDEBUG", "-Wall", "-DSAFE_BUFFER"
	],
	deps  = [
		"@googletest//:gtest_main",
		"//src:chess",
	],
)

cc_test(
	name  = "MoveGen_ut",
	srcs  = ["ut_main.cpp", "MoveGen_ut.cpp"],
	copts = [
		"-DDEBUG", "-Wall", "-DSAFE_BUFFER"
	],
	deps  = [
		"@googletest//:gtest_main",
		"//src:chess",
	],
)

cc_test(
	name  = "test_all",
	srcs  = [
		"DataTables_ut.cpp", "Position_ut.cpp", "MoveGen_ut.cpp",
		"ut_main.cpp"
	],
	copts = [
		"-DDEBUG", "-Wall", "-DSAFE_BUFFER"
	],
	deps  = [
		"@googletest//:gtest_main",
		"//src:chess",
	],
)
