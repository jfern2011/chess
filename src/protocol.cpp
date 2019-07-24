#include "protocol.h"

namespace Chess
{
    Protocol::Protocol(std::shared_ptr<std::ostream> stream)
        : m_engine(nullptr),
          m_quitRequested(false),
          m_stream(stream)
    {
    }

    bool Protocol::install(std::shared_ptr< EngineInterface>
                           engine)
    {
        AbortIfNot(engine, false);
        
        m_engine = engine;
        return true;
    }

    bool Protocol::terminated() const
    {
        return m_quitRequested;
    }
}
