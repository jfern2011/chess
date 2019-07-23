#include "uci.h"

#include "abort/abort.h"

namespace Chess
{
    UCI::UCI()
    {
    }

    bool UCI::cmd_stop( const std::string&  )
    {
        AbortIfNot(m_engine, false);
        AbortIfNot(m_engine->m_search, false);

        m_engine->m_search->abort();
        return true;
    }

    bool UCI::init(std::shared_ptr<CommandInterface>
                   cmd)
    {
        AbortIfNot(cmd, false);
        AbortIfNot(cmd->install("stop", std::bind(
            &UCI::cmd_stop, *this, std::placeholders::_1)),
                false);

        return true;
    }
}
