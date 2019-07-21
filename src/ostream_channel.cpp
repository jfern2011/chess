#include "ostream_channel.h"

namespace Chess
{
    OStreamChannel::OStreamChannel(std::shared_ptr<std::ostream> stream)
        : _stream(stream)
    {
    }

    bool OStreamChannel::write(const char* buf, size_t size)
    {
        AbortIf(_stream == nullptr, false);

        _stream->write( buf, size );

        AbortIfNot(_stream->good(), false);

        return true;
    }
}
