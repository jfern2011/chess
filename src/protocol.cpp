#include "protocol.h"

namespace Chess
{
    Protocol::Protocol()
        : m_engine(nullptr)
    {
    }

    bool Protocol::install(std::shared_ptr<EngineInterface>
                           engine)
    {
        AbortIfNot(engine, false);
        
        m_engine = engine;
        return true;
    }
}
