#include "FdInputHandler.h"

#include <cstddef>
#include <poll.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <vector>

#include "abort/abort.h"
#include "util/str_util.h"

namespace Chess
{
    FdInputHandler::FdInputHandler(int fd)
        : _buf(), _fd(fd)
    {
    }

    FdInputHandler::~FdInputHandler()
    {
        if (_fd > 2) ::close(_fd);
    }

    bool FdInputHandler::poll(int timeout)
    {
        struct pollfd pfd;

        AbortIf(_fd < 0, false);

        pfd.fd     = _fd;
        pfd.events = POLLIN;

        const int num_events = ::poll( &pfd, 1, timeout );
        AbortIf(num_events < 0, false);

        if (num_events)
        {
            AbortIf(pfd.revents != POLLIN,
                false);

            int fsize;
            AbortIf(::ioctl( _fd, FIONREAD, &fsize ) < 0,
                false);

            if ( (size_t)fsize > _buf.size() )
                _buf.resize(fsize);

            AbortIf(::read(_fd, &_buf[0], fsize) < 0,
                false);

            std::vector< std::string > tokens; Util::split(
                _buf, tokens, "\n");

            if (input_signal)
            {
                for (auto& cmd : tokens)
                {
                    AbortIfNot(input_signal(cmd),
                        false);
                }
            }
        }

        return true;
    }
}
