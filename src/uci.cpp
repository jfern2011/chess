#include "uci.h"

#include <algorithm>
#include <cmath>
#include <limits>
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

    bool UCI::cmd_go( const std::string& args )
    {
        AbortIfNot(m_engine, false);
        AbortIfNot(m_engine->m_search, false);
        AbortIfNot(m_engine->m_master, false);

        const uint64 maxU64 =
            std::numeric_limits<uint64>::max();

        // Inputs used by the search class

        uint32             depth        = max_moves;
        bool               mateSearch   = false;
        uint64             nodeLimit    = maxU64;
        std::vector<int32> searchMoves;
        duration_t         timeout      = duration_t::max();

        const std::string argsLc =
            Util::to_lower(args);

        std::vector< std::string > tokens;
        Util::split(argsLc, tokens);

        auto iter = std::find(tokens.begin(),
            tokens.end(), "searchmoves");

        if (iter != tokens.end())
        {
            const Position& master = *m_engine->m_master;

            const bool in_check =
                master.in_check(master.get_turn());

            for (auto mv = ++iter, end = tokens.end();
                 mv != end; ++mv)
            {
                const int32 move = str2move(
                    master, *mv);

                if (move == -1) break;

                AbortIfNot(MoveGen::validate_move(
                        master, move, in_check),
                    false);

                // Move has been verified as playable

                searchMoves.push_back(move);
            }
        }

        iter = std::find(
            tokens.begin() , tokens.end(), "mate");

        if (iter != tokens.end())
        {
            mateSearch = true;
            AbortIf(++iter == tokens.end(), false);
            AbortIfNot(Util::from_string(*iter,
                depth), false);
        }
        else
        {
            uint64 wtime     = maxU64;
            uint64 btime     = maxU64;
            uint64 winc      = 0;
            uint64 binc      = 0;
            uint32 movesToGo = max_moves;
            uint64 moveTime  = maxU64;

            iter = std::find(tokens.begin(), tokens.end(),
                             "wtime");

            if (iter != tokens.end())
            {
                AbortIf(++iter == tokens.end(), false);
                AbortIfNot(Util::from_string(*iter,
                    wtime), false);
            }

            iter = std::find(tokens.begin(), tokens.end(),
                             "btime");

            if (iter != tokens.end())
            {
                AbortIf(++iter == tokens.end(), false);
                AbortIfNot(Util::from_string(*iter,
                    btime), false);
            }

            iter = std::find(tokens.begin(), tokens.end(),
                             "winc");

            if (iter != tokens.end())
            {
                AbortIf(++iter == tokens.end(), false);
                AbortIfNot(Util::from_string(*iter,
                    winc), false);
            }

            iter = std::find(tokens.begin(), tokens.end(),
                             "binc");

            if (iter != tokens.end())
            {
                AbortIf(++iter == tokens.end(), false);
                AbortIfNot(Util::from_string(*iter,
                    binc), false);
            }

            iter = std::find(tokens.begin(), tokens.end(),
                             "movestogo");

            if (iter != tokens.end())
            {
                AbortIf(++iter == tokens.end(), false);
                AbortIfNot(Util::from_string(*iter,
                    movesToGo), false);
            }

            iter = std::find(tokens.begin(), tokens.end(),
                             "depth");

            if (iter != tokens.end())
            {
                AbortIf(++iter == tokens.end(), false);
                AbortIfNot(Util::from_string(*iter,
                    depth), false);
            }

            iter = std::find(tokens.begin(), tokens.end(),
                             "nodes");

            if (iter != tokens.end())
            {
                AbortIf(++iter == tokens.end(), false);
                AbortIfNot(Util::from_string(*iter,
                    nodeLimit), false);
            }

            iter = std::find(tokens.begin(), tokens.end(),
                             "movetime");

            if (iter != tokens.end())
            {
                AbortIf(++iter == tokens.end(), false);
                AbortIfNot(Util::from_string(*iter,
                    moveTime), false);
            }

            // If none of movetime, nodes, or depth was specified,
            // figure out how much time we have. The logic is pretty
            // simple; we divide the amount of time we have left in
            // the current time control by the number of moves left;
            // if it's sudden death, we look at how much time our
            // opponent has. If we've more time, take the difference
            // and think for that long. Otherise, think for only 1
            // second, and compensate by pondering
            
            if (depth     == max_moves &&
                nodeLimit == maxU64    &&
                moveTime  == maxU64)
            {
                const uint64 myTime =
                    m_engine->m_master->get_turn() == player_t::white ? 
                        wtime : btime;

                const uint64 theirTime = 
                    m_engine->m_master->get_turn() == player_t::white ? 
                        btime : wtime;

                if (movesToGo == max_moves ||
                    movesToGo == 0)
                    moveTime = 1000;  // in milliseconds
                else
                    moveTime = myTime / movesToGo;

                if (theirTime < myTime)
                {
                    moveTime = std::max(myTime-theirTime,
                        moveTime);
                }

                timeout = std::chrono::milliseconds(
                    moveTime);
            }
        }

        // Now initialize and run the search:

        AbortIfNot(m_engine->m_search->init(
            m_engine->m_master, searchMoves ),
                false);

        m_engine->m_search->run(
            depth, timeout, nodeLimit, mateSearch );

        // Report the best move found and exit:

        const auto pv =
            m_engine->m_search->get_pv(0);

        AbortIf(pv.empty(), false);

        (*m_stream)
            << "bestmove "<< formatCoordinate(pv[0])
            << std::endl;

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
                const int32 move = str2move(
                    master, *mv);

                AbortIf( move == -1, false );

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

    bool UCI::cmd_setoption (const std::string& args)
    {
        const std::string argsLc = Util::to_lower(args);

        std::vector< std::string > tokens;
        Util::split(argsLc, tokens);

        AbortIf(tokens.size() < 2, false);

        AbortIf(Util::trim(tokens[0]) != "name", false);

        const size_t nameStart  =
            argsLc.find("name" ) + 4;
        const size_t valueStart =
            argsLc.find("value");

        AbortIfNot(nameStart < args.size(), false );

        const size_t len =
            valueStart == std::string::npos ?
                valueStart : (valueStart-nameStart);

        const std::string name =
            Util::trim( argsLc.substr(nameStart, len) );

        auto iter = m_options.find(name);

        AbortIf( iter == m_options.end(), false );

        if (valueStart != std::string::npos)
        {
            AbortIfNot(valueStart+5 < args.size(),
                false);

            const std::string value =
                Util::trim( args.substr(valueStart+5) );

            AbortIfNot(iter->second->update(value),
                false);
        }
        else // this is a button (contains no arguments)
        {
            AbortIfNot(iter->second->update("---"),
                false);
        }

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

        stream  <<  "id name Anonymous\n";
        stream  <<  "id author Jason L. Fernandez\n";

        for (const auto& option : m_options)
        {
            stream << option.second->print() << "\n";
        }

        stream  <<  "uciok\n" << std::flush;

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

        AbortIfNot(cmd->install("go", std::bind(
            &UCI::cmd_go        , std::ref(*this), std::placeholders::_1)),
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

        AbortIfNot(cmd->install("setoption", std::bind(
            &UCI::cmd_setoption , std::ref(*this), std::placeholders::_1)),
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
            auto opt = std::make_shared<Spin>("multipv",  // Name
                                              1,          // Default
                                              1,          // Min
                                              max_moves); // Max
            AbortIfNot(opt, false);

            opt->m_updater = std::bind(&Search4::setNumberOfLines,
                                       std::ref(*m_engine->m_search),
                                       std::placeholders::_1);

            m_options["multipv"] = opt;
        }

        return true;
    }

    int32 UCI::str2move(const Position& pos,
                        const std::string& str) const
    {
        if (str.size() < 4) return -1;

        const std::string from_s = str.substr(0, 2);
        const std::string to_s   = str.substr(2, 2);

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

        if (from == square_t::BAD_SQUARE ||
              to == square_t::BAD_SQUARE)
            return -1;

        const piece_t moved  =  pos.piece_on( from );

        const bool is_ep = std::abs(from-to) % 8 != 0
            && moved == piece_t::pawn
            && pos.piece_on(to) == piece_t::empty;

        const piece_t captured =
            is_ep ?  piece_t::pawn : pos.piece_on(to);

        piece_t promote = piece_t::empty;
        if (str.size() > 4)
        {
            if ( moved != piece_t::pawn )
                return -1;

            switch (str.at(4))
            {
                case 'q': promote = piece_t::queen;  break;
                case 'n': promote = piece_t::knight; break;
                case 'b': promote = piece_t::bishop; break;
                case 'r': promote = piece_t::rook;   break;
                default:
                    Abort(-1, "promotion piece: '%c'",
                          str.at(4));
            }
        }

        const int32 move = pack_move(
            captured, from, moved, promote, to);

        return move;
    }
}
