#include "SharedData.h"

int main(int argc, char** argv)
{
	SharedData shared;

	size_t int_id;
	AbortIfNot(shared.create<int>("sample_int", int_id),
		false);

	size_t str_id;
	AbortIfNot(shared.create<std::string>("sample_string", str_id, "hello"),
		false);

	int int_val;
	AbortIfNot(shared.get(int_id, int_val),
		false);

	std::string str_val;
	AbortIfNot(shared.get(str_id, str_val),
		false);

	std::printf("int = %d, str = '%s'\n", int_val, str_val.c_str());

	AbortIfNot(shared.set<std::string>(str_id, "bye"),
		false);
	AbortIfNot(shared.set(int_id, 12345),
		false);

	AbortIfNot(shared.get(int_id, int_val),
		false);

	AbortIfNot(shared.get(str_id, str_val),
		false);

	std::printf("int = %d, str = '%s'\n", int_val, str_val.c_str());
	std::fflush(stdout);

	return 0;
}
