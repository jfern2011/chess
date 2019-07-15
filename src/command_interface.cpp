#include "command_interface.h"

#include <vector>

#include "abort/abort.h"
#include "util/str_util.h"

namespace Chess
{
    CommandInterface::CommandInterface()
        : _cmd_map()
    {
    }

    bool CommandInterface::install(const std::string& name,
                                   handler_t handler)
    {
        const std::string cmdName = Util::trim(Util::to_lower(name));
        AbortIf(cmdName.empty(), false);

        _cmd_map[cmdName] = handler;
        return true;
    }

    bool CommandInterface::process(const std::string& data)
    {
        const std::string input = Util::trim(data);
        AbortIf(input.empty(), false);

        std::vector<std::string> tokens;  Util::split(input, tokens);
        AbortIf(tokens.empty(), false);

        const std::string cmd = Util::to_lower( tokens[0] );
        auto iter = _cmd_map.find(cmd);

        AbortIf(iter == _cmd_map.end(), false);

        tokens.erase(tokens.begin());

        const std::string args = Util::build_string(tokens, " ");

        // Invoke the handler for this command

        iter->second(args);
        return true;
    }
}
