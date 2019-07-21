#ifndef __COMMAND_HANDLER_H__
#define __COMMAND_HANDLER_H__

#include <functional>
#include <string>

namespace Chess
{
    class CommandHandler
    {

    public:

        CommandHandler();

        CommandHandler(const CommandHandler& cmd) = default;
        CommandHandler(CommandHandler&& cmd)      = default;

        CommandHandler& operator=(const CommandHandler& cmd) = default;
        CommandHandler& operator=(CommandHandler&& cmd)      = default;

        virtual ~CommandHandler() = default;

        virtual bool poll(int timeout ) = 0;

        std::function<bool(const std::string&)>
            input_signal;
    };
}

#endif
