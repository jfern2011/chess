#include "util/bit_tools.h"
#include "Position4.h"
#include "util/str_util.h"
#include "Verbosity.h"

namespace Chess
{
    constexpr char Position::init_fen[];

    /**
     * Perform a byte-wise comparison between this object
     * and another
     *
     * @param[in] rhs The object to compare against
     *
     * @return True if this EnPassant is the same as \a
     *         rhs
     */
    bool Position::EnPassant::operator==(const EnPassant& rhs)
        const
    {
        return src[0] == rhs.src[0] &&
               src[1] == rhs.src[1] &&
               target == rhs.target;
    }

    /**
     * Set all members to their defaults
     */
    void Position::EnPassant::clear()
    {
        target = src[0] = src[1] = square_t::BAD_SQUARE;
    }

    /**
     * Perform a byte-wise comparison between this object
     * and another
     *
     * @param[in] rhs The object to compare against
     *
     * @return True if this HashInput is the same as \a
     *         rhs
     */
    bool Position::HashInput::operator==(
        const HashInput& rhs) const
    {
        bool same = true;

        for (int i = 0; i < 2; i++)
        {
            same = same &&
                castle_rights[player_t::white][i]
                    == rhs.castle_rights[player_t::white][i];
            same = same && 
                castle_rights[player_t::black][i]
                    == rhs.castle_rights[player_t::black][i];
        }

        for (int i = 0; i < 8; i++)
        {
            same = same &&
                en_passant[i] == rhs.en_passant[i];
        }

        for (int i = 0; i < 6; i++)
        {
            for (int j = 0; j < 64; j++)
            {
                same = same && piece[player_t::black][i][j] ==
                    rhs.piece[player_t::black][i][j];

                same = same && piece[player_t::white][i][j] ==
                    rhs.piece[player_t::white][i][j];
            }
        }

        same = same &&
            to_move == rhs.to_move;

        return same;
    }

    /**
     * Clear all entries
     */
    void Position::HashInput::clear()
    {
        for (int i = 0; i < 2; i++)
        {
            castle_rights[player_t::white][i] = 0;
            castle_rights[player_t::black][i] = 0;
        }

        for (int i = 0; i < 8; i++)
            en_passant[i] = 0;

        for (int i = 0; i < 6; i++)
        {
            for (int j = 0; j < 64; j++)
            {
                piece[player_t::black][i][j] = 0;
                piece[player_t::white][i][j] = 0;
            }
        }

        to_move = 0;
    }

    /**
     * Dump the set of random numbers to standard output
     */
    void Position::HashInput::print()
    {
        for (int i = 0; i < 2; i++)
        {
            std::printf("castle_rights[white][%d]  = 0x%" PRIx64 "\n",
                i, castle_rights[player_t::white][i]);
            std::printf("castle_rights[black][%d]  = 0x%" PRIx64 "\n",
                i, castle_rights[player_t::black][i]);
        }

        for (int i = 0; i < 8; i++)
        {
            std::printf("en_passant[%d]            = 0x%" PRIx64 "\n",
                i, en_passant[i]);
        }

        for (int i = 0; i < 6; i++)
        {
            for (int j = 0; j < 64; j++)
            {
                std::printf("piece[black][%d][%2d]      = 0x%" PRIx64 "\n",
                    i, j, piece[player_t::black][i][j]);
                std::printf("piece[white][%d][%2d]      = 0x%" PRIx64 "\n",
                    i, j, piece[player_t::white][i][j]);
            }
        }

        std::printf("to_move                  = 0x%" PRIx64 "\n",
            to_move);

        std::fflush(stdout);
    }

    /**
     * Constructor
     *
     * @param[in] stream The stream to write log messages to
     * @param[in] fen    An initial FEN position
     */
    Position::Position(Handle<std::ostream> stream,
                       const std::string& fen)
        : _output(new OutputSource("Position", stream))
    {
        _is_init = reset(fen);
    }

    /**
     * Destructor
     */
    Position::~Position()
    {
    }

    /**
     * Compare this Position with another in a byte-wise sense
     *
     * @param[in] rhs The Position to compare against
     *
     * @return True if they are identical, false otherwise
     */
    bool Position::operator==(const Position& rhs) const
    {
        bool same = true;

        for (int j = 0; j < 2; j++)
        {
            same = same
                && _bishops[j]  == rhs._bishops[j]
                && _kings[j]    == rhs._kings[j]
                && _king_sq[j]  == rhs._king_sq[j]
                && _knights[j]  == rhs._knights[j]
                && _occupied[j] == rhs._occupied[j]
                && _pawns[j]    == rhs._pawns[j]
                && _queens[j]   == rhs._queens[j]
                && _rooks[j]    == rhs._rooks[j];
        }

        for (uint32 i = 0; i < max_ply; i++)
        {
            same = same
                && _ep_info[i]             == rhs._ep_info[i]
                && _half_move[i]           == rhs._half_move[i]
                && _castle_rights[i][0]    == rhs._castle_rights[i][0]
                && _castle_rights[i][1]    == rhs._castle_rights[i][1]
                && _last_halfmove_reset[i] == rhs._last_halfmove_reset[i]
                && _save_hash[i]           == rhs._save_hash[i];
        }

        same = same
            && _full_move  == rhs._full_move
            && _hash_input == rhs._hash_input
            && _is_init    == rhs._is_init
            && _material[player_t::white] 
                == rhs._material[player_t::white]
            && _material[player_t::black]
                == rhs._material[player_t::black]
            && _to_move    == rhs._to_move
            && _ply        == rhs._ply;

        for (int i = 0; i < 64; i++)
        {
            same = same && _pieces[i] ==
                rhs._pieces[i];
        }

        return same;
    }

    /**
     * Compare this Position with another at a given ply
     *
     * @note Certain data structures are indexed by ply; this performs
     *       comparisons such that if two positions are byte-wise
     *       equal at a given ply, then they are considered equal, and
     *       this method returns true
     *
     * @param[in] rhs The Position to compare against
     * @param[in] ply The ply at which to perform the comparison
     *
     * @return True if they are the same
     */
    bool Position::equals(const Position& rhs, int ply) const
    {
        bool same = true;

        for (int j = 0; j < 2; j++)
        {
            same = same
                && _bishops[j]  == rhs._bishops[j]
                && _kings[j]    == rhs._kings[j]
                && _king_sq[j]  == rhs._king_sq[j]
                && _knights[j]  == rhs._knights[j]
                && _occupied[j] == rhs._occupied[j]
                && _pawns[j]    == rhs._pawns[j]
                && _queens[j]   == rhs._queens[j]
                && _rooks[j]    == rhs._rooks[j];
        }

        same = same
            && _ep_info[_ply]          == rhs._ep_info[_ply]
            && _half_move[_ply]        == rhs._half_move[_ply]
            && _castle_rights[_ply][0] == rhs._castle_rights[_ply][0]
            && _castle_rights[_ply][1] == rhs._castle_rights[_ply][1]
            && _save_hash[_ply]        == rhs._save_hash[_ply];

        same = same
            && _full_move  == rhs._full_move
            && _hash_input == rhs._hash_input
            && _is_init    == rhs._is_init
            && _material[player_t::white]
                == rhs._material[player_t::white]
            && _material[player_t::black]
                == rhs._material[player_t::black]
            && _to_move    == rhs._to_move
            && _ply        == rhs._ply;

        for (int i = 0; i < 64; i++)
        {
            same = same && _pieces[i] ==
                rhs._pieces[i];
        }   

        return same;
    }

    /**
     * Generates a new hash signature for this position. This should
     * be called for every reset()
     *
     * @param[in] max Upper bound on each random number
     */
    void Position::generate_hash(uint64 max)
    {
        /*
         * Generate pseudo-random numbers used for updating the hash
         * keys
         */

        for (int i = 0; i < 2; i++)
        {
            _hash_input.castle_rights[player_t::black][i] = rand64(max);
            _hash_input.castle_rights[player_t::white][i] = rand64(max);
        }

        for (int i = 0; i < 8; i++)
            _hash_input.en_passant[i] = rand64(max);

        for (int i = 0; i < 6; i++)
        {
            for (int j = 0; j < 64; j++)
            {
                _hash_input.piece[player_t::black][i][j]  = rand64(max);
                _hash_input.piece[player_t::white][i][j]  = rand64(max);
            }
        }

        _hash_input.to_move = rand64(max);

        /*
         * Compute the hash signature for this position
         */

        uint64& signature  = _save_hash[_ply];
        signature = 0;

        if (_ep_info[_ply].target != square_t::BAD_SQUARE)
            signature ^= _hash_input.en_passant[get_file(_ep_info[_ply].target)];

        if (_to_move == player_t::white)
            signature ^= _hash_input.to_move;

        if (_castle_rights[_ply][player_t::white] & castle_K)
            signature ^=
                _hash_input.castle_rights[player_t::white][castle_K_index];
        if (_castle_rights[_ply][player_t::white] & castle_Q)
            signature ^=
                _hash_input.castle_rights[player_t::white][castle_Q_index];
        if (_castle_rights[_ply][player_t::black] & castle_K)
            signature ^=
                _hash_input.castle_rights[player_t::black][castle_K_index];
        if (_castle_rights[_ply][player_t::black] & castle_Q)
            signature ^=
                _hash_input.castle_rights[player_t::black][castle_Q_index];

        auto& tables = DataTables::get();

        for (int i = 0; i < 64; i++)
        {
            if (_pieces[i] != piece_t::empty)
            {
                if (_occupied[player_t::black] & tables.set_mask[i])
                {
                    signature ^=
                        _hash_input.piece[ player_t::black ][ _pieces[i] ][i];
                }
                else
                {
                    signature ^=
                        _hash_input.piece[ player_t::white ][ _pieces[i] ][i];
                }
            }
        }
    }

    /**
     * Get the FEN representation of this position
     *
     * @return The FEN position, or an empty string if this position was
     *         not initialized
     */
    std::string Position::get_fen() const
    {
        int empty = 0; char empty_s[16];

        std::string fen = "";
        AbortIfNot( _is_init, fen );

        auto& tables = DataTables::get();

        for (register int i = 63; i >= 0; i--)
        {
            std::sprintf(empty_s, "%d", empty);
            if (_pieces[i] != piece_t::empty)
            {
                bool whitePiece =
                    static_cast<bool>(tables.set_mask[i]
                        & _occupied[player_t::white]);

                if (empty != 0)
                {
                    fen += empty_s;  empty = 0;
                }

                switch (_pieces[i])
                {
                case piece_t::pawn:
                    fen += (whitePiece ? "P" : "p");
                    break;
                case piece_t::knight:
                    fen += (whitePiece ? "N" : "n");
                    break;
                case piece_t::bishop:
                    fen += (whitePiece ? "B" : "b");
                    break;
                case piece_t::rook:
                    fen += (whitePiece ? "R" : "r");
                    break;
                case piece_t::queen:
                    fen += (whitePiece ? "Q" : "q");
                    break;
                default:
                    fen += (whitePiece ? "K" : "k");
                }
            }
            else
                empty++;

            // Start the next rank:
            if (i % 8 == 0)
            {
                if (empty != 0)
                {
                    std::sprintf(empty_s, "%d", empty);
                    empty = 0;
                    fen += empty_s; 
                }
                if (i != 0)
                    fen += "/";
            }
        }

        if (_to_move == player_t::white)
            fen += " w ";
        else
            fen += " b ";

        if (_castle_rights[_ply][player_t::white] & castle_K)
            fen += "K";
        if (_castle_rights[_ply][player_t::white] & castle_Q)
            fen += "Q";
        if (_castle_rights[_ply][player_t::black] & castle_K)
            fen += "k";
        if (_castle_rights[_ply][player_t::black] & castle_Q)
            fen += "q";

        if (_castle_rights[_ply][player_t::white] == 0 &&
            _castle_rights[_ply][player_t::black] == 0)
            fen += "-";

        fen += " ";

        if (_ep_info[_ply].target != square_t::BAD_SQUARE)
            fen += square_str[_ep_info[_ply].target];
        else
            fen += "-";

        char halfMove_s[8];
        char fullMove_s[8];

        std::sprintf(halfMove_s, "%d", _half_move[_ply]);
        std::sprintf(fullMove_s, "%d", _full_move);

        std::string space = " ";

        fen +=  space + std::string(halfMove_s) +
                space + fullMove_s;

        return fen;
    }

    /**
     * Get the full move number, i.e. from the FEN representation
     * of this position
     *
     * @return The move number
     */
    int Position::get_fullmove_number() const
    {
        return _full_move;
    }

    /**
     * Get the half-move clock at the specified ply
     *
     * @param[in] ply Get the half-move clock at this ply
     *
     * @return The half-move clock
     */
    int Position::halfmove_clock(int ply) const
    {
        return _half_move[ply];
    }

    /**
     * Return the half-move clock at the current ply
     *
     * @return The half-move clock
     */
    int Position::halfmove_clock() const
    {
        return _half_move[_ply];
    }

    /**
     * Get the 64-bit integers used to hash a position
     *
     * @return The hash inputs
     */
    auto Position::get_hash_inputs() const -> const HashInput& 
    {
        return _hash_input;
    }

    /**
     * Get the ply at which the halfmove was last reset
     * that is not later than the specified ply
     *
     * @param [in] ply  Get the last reset that occured
     *                  before this ply was reached
     *
     * @return The last halfmove reset
     */
    int Position::last_halfmove_reset(int ply) const
    {
        return _last_halfmove_reset[ply];
    }

    /**
     * Display the current position
     */
    void Position::print() const
    {
        int prev_rank = 8;
        uint64 one = 1;

        for (int sq = 63; sq >= -1; sq--)
        {
            if (get_rank(sq) != prev_rank)
            {
                _output->write("\n ---+---+---+---+---+---+---+--- \n");
                if (sq == -1) break;

                prev_rank = get_rank(sq);
            }

            if (_pieces[sq] != piece_t::empty)
            {
                char piece = ' ';
                switch (_pieces[sq])
                {
                    case piece_t::pawn:
                        piece = 'P'; break;
                    case piece_t::rook:
                        piece = 'R'; break;
                    case piece_t::knight:
                        piece = 'N'; break;
                    case piece_t::bishop:
                        piece = 'B'; break;
                    case piece_t::queen:
                        piece = 'Q'; break;
                    case piece_t::king:
                        piece = 'K'; break;
                    default:
                        break;
                }

                if (_occupied[player_t::black] & (one << sq))
                    piece = Util::to_lower(piece);

                _output->write("| %c ", piece);
            }
            else
                _output->write("|   ");

            if (sq % 8 == 0)
                _output->write("|");
        }
    }

    /**
     * Reset to the position encoded in Forsyth–Edwards Notation (FEN)
     *
     * @param [in] fenPos A FEN position
     *
     * @return True if the new FEN position was successfully processed
     */
    bool Position::reset(const std::string& fenPos)
    {
        Position backup(*this);
        int square = 63;

        const std::string fen = Util::trim( fenPos );

        // Clear member fields. Note this sets _ply = 0
        set_default();

        std::vector<std::string> tokens; Util::split(fen, tokens, "/");

        if (tokens.size() != 8)
        {
            if (verbosity >= Verbosity::terse && _output)
            {
                _output->write("Invalid FEN (wrong number of ranks): '%s'\n",
                    fen.c_str());
            }

            *this = backup;
            return false;
        }

        /*
         * Make sure the pieces/squares in each rank add
         * up to 8:
         */
        for (size_t i = 0; i < tokens.size(); i++)
        {
            int sum = 0;
            std::string rank = tokens[i];
            for (size_t j = 0; j < rank.size(); j++)
            {
                if (is_piece(rank[j]))
                    sum += 1;
                else if (std::isdigit(rank[j]))
                {
                    int val = 0;
                    if (!Util::from_string(std::string(&rank[j],1),val))
                    {
                        *this = backup;
                        return false;
                    }

                    sum += val;
                }
                else if (std::isspace(rank[j])) break;
                else
                {
                    if (verbosity >= Verbosity::terse && _output)
                    {
                        _output->write(
                            "Invalid FEN (unexpected character '%c'): %s\n",
                            rank[j], fen.c_str());
                    }
                    
                    *this = backup;
                    return false;
                }
            }

            if (sum != 8)
            {
                if (verbosity >= Verbosity::terse && _output)
                {
                    _output->write("Invalid FEN (pieces/squares in rank %d "
                                   "is wrong): %s\n",
                        8-i, fen.c_str());
                }

                *this = backup;
                return false;
            }
        }

        for (size_t i = 0; i < 8; i++)
        {
            for (size_t j = 0; j < tokens[i].size(); j++)
            {
                char c = tokens[i][j];
                if (is_piece(c))
                {
                    uint64 mask = Util::get_bit<uint64>(square);
                    _pieces[square] = piece2enum(c);

                    if (Util::to_lower(c) == c)
                        _occupied[player_t::black] |= mask;
                    else
                        _occupied[player_t::white] |= mask;
                    square -= 1;

                    switch (c)
                    {
                        case 'p':
                            _pawns  [player_t::black] |= mask; break;
                        case 'P':
                            _pawns  [player_t::white] |= mask; break;
                        case 'r':
                            _rooks  [player_t::black] |= mask; break;
                        case 'R':
                            _rooks  [player_t::white] |= mask; break;
                        case 'n':
                            _knights[player_t::black] |= mask; break;
                        case 'N':
                            _knights[player_t::white] |= mask; break;
                        case 'b':
                            _bishops[player_t::black] |= mask; break;
                        case 'B':
                            _bishops[player_t::white] |= mask; break;
                        case 'q':
                            _queens [player_t::black] |= mask; break;
                        case 'Q':
                            _queens [player_t::white] |= mask; break;
                        case 'k':
                            _kings[player_t::black]   |= mask;
                            _king_sq[player_t::black]  =
                                static_cast<square_t>(Util::get_lsb(mask));
                            break;
                        case 'K':
                            _kings[player_t::white]   |= mask;
                            _king_sq[player_t::white]  =
                                static_cast<square_t>(Util::get_lsb(mask));
                            break;
                        default:
                            *this = backup;
                            return false;
                    }
                }
                else
                {
                    /*
                     * Throws invalid_argument exception on error
                     */
                    square -= std::stoi(std::string(&c, 1));
                }

                if ((square < 0 && i != 7) || square < -1)
                {
                    if (verbosity >= Verbosity::terse && _output)
                    {
                        _output->write("Invalid FEN (more than 64 squares given): '%s'\n",
                            fen.c_str());
                    }
                    
                    *this = backup;
                    return false;
                }
                else if (square < 0)
                {
                    // Move on to whose turn:
                    break;
                }
            }
        }

        std::vector<std::string> posn_info;
        Util::split(tokens.back(), posn_info, " ");

        _half_move[_ply] = 0;
        _full_move       = 1;

        switch (posn_info.size())
        {
            default:
                // Ignore anything beyond the 6th token instead of
                // returning an error
            case 6:
                if (!Util::from_string(posn_info[5], _full_move))
                {
                    if (verbosity >= Verbosity::terse && _output)
                    {
                        _output->write("Invalid FEN (fullmove number): '%s'\n",
                            fen.c_str());
                    }

                    *this = backup;
                    return false;
                }
            case 5:
                if (!Util::from_string(posn_info[4], _half_move[_ply]))
                {
                    if (verbosity >= Verbosity::terse && _output)
                    {
                        _output->write("Invalid FEN (halfmove clock): '%s'\n",
                            fen.c_str());
                    }

                    *this = backup;
                    return false;
                }
            case 4:
                _ep_info[_ply].target = square_t::BAD_SQUARE;

                if (posn_info[3] != "-")
                {
                    for (int i = 0; i < 64; i++)
                    {
                        if (Util::to_lower(posn_info[3]) == square_str[i])
                        {
                            _ep_info[_ply].target =
                                static_cast<square_t>(i);
                            break;
                        }
                    }

                    if (_ep_info[_ply].target == square_t::BAD_SQUARE)
                    {
                        if (verbosity >= Verbosity::terse && _output)
                        {
                            _output->write("Invalid FEN (en passant square): '%s'\n",
                                fen.c_str());
                        }
                        
                        *this = backup;
                        return false;
                    }
                }
            case 3:
                for (size_t i = 0; i < posn_info[2].size(); i++)
                {
                    switch (posn_info[2][i])
                    {
                        case 'K':
                            _castle_rights[_ply][player_t::white] |= castle_K;
                            break;
                        case 'Q':
                            _castle_rights[_ply][player_t::white] |= castle_Q;
                            break;
                        case 'k':
                            _castle_rights[_ply][player_t::black] |= castle_K;
                            break;
                        case 'q':
                            _castle_rights[_ply][player_t::black] |= castle_Q;
                            break;
                        case '-':
                            if (posn_info[2].size() == 1)
                                continue;
                        default:
                            if (verbosity >= Verbosity::terse && _output)
                            {
                                _output->write("Invalid FEN (castling rights): '%s'\n",
                                    fen.c_str());
                            }
                            
                            *this = backup;
                            return (false);
                    }
                }
            case 2:
                if (posn_info[1] != "w" && posn_info[1] != "b")
                {
                    if (verbosity >= Verbosity::terse && _output)
                    {
                        _output->write("Invalid FEN (invalid color): '%s'\n",
                            fen.c_str());
                    }
                    
                    *this = backup;
                    return false;
                }
                _to_move =
                    posn_info[1] == "w" ? player_t::white : player_t::black;
                break;
            case 1:
                if (verbosity >= Verbosity::terse && _output)
                {
                    _output->write("Invalid FEN (unspecified color): '%s'\n",
                        fen.c_str());
                }
                
                *this = backup;
                    return false;
        }

        /*
         * Set the squares from which we can capture via en passant:
         */
        uint64 src = 0;

        auto& tables = DataTables::get();
        
        if (_ep_info[_ply].target != square_t::BAD_SQUARE)
        {
            const square_t victim =
                tables.minus_8[ _to_move ][_ep_info[_ply].target];

            src = _pawns[_to_move] & tables.rank_adjacent[victim];

            if (src & (tables.set_mask[victim+1]))
                _ep_info[_ply].src[0] =
                    static_cast<square_t>(victim+1);

            if (src & (tables.set_mask[victim-1]))
                _ep_info[_ply].src[1] =
                    static_cast<square_t>(victim-1);
        }

        /*
         * Validate the new position. If it violates any of the rules
         * of chess, reject it
         */
        if (!_validate(fen))
        {
            *this = backup;
                return(false);
        }

        /*
         * Compute the material balance. This avoids
         * having to do so during static eval
         */
        _material[player_t::white] =
            Util::bit_count(_pawns[player_t::white])   * pawn_value   +
            Util::bit_count(_knights[player_t::white]) * knight_value + 
            Util::bit_count(_bishops[player_t::white]) * bishop_value + 
            Util::bit_count(_rooks[player_t::white])   * rook_value   +
            Util::bit_count(_queens[player_t::white])  * queen_value;

        _material[player_t::black] =
            Util::bit_count(_pawns[player_t::black])   * pawn_value   + 
            Util::bit_count(_knights[player_t::black]) * knight_value + 
            Util::bit_count(_bishops[player_t::black]) * bishop_value + 
            Util::bit_count(_rooks[player_t::black])   * rook_value   +
            Util::bit_count(_queens[player_t::black])  * queen_value;

        /*
         * Generate a hash signature for this position
         */
        generate_hash();

        _is_init = true;
        return true;
    }

    /**
     * Set default (uninitialized) values for this position
     */
    void Position::set_default()
    {
        for (int i = 0; i < 64; i++ ) _pieces[i] = piece_t::empty;

        for (int i = 0; i < 2; i++)
        {
            _bishops[i]  = 0;
            _kings[i]    = 0;
            _king_sq[i]  = square_t::BAD_SQUARE;
            _knights[i]  = 0;
            _occupied[i] = 0;
            _pawns[i]    = 0;
            _queens[i]   = 0;
            _rooks[i]    = 0;
        }

        _full_move = -1;

        _material[player_t::white] = 0;
        _material[player_t::black] = 0;

        _to_move  = player_t::white;

        for (uint32 i = 0; i < max_ply; i++)
        {
            _last_halfmove_reset[i] = 0;

            _ep_info[i].clear();

            _castle_rights[i][player_t::black] = 0;
            _castle_rights[i][player_t::white] = 0;

            _half_move[i]  = -1;
            _save_hash[i]  =  0;
        }

        _hash_input.clear();

        _is_init = false;
        _ply     = 0;
    }

    /**
     * Validate the position according to the following rules:
     *
     * 1. No pawns on the 1st or 8th ranks
     * 2. Only two kings on board
     * 3. Side to move cannot capture a king
     * 4. Castling rights make sense (e.g. king is not on its home
     *    square => cannot castle)
     * 5. En passant target makes sense (e.g. there must be a pawn
     *    that has advanced two squares)
     * 6. Maximum 8 pawns per side
     * 7. At most 10 of any piece, per side
     *
     * @param[in] fen The FEN representation of the position to
     *                validate
     *
     * @return True if this is a valid Position
     */
    bool Position::_validate(const std::string& fen) const
    {
        auto& tables = DataTables::get();

        // Rule 1:
        if ((_pawns[player_t::black] | _pawns[player_t::white]) & (rank_1 | rank_8))
        {
            if (verbosity >= Verbosity::terse && _output)
            {
                _output->write("Invalid FEN (pawn(s) on back rank): '%s'\n",
                    fen.c_str());
            }
            
            return false;
        }

        // Rule 2:
        if ((Util::bit_count(_kings[player_t::white]) != 1) ||
            (Util::bit_count(_kings[player_t::black]) != 1))
        {
            if (verbosity >= Verbosity::terse && _output)
            {
                _output->write("Invalid FEN (wrong number of kings): '%s'\n",
                    fen.c_str());
            }
            
            return false;
        }

        // Rule 3:
        if (in_check(flip(_to_move)))
        {
            if (verbosity >= Verbosity::terse && _output)
            {
                _output->write("Invalid FEN (king can be captured): '%s'\n",
                    fen.c_str());
            }
            
            return false;
        }

        // Rule 4:
        int castle_mask = castle_K | castle_Q;

        if (!(_kings[player_t::white] & Util::get_bit< uint64 >(square_t::E1)))
        {
            if (_castle_rights[_ply][player_t::white] & castle_mask)
            {
                if (verbosity >= Verbosity::terse && _output)
                {
                    _output->write("Invalid FEN (player_t::white may not castle): '%s'\n",
                        fen.c_str());
                }
                
                return false;
            }
        }
        else
        {
            if ((_castle_rights[_ply][player_t::white] & castle_K)
                 && !(_rooks[player_t::white] & Util::get_bit<uint64>(square_t::H1)))
            {
                if (verbosity >= Verbosity::terse && _output)
                {
                    _output->write("Invalid FEN (player_t::white may not castle short): '%s'\n",
                        fen.c_str());
                }
                
                return false; 
            }

            if ((_castle_rights[_ply][player_t::white] & castle_Q)
                 && !(_rooks[player_t::white] & Util::get_bit<uint64>(square_t::A1)))
            {
                if (verbosity >= Verbosity::terse && _output)
                {
                    _output->write("Invalid FEN (player_t::white may not castle long): '%s'\n",
                        fen.c_str());
                }
                
                return false; 
            }
        }

        if (!(_kings[player_t::black] & Util::get_bit< uint64 >(square_t::E8)))
        {
            if (_castle_rights[_ply][player_t::black] & castle_mask)
            {
                if (verbosity >= Verbosity::terse && _output)
                {
                    _output->write("Invalid FEN (player_t::black may not castle): '%s'\n",
                        fen.c_str());
                }
                
                return false;
            }
        }
        else
        {
            if ((_castle_rights[_ply][player_t::black] & castle_K)
                 && !(_rooks[player_t::black] & Util::get_bit<uint64>(square_t::H8)))
            {
                if (verbosity >= Verbosity::terse && _output)
                {
                    _output->write("Invalid FEN (player_t::black may not castle short): '%s'\n",
                        fen.c_str());
                }
                
                return false; 
            }

            if ((_castle_rights[_ply][player_t::black] & castle_Q)
                 && !(_rooks[player_t::black] & Util::get_bit<uint64>(square_t::A8)))
            {
                if (verbosity >= Verbosity::terse && _output)
                {
                    _output->write("Invalid FEN (player_t::black may not castle long): '%s'\n",
                        fen.c_str());
                }
                
                return false; 
            }
        }

        // Rule 5:
        if (_ep_info[_ply].target != square_t::BAD_SQUARE)
        {
            const square_t target
                = _ep_info[_ply].target;

            if (!(tables.set_mask[target] & tables._3rd_rank[flip(_to_move)]) ||
                !(_pawns[flip(_to_move)]
                    & tables.set_mask[tables.minus_8[_to_move][target]]))
            {
                if (verbosity >= Verbosity::terse && _output)
                {
                    _output->write("Invalid FEN (En passant square): '%s'\n",
                        fen.c_str());
                }
                
                return false;
            }
        }

        // Rule 6:
        if (Util::bit_count<uint64>(_pawns[player_t::white]) > 8 || 
            Util::bit_count<uint64>(_pawns[player_t::black]) > 8)
        {
            if (verbosity >= Verbosity::terse && _output)
            {
                _output->write("Invalid FEN (Max 8 pawns allowed per side): '%s'\n",
                    fen.c_str());
            }
            
            return false;
        }

        // Rule 7:
        if (Util::bit_count<uint64>(_knights[player_t::white]) > 10 ||
            Util::bit_count<uint64>(_knights[player_t::black]) > 10)
        {
            if (verbosity >= Verbosity::terse && _output)
            {
                _output->write("Invalid FEN (Max 10 knights allowed per side): '%s'\n",
                    fen.c_str());
            }
            
            return false;
        }

        if (Util::bit_count<uint64>(_rooks[player_t::white]) > 10 ||
            Util::bit_count<uint64>(_rooks[player_t::black]) > 10)
        {
            if (verbosity >= Verbosity::terse && _output)
            {
                _output->write("Invalid FEN (Max 10 rooks allowed per side): '%s'\n",
                    fen.c_str());
            }
            
            return false;
        }

        if (Util::bit_count<uint64>(_queens[player_t::white]) > 10 ||
            Util::bit_count<uint64>(_queens[player_t::black]) > 10)
        {
            if (verbosity >= Verbosity::terse && _output)
            {
                _output->write("Invalid FEN (Max 10 queens allowed per side): '%s'\n",
                    fen.c_str());
            }
            
            return false;
        }

        if (Util::bit_count<uint64>(_bishops[player_t::white]) > 10 ||
            Util::bit_count<uint64>(_bishops[player_t::black]) > 10)
        {
            if (verbosity >= Verbosity::terse && _output)
            {
                _output->write("Invalid FEN (Max 10 bishops allowed per side): '%s'\n",
                    fen.c_str());
            }
            
            return false;
        }
        
        return true;
    }
}
