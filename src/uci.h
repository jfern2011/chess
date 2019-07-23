#ifndef __UCI_PROTOCOL_H__
#define __UCI_PROTOCOL_H__

#include "protocol.h"

namespace Chess
{
    class UCI final : public Protocol
    {

    public:

        UCI();

        UCI(const UCI& uci) = default;
        UCI(UCI&& uci)      = default;

        UCI& operator=(const UCI& uci) = default;
        UCI& operator=(UCI&& uci)      = default;

        ~UCI() = default;

        bool cmd_stop(const std::string& );

        bool init(std::shared_ptr<CommandInterface>
                  cmd) override;

    protected:

    };
}

#endif
