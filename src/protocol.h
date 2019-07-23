#ifndef __PROTOCOL_H__
#define __PROTOCOL_H__

#include <memory>
#include <string>

#include "command_interface.h"
#include "engine_interface.h"

namespace Chess
{
    class Protocol
    {

    public:

        Protocol();

        Protocol(const Protocol& protocol) = default;
        Protocol(Protocol&& protocol)      = default;

        Protocol& operator=(const Protocol& protocol) = default;
        Protocol& operator=(Protocol&& protocol)      = default;

        virtual ~Protocol() = default;

        virtual bool init(
            std::shared_ptr<CommandInterface> cmd) = 0;

        bool install(std::shared_ptr< EngineInterface >
                     engine);

    protected:

        std::shared_ptr<EngineInterface>
            m_engine;
    };
}

#endif
