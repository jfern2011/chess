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
	name  = "see_ut",
	srcs  = ["ut_main.cpp", "see_ut.cpp"],
	copts = [
		"-DDEBUG", "-Wall", "-DSAFE_BUFFER", "-DSEE_TEST"
	],
	deps  = [
		"@googletest//:gtest_main",
		"//src:chess",
	],
)

cc_test(
	name  = "selection_sort_ut",
	srcs  = ["ut_main.cpp", "selection_sort_ut.cpp"],
	copts = [
		"-DDEBUG", "-Wall", "-DSAFE_BUFFER"
	],
	deps  = [
		"@googletest//:gtest_main",
		"//src:chess",
	],
)

cc_test(
	name  = "search_ut",
	srcs  = ["ut_main.cpp", "search_ut.cpp"],
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
		"DataTables_ut.cpp",
		"MoveGen_ut.cpp",
		"Position_ut.cpp",
		"search_ut.cpp",
		"see_ut.cpp",
		"selection_sort_ut.cpp",
		"ut_main.cpp"
	],
	copts = [
		"-DDEBUG", "-Wall", "-DSAFE_BUFFER", "-DSEE_TEST"
	],
	deps  = [
		"@googletest//:gtest_main",
		"//src:chess",
	],
)
