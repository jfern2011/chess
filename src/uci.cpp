#include "uci.h"

#include "abort/abort.h"

namespace Chess
{
    UCI::UCI( std::shared_ptr<std::ostream> stream )
        : Protocol(stream), m_debug(false)
    {
    }

    bool UCI::cmd_debug(const std::string& enable)
    {
        if (enable == "on")
        {
            m_debug = true;
        }
        else if (enable == "off")
        {
            m_debug = false;
        }
        else
        {
            Abort(false, "Received '%s'",
                  enable.c_str());
        }

        return true;
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

    /**
     * @todo Clear the hash tables
     */
    bool UCI::cmd_ucinewgame(const std::string& )
    {
        // Reset to the starting position. Note this isn't specified
        // by the protocol, but guarantees that a subsequent "go"
        // command (without an accompanying "position" command) will
        // search the starting position

        AbortIfNot(m_engine, false);
        AbortIfNot(m_engine->m_master, false );
        AbortIfNot(m_engine->m_master->reset(),
            false);

        return true;
    }

    bool UCI::init(std::shared_ptr<CommandInterface>
                   cmd)
    {
        AbortIfNot(cmd, false);

        AbortIfNot(cmd->install("debug", std::bind(
            &UCI::cmd_debug     , std::ref(*this), std::placeholders::_1)),
                false);

        AbortIfNot(cmd->install("isready", std::bind(
            &UCI::cmd_isready   , std::ref(*this), std::placeholders::_1)),
                false);

        AbortIfNot(cmd->install("quit", std::bind(
            &UCI::cmd_quit      , std::ref(*this), std::placeholders::_1)),
                false);

        AbortIfNot(cmd->install("stop", std::bind(
            &UCI::cmd_stop      , std::ref(*this), std::placeholders::_1)),
                false);

        AbortIfNot(cmd->install("ucinewgame", std::bind(
            &UCI::cmd_ucinewgame, std::ref(*this), std::placeholders::_1)),
                false);

        return true;
    }
}
