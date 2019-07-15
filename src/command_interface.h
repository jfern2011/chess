#ifndef __COMMAND_INTERFACE_H__
#define __COMMAND_INTERFACE_H__

#include <functional>
#include <map>
#include <string>

namespace Chess
{
    class CommandInterface final
    {

    public:

        using handler_t = std::function<bool(const std::string&)>;

        CommandInterface();

        CommandInterface(const CommandInterface& cmd) = default;
        CommandInterface(CommandInterface&& cmd)      = default;

        CommandInterface& operator=(const CommandInterface& cmd) = default;
        CommandInterface& operator=(CommandInterface&& cmd)      = default;

        ~CommandInterface() = default;

        bool install( const std::string& name, handler_t handler );

        bool process( const std::string& data);

    private:

        std::map<std::string, handler_t>
            _cmd_map;
    };
}

#endif
