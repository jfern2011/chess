#include "uci.h"

namespace Chess
{
    UCI::UCI()
    {
    }

    bool UCI::install(std::shared_ptr<EngineInterface>
                      engine)
    {
        AbortIfNot(engine, false);
        
        m_engine = engine;
        return true;
    }
}
