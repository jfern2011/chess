#include "uci.h"

#include <algorithm>
#include <cmath>
#include <string>
#include <vector>

#include "MoveGen4.h"

namespace Chess
{
    OptionBase::OptionBase(const std::string& name,
                           const std::string& type)
        : m_name(name), m_type(type)
    {
    }

    std::string OptionBase::name() const
    {
        return m_name;
    }

    std::string OptionBase::type() const
    {
        return m_type;
    }

    std::string OptionBase::print() const
    {
        return "option" + m_uciOutput + "\n";
    }

    Check::Check(const std::string& name, bool initValue)
        : Option(name, "check"),
          m_default(initValue)
    {
        m_uciOutput = " name " + m_name +
                      " type " + m_type +
                      " default ";

        m_uciOutput += m_default ? "true" : "false";
    }

    Spin::Spin(const std::string& name,
               int64 initValue, int64 min, int64 max)
        : Option(name, "spin"),
          m_default(initValue),
          m_max(max),
          m_min(min)
    {
        m_uciOutput = " name "    + m_name +
                      " type "    + m_type +
                      " default " + std::to_string(m_default) +
                      " min "     + std::to_string(m_min)     +
                      " max "     + std::to_string(m_max);
    }

    Combo::Combo(const std::string& name,
                 const std::string& initValue)
        : Option(name, "combo")
    {
        m_options.push_back(initValue);
        m_default = initValue;

        m_uciOutput = " name "    + m_name +
                      " type "    + m_type +
                      " default " + m_default;

        for (auto& var : m_options)
            m_uciOutput += " var " + var;
    }

    Button::Button(const std::string& name)
        : OptionBase(name, "button"),
          m_updater()
    {
        m_uciOutput = " name " + m_name +
                      " type " + m_type;
    }

    bool Button::update (const std::string& )
    {
        AbortIfNot(m_updater  , false);
        AbortIfNot(m_updater(), false);
        return true;
    }

    String::String(const std::string& name,
                   const std::string& initValue)
        : Option(name, "string")
    {
        m_uciOutput = " name "    + m_name +
                      " type "    + m_type +
                      " default " + m_default;
    }

    UCI::UCI( std::shared_ptr<std::ostream> stream )
        : Protocol(stream),
          m_debug(false), m_options()
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

    bool UCI::cmd_position(const std::string& args)
    {
        AbortIf(args.empty(), false);

        AbortIfNot(m_engine, false);
        AbortIfNot(m_engine->m_master, false);

        const std::string argsLc = Util::to_lower(args);

        std::vector< std::string > tokens;
        Util::split(argsLc, tokens);

        AbortIfNot(m_stream, false);
        Position master(m_stream);

        if (tokens[0] == "startpos")
        {
            AbortIfNot(master.reset(), false );
        }
        else if (tokens[0] == "fen")
        {
            size_t fenInd = argsLc.find( "fen" );

            AbortIf(fenInd == std::string::npos,
                false);

            fenInd += std::string("fen").size();

            size_t fenEnd = argsLc.find("moves");

            size_t size = fenEnd;
            if ( fenEnd != std::string::npos )
                size = fenEnd - fenInd;

            AbortIfNot(master.reset(args.substr(
                fenInd, size)), false);
        }
        else
        {
            Abort(false, "expected 'startpos' or 'fen', "
                  "got '%s'", tokens[0].c_str());
        }

        auto iter = std::find(
            tokens.begin(),tokens.end(),"moves");

        if (iter != tokens.end())
        {
            for (auto mv = ++iter, end = tokens.end();
                 mv != end; ++mv)
            {
                AbortIf(mv->size() < 4, false);
                const std::string from_s = mv->substr(0, 2);
                const std::string to_s   = mv->substr(2, 2);

                square_t from = square_t::BAD_SQUARE;
                square_t to   = square_t::BAD_SQUARE;

                for (int i = 0; i < 64; i++)
                {
                    if (from_s == square_str[i])
                        from = static_cast<square_t>(i);
                    if (to_s   == square_str[i])
                        to   = static_cast<square_t>(i);

                    if (from != square_t::BAD_SQUARE &&
                        to   != square_t::BAD_SQUARE)
                        break;
                }

                const piece_t moved    = master.piece_on(from);

                const bool is_ep = std::abs(from-to) % 8 != 0
                    && moved == piece_t::pawn
                    && master.piece_on(to) == piece_t::empty;

                const piece_t captured =
                    is_ep ? piece_t::pawn :master.piece_on(to);

                piece_t promote = piece_t::empty;
                if (mv->size() > 4)
                {
                    AbortIf(moved != piece_t::pawn, false);

                    switch (mv->at(4))
                    {
                        case 'q': promote = piece_t::queen;  break;
                        case 'n': promote = piece_t::knight; break;
                        case 'b': promote = piece_t::bishop; break;
                        case 'r': promote = piece_t::rook;   break;
                        default:
                            Abort(false, "promotion piece: '%c'",
                                  mv->at(4));
                    }
                }

                const int32 move = pack_move(
                    captured, from, moved, promote, to);

                const bool in_check =
                    master.in_check(master.get_turn() );

                AbortIfNot(MoveGen::validate_move(
                        master, move, in_check),
                    false);

                // The move has been verified; play it

                master.make_move(move);
            }
        }

        // Update the internal position:

        *m_engine->m_master = master;
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

    bool UCI::cmd_uci(const std::string& )
    {
        AbortIfNot(m_stream, false);

        auto& stream = *m_stream;

        stream << "id name Anonymous\n";
        stream << "id author Jason L. Fernandez\n";

        for (const auto& option : m_options)
        {
            stream<< option.second->print()<< "\n";
        }

        stream << "uciok\n" << std::flush;

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
        AbortIfNot(initOptions(), false);

        AbortIfNot(cmd->install("debug", std::bind(
            &UCI::cmd_debug     , std::ref(*this), std::placeholders::_1)),
                false);

        AbortIfNot(cmd->install("isready", std::bind(
            &UCI::cmd_isready   , std::ref(*this), std::placeholders::_1)),
                false);

        AbortIfNot(cmd->install("position", std::bind(
            &UCI::cmd_position  , std::ref(*this), std::placeholders::_1)),
                false);

        AbortIfNot(cmd->install("quit", std::bind(
            &UCI::cmd_quit      , std::ref(*this), std::placeholders::_1)),
                false);

        AbortIfNot(cmd->install("stop", std::bind(
            &UCI::cmd_stop      , std::ref(*this), std::placeholders::_1)),
                false);

        AbortIfNot(cmd->install("uci", std::bind(
            &UCI::cmd_uci       , std::ref(*this), std::placeholders::_1)),
                false);

        AbortIfNot(cmd->install("ucinewgame", std::bind(
            &UCI::cmd_ucinewgame, std::ref(*this), std::placeholders::_1)),
                false);

        return true;
    }

    bool UCI::initOptions()
    {
        AbortIfNot(m_engine, false);
        AbortIfNot(m_engine->m_search, false);

        {
            auto opt = std::make_shared<Spin>("MultiPV",  // Name
                                              1,          // Default
                                              1,          // Min
                                              max_moves); // Max
            AbortIfNot(opt, false);

            opt->m_updater = std::bind(&Search4::setNumberOfLines,
                                       std::ref(*m_engine->m_search),
                                       std::placeholders::_1);

            m_options["MultiPV"] = opt;
        }

        return true;
    }
}
