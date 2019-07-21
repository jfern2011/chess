#ifndef __FD_INPUT_HANDLER_H__
#define __FD_INPUT_HANDLER_H__

#include <string>

#include "command_handler.h"

namespace Chess
{
    /**
     * @todo Use Fd class
     */
    class FdInputHandler final : public CommandHandler
    {

    public:

        explicit FdInputHandler(int fd);

        FdInputHandler(const FdInputHandler& handler) = default;
        FdInputHandler(FdInputHandler&& handler)      = default;

        FdInputHandler& operator=(const FdInputHandler& handler) = default;
        FdInputHandler& operator=(FdInputHandler&& handler)      = default;

        virtual ~FdInputHandler();

        bool poll(int timeout) override;

    private:

        std::string _buf;

        int _fd;
    };
}

#endif
