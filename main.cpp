#include "cmd.h"
#include "engine.h"
#include "util.h"

class Interface
{
	typedef CommandRouter<bool,Engine,const Util::str_v&> router_t;

public:

	Interface()
		: _router(_engine)
	{
	}

	~Interface()
	{
	}

	bool init()
	{
		AbortIfNot(commandRegistration(), false);
		return true;
	}

private:

	bool commandRegistration()
	{
		AbortIfNot(_router.install("force", &Engine::force),
			       false);

		return true;
	}

	Engine   _engine;
	router_t _router;
};

int main(int argc, char** argv)
{
	Interface interface;
	interface.init();

	return EXIT_SUCCESS;
}