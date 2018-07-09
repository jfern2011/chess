cc_test(
	name  = "DataTables_ut",
	srcs  = ["ut_main.cpp", "DataTables_ut.cpp"],
	copts = ["-DDEBUG"],
	deps  = [
		"@googletest//:gtest_main",
		"//src:chess",
	],
)

cc_test(
	name  = "Position_ut",
	srcs  = ["ut_main.cpp", "Position_ut.cpp"],
	copts = ["-DDEBUG"],
	deps  = [
		"@googletest//:gtest_main",
		"//src:chess",
	],
)

cc_test(
	name  = "test_all",
	srcs  = ["DataTables_ut.cpp", "Position_ut.cpp", "ut_main.cpp"],
	copts = ["-DDEBUG"],
	deps  = [
		"@googletest//:gtest_main",
		"//src:chess",
	],
)
