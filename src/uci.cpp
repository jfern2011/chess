#include "uci.h"

#include "abort/abort.h"

namespace Chess
{
    UCI::UCI( std::shared_ptr<std::ostream> stream )
        : Protocol(stream)
    {
    }

    bool UCI::cmd_isready(const std::string& )
    {
        AbortIfNot(m_stream, false);

        ( *m_stream) << "readyok\n";

        m_stream->flush();
        return true;
    }

    bool UCI::cmd_quit( const std::string& )
    {
        m_quitRequested = true;
        return true;
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

        AbortIfNot(cmd->install("isready", std::bind(
            &UCI::cmd_isready, std::ref(*this), std::placeholders::_1)),
                false);

        AbortIfNot(cmd->install("quit", std::bind(
            &UCI::cmd_quit   , std::ref(*this), std::placeholders::_1)),
                false);

        AbortIfNot(cmd->install("stop", std::bind(
            &UCI::cmd_stop   , std::ref(*this), std::placeholders::_1)),
                false);

        return true;
    }
}
