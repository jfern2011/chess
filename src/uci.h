#ifndef __UCI_PROTOCOL_H__
#define __UCI_PROTOCOL_H__

#include "protocol.h"

namespace Chess
{
    class UCI final : public Protocol
    {

    public:

        explicit UCI(std::shared_ptr< std::ostream >
                     stream);

        UCI(const UCI& uci) = default;
        UCI(UCI&& uci)      = default;

        UCI& operator=(const UCI& uci) = default;
        UCI& operator=(UCI&& uci)      = default;

        ~UCI() = default;

        bool cmd_debug     (const std::string& enable);
        bool cmd_isready   (const std::string& );
        bool cmd_position  (const std::string& args);
        bool cmd_quit      (const std::string& );
        bool cmd_stop      (const std::string& );
        bool cmd_ucinewgame(const std::string& );

        bool init(std::shared_ptr<CommandInterface>
                  cmd) override;

    private:

        bool m_debug;
    };
}

#endif
