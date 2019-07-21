#ifndef __OSTREAM_CHANNEL_H__
#define __OSTREAM_CHANNEL_H__

#include <memory>
#include <ostream>

#include "OutputChannel.h"

namespace Chess
{
    class OStreamChannel final : public OutputChannel
    {

    public:

        explicit OStreamChannel(std::shared_ptr<std::ostream> stream);

        OStreamChannel(const OStreamChannel& channel) = default;
        OStreamChannel(OStreamChannel&& channel)      = default;

        OStreamChannel& operator=(const OStreamChannel& channel) = default;
        OStreamChannel& operator=(OStreamChannel&& channel)      = default;

        ~OStreamChannel() = default;

        bool write(const char* buf, size_t size) override;

    private:

        std::shared_ptr<std::ostream>
            _stream;
    };
}

#endif
