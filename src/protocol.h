#ifndef __PROTOCOL_H__
#define __PROTOCOL_H__

#include <ostream>
#include <memory>
#include <string>

#include "command_interface.h"
#include "engine_interface.h"

namespace Chess
{
    class Protocol
    {

    public:

        explicit Protocol(std::shared_ptr<std::ostream>
                          stream);

        Protocol(const Protocol& protocol) = default;
        Protocol(Protocol&& protocol)      = default;

        Protocol& operator=(const Protocol& protocol) = default;
        Protocol& operator=(Protocol&& protocol)      = default;

        virtual ~Protocol() = default;

        virtual bool init(
            std::shared_ptr<CommandInterface> cmd) = 0;

        bool install(std::shared_ptr< EngineInterface >
                     engine);

        bool terminated() const;

    protected:

        std::shared_ptr<EngineInterface>
            m_engine;

        bool m_quitRequested;

        std::shared_ptr<  std::ostream >
            m_stream;
    };
}

#endif
