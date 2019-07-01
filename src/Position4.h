#ifndef __POSITION_H__
#define __POSITION_H__

#include "chess_util4.h"
#include "DataTables4.h"
#include "OutputSource.h"

namespace Chess
{
    /**
     * @class Position
     *
     * Represents a chess position
     */
    class Position final
    {

    public:

        /**
         * Structure containing en passant information for
         * a position
         */
        struct EnPassant
        {
            /**
             * The en passant target (i.e. "to") square
             */
            square_t target;

            /**
             * The origin square(s) from which a player may
             * capture en passant
             */
            BUFFER(square_t, src, 2);

            bool operator==(const EnPassant& rhs) const;

            void clear();
        };

        /**
         * Structure that contains 781 64-bit integers used
         * create a hash signature
         */
        struct HashInput
        {
            /**
             * 2 integers for castling rights for each
             * player (4 total)
             */
            BUFFER(uint64, castle_rights, 2, 2);

            /**
             * 8 integers for the en passant square (1 per
             * file)
             */
            BUFFER(uint64, en_passant, 8);

            /**
             * 1 integer for each piece on each square for
             * both sides (768 total)
             */
            BUFFER(uint64, piece, 2, 6, 64);

            /**
             * 1 integer for whose turn it is
             */
            uint64 to_move;

            bool operator==(
                const HashInput& rhs) const;

            void clear();

            void print();
        };

        /**
         * The default starting position
         */
        static constexpr char init_fen[] =
            "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

    public:

        Position(Handle<std::ostream> stream,
                 const std::string& fen= init_fen);

        Position(const Position& other);

        ~Position();

        Position& operator=( const Position& rhs );

        bool operator==(const Position& rhs) const;

        template <piece_t type>
        uint64 attacks_from(
            square_t square, player_t to_move= player_t::both) const;

        template <piece_t type>
        uint64 attacks_from(square_t square, uint64 occupied,
            player_t to_move = player_t::both) const;

        uint64 attacks_to  (square_t square, player_t to_move) const;

        bool can_castle_long( player_t to_move) const;

        bool can_castle_short(player_t to_move) const;

        const EnPassant& ep_data() const;

        bool equals(const Position& p, int ply) const;

        void generate_hash(uint64 max =
            std::numeric_limits<uint64>::max());

        template <piece_t type>
        uint64 get_bitboard( player_t to_move ) const;

        uint64 get_discover_ready(player_t to_move) const;

        std::string get_fen()         const;

        int get_fullmove_number()     const;

        const HashInput& get_hash_inputs() const;

        uint64 get_hash_key(int  ply) const;

        uint64 get_hash_key()         const;

        square_t get_king_square (player_t to_move) const;

        int get_material(player_t to_move)          const;

        template <piece_t type>
        int get_mobility(square_t square) const;

        uint64 get_occupied( player_t to_move ) const;

        uint64 get_pinned_pieces (player_t to_move) const;

        player_t get_turn() const;

        int halfmove_clock(int ply) const;

        int halfmove_clock()        const;

        bool in_check( player_t to_move ) const;

        direction_t is_pinned(
            square_t square, player_t to_move ) const;

        int last_halfmove_reset(int ply ) const;

        bool make_move(int32 move);

        piece_t piece_on(square_t square) const;

        void print() const;

        bool reset(const std::string& fen = init_fen);

        void set_default();

        bool under_attack(
            square_t square, player_t to_move ) const;

        bool unmake_move (int32 move);

    private:

        /* Methods that compute sliding piece attacks  */

        uint64 _attacks_from_diag (square_t square,
            uint64 occupied) const;

        uint64 _attacks_from_queen(square_t square,
            uint64 occupied) const;

        uint64 _attacks_from_rook (square_t square,
            uint64 occupied) const;

        /* Methods that compute sliding piece mobility */

        int _get_diag_mobility (square_t square, uint64 occupied) const;

        int _get_queen_mobility(square_t square, uint64 occupied) const;

        int _get_rook_mobility (square_t square, uint64 occupied) const;

        void _update_hash(int32 move);

        bool _validate(
            const std::string& fen) const;

        /**
         * Bitboards that give the locations of all bishops in the position
         */
        BUFFER(uint64, _bishops, 2);

        /**
         * Bitmasks describing the allowed castling abilities for each
         * side
         */
        BUFFER(uint8, _castle_rights, max_ply, 2);

        /**
         * A record of en passant squares, indexed by ply
         */
        BUFFER(EnPassant, _ep_info, max_ply);

        /**
         * The full-move number (see FEN notation). Since this is really
         * just informational, we don't bother incrementing it for
         * null moves
         */
        int32 _full_move;

        /**
         * The half-move clock (see FEN notation). Note that we do not
         * increment this for null moves
         */
        BUFFER(int32, _half_move, max_ply);

        /**
         * A set of 64-bit integers used to generate a Zobrist hash key
         * for this position
         */
        HashInput _hash_input;

        /**
         * Flag indicating this position is ready to use
         */
        bool _is_init;

        /**
         * Bitboards that give the locations of both kings
         */
        BUFFER(uint64, _kings, 2);

        /**
         * The square each king is on. This avoids having to \ref MSB the
         * \ref _kings bitboard to get an index
         */
        BUFFER(square_t, _king_sq, 2);

        /**
         * Bitboards that give the locations of all knights in this
         * position
         */
        BUFFER(uint64, _knights, 2);

        /**
         * The ply at which we last reset the halfmove clock
         */
        BUFFER(int, _last_halfmove_reset, max_ply);

        /**
         * The material balance, per side
         */
        BUFFER(uint16, _material, 2);

        /**
         *  Bitboards that give the locations of all squares occupied by
         *  both sides
         */
        BUFFER(uint64, _occupied, 2);

        /**
         *  The output stream on which to write log messages
         */
        Handle<OutputSource> _output;

        /**
         * Bitboards that give the locations of all pawns in
         * the position
         */
        BUFFER(uint64, _pawns, 2);

        /**
         * Tells us what piece is on each square
         */
        BUFFER(piece_t, _pieces, 64);

        /**
         * The current ply (resets for every search)
         */
        int _ply;

        /**
         * Bitboards that give the locations of all queens in this
         * position
         */
        BUFFER(uint64, _queens, 2);

        /**
         *  Bitboards that give the locations of all rooks in this
         *  position
         */
        BUFFER(uint64, _rooks, 2);

        /**
         * A record of all hash signatures, indexed by ply
         */
        BUFFER(uint64, _save_hash, max_ply);

        /**
         * The player whose turn it is to move
         */
        player_t _to_move;
    };

    /**
     * Generates the squares attacked by the given piece located at the
     * given square
     *
     * @tparam type  Which piece to generate attacked squares for
     *
     * @param[in] square   The square this piece is on
     * @param[in] to_move  Generate an attacks board for this side (for
     *                     pawn attacks)
     *
     * @return A bitboard specifying all squares attacked by this
     *         piece from \a square
     */
    template <piece_t type>
    inline uint64 Position::attacks_from(square_t square,
        player_t to_move) const
    {
        Abort(0);
    }

    /**
     * Generates the squares attacked by the given piece located at the
     * given square
     *
     * @tparam type  Which piece to generate attacked squares for
     *
     * @param[in] square   The square this piece is on
     * @param[in] occupied A custom occupied squares bitboard
     * @param[in] to_move  Generate an attacks board for this side (for
     *                     pawn attacks)
     *
     * @return A bitboard specifying all squares attacked by this
     *         piece from \a square
     */
    template <piece_t type>
    inline uint64 Position::attacks_from(square_t square,
        uint64 occupied, player_t to_move) const
    {
        Abort(0);
    }

#ifndef DOXYGEN_SKIP
    template <>
    inline uint64 Position::attacks_from< piece_t::rook >(square_t square,
        player_t) const
    {
        return _attacks_from_rook(square, _occupied[player_t::white] |
                                          _occupied[player_t::black]);
    }

    template <>
    inline uint64 Position::attacks_from<piece_t::knight>(square_t square,
        player_t) const
    {
        return DataTables::get().knight_attacks[square];
    }

    template <>
    inline uint64 Position::attacks_from<piece_t::bishop>(square_t square,
        player_t) const
    {
        return _attacks_from_diag(square, _occupied[player_t::white] |
                                          _occupied[player_t::black]);
    }

    template <>
    inline uint64 Position::attacks_from< piece_t::pawn >(square_t square,
        player_t to_move) const
    {
        return DataTables::get().pawn_attacks[to_move][square];
    }

    template <>
    inline uint64 Position::attacks_from< piece_t::king >(square_t square,
        player_t) const
    {
        return DataTables::get().king_attacks[square];
    }

    template <>
    inline uint64 Position::attacks_from<piece_t::queen >(square_t square,
        player_t) const
    {
        const uint64 occupied = _occupied[player_t::white] |
                                _occupied[player_t::black];

        return _attacks_from_rook(square, occupied) |
               _attacks_from_diag(square, occupied);
    }

    template <>
    inline uint64 Position::attacks_from< piece_t::rook >(square_t square,
        uint64 occupied, player_t) const
    {
        return _attacks_from_rook(square, occupied);
    }

    template <>
    inline uint64 Position::attacks_from<piece_t::knight>(square_t square,
        uint64, player_t) const
    {
        return attacks_from<piece_t::knight>(square);
    }

    template <>
    inline uint64 Position::attacks_from<piece_t::bishop>(square_t square,
        uint64 occupied, player_t) const
    {
        return _attacks_from_diag(square, occupied);
    }

    template <>
    inline uint64 Position::attacks_from< piece_t::pawn >(square_t square,
        uint64, player_t to_move) const
    {
        return attacks_from<piece_t::pawn>(square, to_move);
    }

    template <>
    inline uint64 Position::attacks_from< piece_t::king >(square_t square,
        uint64, player_t) const
    {
        return attacks_from<piece_t::king>(square);
    }

    template <>
    inline uint64 Position::attacks_from<piece_t::queen >(square_t square,
        uint64 occupied, player_t) const
    {
        return _attacks_from_rook(square, occupied) |
               _attacks_from_diag(square, occupied);
    }
#endif

    /**
     * Get a bitboard containing 1-bits for each square occupied by \a
     * to_move that has a piece attacking \a square
     *
     * @param[in] square  The square to examine
     * @param[in] to_move The side whose attackers to the given square
     *                    we want
     *
     * @return A bitboard of attackers for \a to_move
     */
    inline uint64 Position::attacks_to(square_t square,
        player_t to_move) const
    {
        uint64 out = 0;

        const uint64 occupied = _occupied[player_t::white] |
                                _occupied[player_t::black];

        const auto& tables = DataTables::get();

        out |= tables.pawn_attacks[flip(to_move)][square]
                & _pawns[to_move];
        
        out |= tables.knight_attacks[square]
                & _knights[to_move];

        out |= _attacks_from_rook(square, occupied)
                & ( _rooks[ to_move ] | _queens[to_move] );

        out |= _attacks_from_diag(square, occupied)
                & ( _bishops[to_move] | _queens[to_move] );

        out |= tables.king_attacks[square]
                & _kings[to_move];

        return out;
    }

    /**
     * Determine whether or not the given player has forfeited his ability
     * to castle long
     *
     * @param[in] to_move The player of interest
     *
     * @return True if he may castle
     */
    inline bool Position::can_castle_long(player_t to_move) const
    {
        return _castle_rights[_ply][to_move] & castle_Q;
    }

    /**
     * Determine whether or not the given player has forfeited his ability
     * to castle short
     *
     * @param[in] to_move The player of interest
     *
     * @return True if he may castle
     */
    inline bool Position::can_castle_short(player_t to_move) const
    {
        return _castle_rights[_ply][to_move] & castle_K;
    }

    /**
     * Get a reference to the en passant information at the current ply
     *
     * @return The en passant info
     */
    inline auto Position::ep_data() const -> const EnPassant&
    {
        return _ep_info[_ply];
    }

    /**
     * Get a bitboard representing all of a player's type of piece. For
     * example, to get all of white's bishops:
     *
     * @code
     * get_bitboard< piece_t::bishop >(player_t::white)
     * @endcode
     *
     * @tparam type Specifies the piece type
     *
     * @param[in] to_move Specifies whose pieces to get
     *
     * @return A bitboard with 1 bit set for each square containing the
     *         specified piece belonging to \a to_move
     */
    template <piece_t type>
    inline uint64 Position::get_bitboard(player_t to_move) const
    {
        return 0;
    }

#ifndef DOXYGEN_SKIP
    template <>
    inline uint64 Position::get_bitboard< piece_t::rook >(
        player_t to_move) const
    {
        return _rooks[to_move];
    }

    template <>
    inline uint64 Position::get_bitboard<piece_t::knight>(
        player_t to_move) const
    {
        return _knights[to_move];
    }

    template <>
    inline uint64 Position::get_bitboard<piece_t::bishop>(
        player_t to_move) const
    {
        return _bishops[to_move];
    }

    template <>
    inline uint64 Position::get_bitboard< piece_t::pawn >(
        player_t to_move) const
    {
        return _pawns[to_move];
    }

    template <>
    inline uint64 Position::get_bitboard< piece_t::king >(
        player_t to_move) const
    {
        return _kings[to_move];
    }

    template <>
    inline uint64 Position::get_bitboard<piece_t::queen >(
        player_t to_move) const
    {
        return _queens[to_move];
    }
#endif

    /**
     * Get a bitboard containing all pieces that, if moved, would uncover
     * check on \a to_move
     *
     * @param[in] to_move The side whose king is vulnerable
     *
     * @return A bitboard with bits set for each square whose occupant is
     *         ready to uncover check
     */
    inline uint64 Position::get_discover_ready(player_t to_move) const
    {
        const uint64 occupied = _occupied[player_t::black] |
                                _occupied[player_t::white];

        uint64 pinned =
            _attacks_from_queen(_king_sq[to_move], occupied)
                & _occupied[flip(to_move)];

        auto& tables = DataTables::get();

        for (uint64 temp = pinned; temp; )
        {
            const square_t sq =
                static_cast<square_t>(msb64(temp));

            switch (tables.directions[sq][_king_sq[to_move]])
            {
            case direction_t::along_rank:
                if (!(_attacks_from_rook( sq, occupied)
                        & tables.ranks64[sq]
                        & (_rooks[flip(to_move)] |
                            _queens[flip(to_move)])))
                    clear_bit64(sq, pinned);
                break;
            case direction_t::along_file:
                if (!(_attacks_from_rook( sq, occupied)
                        & tables.files64[sq]
                        & (_rooks[flip(to_move)] |
                            _queens[flip(to_move)])))
                    clear_bit64(sq, pinned);
                break;
            case direction_t::along_a1h8:
                if (!(_attacks_from_diag(sq, occupied)
                      & tables.a1h8_64[sq]
                      & (_bishops[flip(to_move)] |
                             _queens[flip(to_move)])))
                    clear_bit64(sq, pinned);
                break;
            case direction_t::along_h1a8:
                if (!(_attacks_from_diag(sq, occupied)
                      & tables.h1a8_64[sq]
                      & (_bishops[flip(to_move)] |
                             _queens[flip(to_move)])))
                    clear_bit64(sq, pinned);
            default:
                break;
            }

            clear_bit64(sq, temp);
        }

        return pinned;
    }

    /**
     * Get the 64-bit Zobrist key associated with the position
     * at the specified ply
     *
     * @param[in] ply Get the key at this ply
     *
     * @return The hash key
     */
    inline uint64 Position::get_hash_key(int ply) const
    {
        return _save_hash[ply];
    }

    /**
     * Get the 64-bit Zobrist key associated with the position
     * at the current ply
     *
     * @return The hash key
     */
    inline uint64 Position::get_hash_key() const
    {
        return _save_hash[_ply];
    }

    /**
     * Get the square that the king belonging to \a to_move is
     * currently on
     *
     * @param[in] to_move Get this guy's king
     *
     * @return  The king's location, which should be between 0
     *          and 63 inclusive
     */
    inline square_t Position::get_king_square(player_t to_move) const
    {
        return _king_sq[to_move];
    }

    /**
     * Get the material score for a player
     *
     * @param[in] to_move Get this player's material score
     *
     * @return The material score for \a to_move
     */
    inline int Position::get_material(player_t to_move) const
    {
        return  _material[to_move];
    }

    /**
     * Get the mobility of a given piece on the given \a square. This
     * is hashed to avoid computing it on the fly
     *
     * @param[in] square The piece's location
     *
     * @return The Hamming weight of \ref attacks_from(). For sliding
     *         pieces only, this may be positive
     */
    template <piece_t type>
    inline int Position::get_mobility(square_t square) const
    {
        return 0;
    }

#ifndef DOXYGEN_SKIP
    template <>
    inline int Position::get_mobility< piece_t::rook >(square_t square)
        const
    {
        return _get_rook_mobility(square,
                                  _occupied[player_t::white] |
                                  _occupied[player_t::black]);
    }

    template <>
    inline int Position::get_mobility<piece_t::bishop>(square_t square)
        const
    {
        return _get_diag_mobility(square,
                                  _occupied[player_t::white] |
                                  _occupied[player_t::black]);
    }

    template <>
    inline int Position::get_mobility<piece_t::queen >(square_t square)
        const
    {
        const uint64 occupied =   _occupied[player_t::white] |
                                  _occupied[player_t::black];

        return _get_diag_mobility(square, occupied) |
               _get_rook_mobility(square, occupied);
    }
#endif

    /**
     * Get a bitboard representing all squares occupied by \a to_move
     *
     * @param[in] to_move Get the squares occupied by this guy
     *
     * @return A bitboard with 1 bit set for each square containing a
     *         piece belonging to \a to_move
     */
    inline uint64 Position::get_occupied(player_t to_move) const
    {
        return _occupied[to_move];
    }

    /**
     * Get a bitboard containing all pieces that are pinned on the king
     * for the specified side
     *
     * @param[in] to_move Get pinned pieces for this side
     *
     * @return A bitboard with a bit set for each of the pinned squares
     */
    inline uint64 Position::get_pinned_pieces(player_t to_move) const
    {
        const uint64 occupied = _occupied[player_t::black] |
                                _occupied[player_t::white];

        uint64 pinned =
            _attacks_from_queen(_king_sq[to_move], occupied)
                & _occupied[to_move];

        auto& tables = DataTables::get();

        for (uint64 temp = pinned; temp; )
        {
            const square_t sq =
                static_cast<square_t>(msb64(temp));

            switch (tables.directions[sq][_king_sq[to_move]])
            {
            case direction_t::along_rank:
                if (!(_attacks_from_rook(sq, occupied)
                        & tables.ranks64[sq]
                        & (_rooks[flip(to_move)] |
                            _queens[flip(to_move)])))
                    clear_bit64(sq, pinned);
                break;
            case direction_t::along_file:
                if (!(_attacks_from_rook( sq, occupied)
                        & tables.files64[sq]
                        & (_rooks[flip(to_move)] |
                            _queens[flip(to_move)])))
                    clear_bit64(sq, pinned);
                break;
            case direction_t::along_a1h8:
                if (!(_attacks_from_diag(sq, occupied)
                      & tables.a1h8_64[sq]
                      & (_bishops[flip(to_move)] |
                             _queens[flip(to_move)])))
                    clear_bit64(sq, pinned);
                break;
            case direction_t::along_h1a8:
                if (!(_attacks_from_diag(sq, occupied)
                      & tables.h1a8_64[sq]
                      & (_bishops[flip(to_move)] |
                             _queens[flip(to_move)])))
                    clear_bit64(sq, pinned);
            default:
                break;
            }

            clear_bit64(sq, temp);
        }

        return pinned;
    }

    /**
     * Get the player whose turn it is to move in this position
     *
     * @return \ref player_t::white or \ref player_t::black
     */
    inline player_t Position::get_turn() const
    {
        return _to_move;
    }

    /**
     * Determine if the given side, \a to_move, is in check
     *
     * @param [in] to_move \ref player_t::white or \ref player_t::black
     *
     * @return True if \a to_move is in check
     */
    inline bool Position::in_check(player_t to_move) const
    {
        return under_attack(_king_sq[to_move],
            flip(to_move));
    }

    /**
     * Determine whether a piece on a particular square would be pinned
     * on the king (if that piece were there)
     *
     * @param[in] square  The square of interest
     * @param[in] to_move The player whose piece would be pinned
     *
     * @return The direction of the pin
     */
    inline direction_t Position::is_pinned(square_t square,
        player_t to_move) const
    {
        const uint64 occupied = _occupied[player_t::black] |
                                _occupied[player_t::white];

        if (_attacks_from_queen(square,occupied) & _kings[to_move])
        {
            auto& tables = DataTables::get();

            switch(tables.directions[ square ][
                _king_sq[to_move]])
            {
            case direction_t::along_rank:
                if (_attacks_from_rook( square, occupied)
                        & tables.ranks64[square]
                        & (_rooks[flip(to_move)] |
                            _queens[flip(to_move)]))

                    return direction_t::along_rank;
                break;
            case direction_t::along_file:
                if (_attacks_from_rook( square, occupied)
                        & tables.files64[square]
                        & (_rooks[flip(to_move)] |
                            _queens[flip(to_move)]))

                    return direction_t::along_file;
                break;
            case direction_t::along_a1h8:
                if (_attacks_from_diag(square, occupied)
                      & tables.a1h8_64[square]
                      & (_bishops[flip(to_move)] |
                            _queens[flip(to_move)]))

                    return direction_t::along_a1h8;
                break;
            case direction_t::along_h1a8:
                if (_attacks_from_diag(square, occupied)
                      & tables.h1a8_64[square]
                      & (_bishops[flip(to_move)] |
                            _queens[flip(to_move)]))

                    return direction_t::along_h1a8;
            default:
                break;
            }
        }

        return direction_t::none;
    }

    /**
     * Play the given move from the current position. Note that this does
     * NOT check for legality
     *
     * @param[in] move The move data bits
     *
     * @return True on success
     */
    inline bool Position::make_move(int32 move)
    {
        /*
         * 1. Update the position hash signature
         */
        _update_hash(move);

        /*
         * 2. Carry over castling rights to the next ply to be modified
         */
        _castle_rights[_ply+1][player_t::black] =
                _castle_rights[_ply][player_t::black];
        _castle_rights[_ply+1][player_t::white] = 
                _castle_rights[_ply][player_t::white];

        /*
         * 3. Increment the ply count
         */
        _ply++;

        /*
         * 4.  Check if this is a null move. If so, switch sides, clear
         *     the en passant square, and return
         */
        if (move == 0)
        {
            _ep_info[_ply].clear(); _to_move = flip(_to_move);
            return true;
        }

        /*
         * 5. Extract the 21-bit packed move data
         */
        const piece_t captured = extract_captured(move);
        const square_t from    = extract_from(move);
        const piece_t moved    = extract_moved(move);
        const piece_t promote  = extract_promote(move);
        const square_t to      = extract_to(move);

        /*
         * 6. Reset the material change, which will be used to compute
         *    the new material balance
         */
        int delta_material = 0;

        /*
         * 7. Clear the en passant info, which expires after
         *    each half-move
         */
        _ep_info[_ply].clear();

        /*
         * 8.  Update the board squares to reflect the new location of
         *     of the piece moved
         */
        _pieces[from] = piece_t::empty;
        _pieces[to]   = moved;

        /*
         * 9. Update the occupied squares bitboard for the player
         *    who moved
         */
        uint64& occupied = _occupied[_to_move];

        clear_set64(from, to, occupied);

        /*
         * 10. Perform piece-specific position updates
         */
        auto& tables = DataTables::get();

        /*
         * 10.1 A pawn was moved
         */
        if (moved == piece_t::pawn)
        {
            uint64& pawns = _pawns[_to_move];

            /*
             * 10.1.1 Clear the origin square in the pawn bitboard
             */
            pawns &= tables.clear_mask[from];

            /*
             * 10.1.2 If this was a promotion, update the board
             *        squares array with the new piece and adjust
             *        the material change accordingly
             */
            if (promote != piece_t::empty)
            {
                _pieces[to] = promote;

                delta_material += tables.piece_value[promote]
                                    -pawn_value;
            }

            /*
             * 10.1.3 If this was a promotion, update the bitboard
             *        of the piece that was promoted to
             */
            switch (promote)
            {
                case piece_t::knight:
                    _knights[_to_move] |= tables.set_mask[to];
                    break;
                case piece_t::rook:
                    _rooks  [_to_move] |= tables.set_mask[to];
                    break;
                case piece_t::queen:
                    _queens [_to_move] |= tables.set_mask[to];
                    break;
                case piece_t::bishop:
                    _bishops[_to_move] |= tables.set_mask[to];
                    break;
                default:
                    pawns |= tables.set_mask[to];
            }

            /*
             * 10.1.4 Set the en passant target square if the pawn
             *        moved 2 spaces
             */
            if (abs(from-to) == 16)
            {
                uint64 src = _pawns[flip(_to_move)]
                    & tables.rank_adjacent[to];

                _ep_info[_ply].target =
                    tables.minus_8[_to_move][to];

                if (src & (tables.set_mask[to+1]))
                    _ep_info[_ply].src[0] =
                        static_cast<square_t>(to+1);
                if (src & (tables.set_mask[to-1]))
                    _ep_info[_ply].src[1] =
                        static_cast<square_t>(to-1);
            }
        }

        /*
         * 10.2 A knight was moved. All that is needed is to clear
         *      the origin bit and set the destination bit of
         *      the knight bitboard
         */
        else if (moved == piece_t::knight)
        {
            clear_set64(from, to, _knights[_to_move]);
        }

        /*
         * 10.3 A rook was moved
         */
        else if (moved == piece_t::rook)
        {
            /*
             * 10.3.1 Clear/set the origin/destination bits of the
             *        rook bitboard
             */
            clear_set64( from, to, _rooks[_to_move]);

            /*
             * 10.3.2 If we were able to castle with this rook, it
             *        is no longer possible. Remove the castling
             *        rights associated with this rook
             */
            if (_castle_rights[_ply][_to_move] &&
                (tables.back_rank[_to_move] & tables.set_mask[from]))
            {
                switch (get_file(from))
                {
                    case 0:
                        _castle_rights[_ply][_to_move]
                            &= castle_Q;
                    break;
                    case 7:
                        _castle_rights[_ply][_to_move]
                            &= castle_K;
                }
            }
        }

        /*
         * 10.4 A bishop was moved. All that is needed is to clear
         *      the origin bit and set the destination bit of
         *      the bishop bitboard
         */
        else if (moved == piece_t::bishop)
        {
            clear_set64(from, to, _bishops[_to_move]);
        }

        /*
         * 10.5 A queen was moved. All that is needed is to clear
         *      the origin bit and set the destination bit of
         *      the queen bitboard
         */
        else if (moved == piece_t::queen)
        {
            clear_set64(from, to, _queens[_to_move]);
        }

        /*
         * 10.6 A king was moved
         */
        else if (moved == piece_t::king)
        {
            /*
             * 10.6.1 Clear/set the origin/destination bits of the
             *        king bitboard
             */

            clear_set64(from, to, _kings[_to_move]);

            /*
             * 10.6.2 Update the king's location
             */
            _king_sq[_to_move] = to;

            /*
             * 10.6.3 Handle castling moves
             */
            if (abs(from-to) == 2)
            {
                /*
                 * 10.6.3.1 Update the board squares array and
                 *          the rook and occupancy bitboards
                 */
                if (to == tables.castle_OO_dest[_to_move])
                {
                    _pieces[to-1] = piece_t::empty;
                    _pieces[to+1] = piece_t::rook;

                    clear_set64(to-1, to+1, _occupied[_to_move]);
                    clear_set64(to-1, to+1, _rooks[_to_move]);
                }
                else // Queenside castle
                {
                    _pieces[to+2] = piece_t::empty;
                    _pieces[to-1] = piece_t::rook;

                    clear_set64(to+2, to-1, _occupied[_to_move]);
                    clear_set64(to+2, to-1, _rooks[_to_move]);
                }
            }

            /*
             * 10.6.4 Because the king was moved, clear all
             *        castling rights for this player:
             */
            _castle_rights[_ply][_to_move] = 0;
        }

        else // invalid piece was moved
        {
            Abort(false);
        }

        /*
         * 11. Handle moves that capture another piece
         */
        if (captured != piece_t::empty)
        {
            const player_t xside = flip(_to_move);

            /*
             * 11.1 Update the material change according to the value
             *      of the captured piece
             */
            delta_material += tables.piece_value[captured];

            uint64& xoccupied = _occupied[xside];

            switch (captured)
            {
                /*
                 * 11.2 A pawn was captured
                 */
                case piece_t::pawn:

                    /*
                     * 11.2.1 If a pawn was captured non-en passant, clear
                     *        the destination bit in the enemy's pawn
                     *        bitboard
                     */
                    if (xoccupied & tables.set_mask[to])
                    {
                        _pawns[xside] &= tables.clear_mask[to];
                    }
                    else
                    {
                        const square_t vic_square =
                            tables.minus_8[_to_move][to];

                        /*
                         * 11.2.2 This was an en passant capture. Update
                         *        the board squares array and the enemy pawn
                         *        and occupany bitboards accordingly
                         */
                        _pieces[vic_square] = piece_t::empty;

                        xoccupied     &= tables.clear_mask[ vic_square ];
                        _pawns[xside] &=
                            tables.clear_mask[ vic_square ];
                    }

                    break;

                /*
                 * 11.3 A knight was captured. Clear the destination bit
                 *      in the enemy knight bitboard
                 */
                case piece_t::knight:

                    _knights[xside] &= tables.clear_mask[to];
                    break;

                /*
                 * 11.4 A bishop was captured. Clear the destination bit
                 *      in the enemy bishop bitboard
                 */
                case piece_t::bishop:

                    _bishops[xside] &= tables.clear_mask[to];
                    break;

                /*
                 * 11.5  A queen was captured. Clear the destination bit
                 *       in the enemy queen bitboard
                 */
                case piece_t::queen:

                    _queens[xside]  &= tables.clear_mask[to];
                    break;

                /*
                 * 11.6 A rook was captured
                 */
                case piece_t::rook:

                    /*
                     * 11.6.1 Clear the destination bit in the enemy rook
                     *        bitboard
                     */
                    _rooks[xside]   &= tables.clear_mask[to];

                    /*
                     * 11.6.2 Update the opponent's castling rights if he
                     *        could have castled with this rook
                     */
                    if (_castle_rights[_ply][xside] &&
                            (tables.back_rank[xside] & tables.set_mask[to]))
                    {
                        switch (get_file(to))
                        {
                            case 0:
                                _castle_rights[_ply][xside]
                                    &= castle_Q;
                            break;
                            case 7:
                                _castle_rights[_ply][xside]
                                    &= castle_K;
                        }
                    }

                    break;
                default:
                    Abort(false);
            }

            /*
             * 11.7 Clear the destination bit of the enemy
             *      occupancy bitboard
             */
            xoccupied &=
                tables.clear_mask[to];
        }

        /*
         * 12. If this is a reversible move, increment the half-move
         *     clock. Otherwise, reset it to zero
         */
        if (moved == piece_t::pawn || captured != piece_t::empty)
        {
            _half_move[_ply] = 0;
            _last_halfmove_reset[_ply] = _ply;
        }
        else
        {
            _half_move[_ply] = _half_move[_ply-1] + 1;

            _last_halfmove_reset[_ply]
                = _last_halfmove_reset[_ply-1];
        }

        /*
         * 13. Increment the full-move number if black played
         */
        if ( _to_move == player_t::black )
            _full_move += 1;

        /*
         * 14. Update the material balance
         */
        _material[ _to_move ] +=
            delta_material;

        /* 
         * 15. Flip the side on move
         */
        _to_move = flip(_to_move);

        return true;
    }

    /**
     * Fetch the piece that is on a particular square, returning one of
     * the piece types defined in chess.h
     *
     * @param[in] square The square of interest
     *
     * @return The piece on \a square
     */
    inline piece_t Position::piece_on(square_t square) const
    {
        return _pieces[square];
    }

    /**
     * Determine if the given square is being attacked by the specified
     * side
     *
     * @param[in] square   The square to examine
     * @param[in] to_move  Check if this side is attacking the square
     *
     * @return  True if the square is under attack by \a to_move, false
     *          otherwise
     */
    inline bool Position::under_attack(square_t square,
        player_t to_move) const
    {
        auto& tables = DataTables::get();

        if (tables.pawn_attacks[flip(to_move)][square]
                & _pawns[to_move])
            return true;

        if (tables.king_attacks  [square] & _kings  [to_move])
            return true;

        if (tables.knight_attacks[square] & _knights[to_move])
            return true;

        const uint64 rooks_queens   =
            _rooks[ to_move ] | _queens[to_move];
        const uint64 bishops_queens =
            _bishops[to_move] | _queens[to_move];

        uint64 rook_attackers =
            attacks_from< piece_t::rook >(square);

        if ( rook_attackers & rooks_queens )
            return true;

        uint64 diag_attackers = 
            attacks_from<piece_t::bishop>(square);

        if (diag_attackers & bishops_queens)
            return true;

        return false;
    }

    /**
     * Undo the given move from the current position. This is essentially
     * the inverse of \ref make_move()
     *
     * @param[in] move The move data bits
     *
     * @return True on success
     */
    inline bool Position::unmake_move(int32 move)
    {
        /*
         * 1. Back up to the previous ply to restore castling,
         *    en passant info, hash, and half-move clock
         */
        _ply--;

        /*
         * 2. Restore the turn to the player who moved
         */
        _to_move = flip(_to_move);

        /*
         * 3. If this is a null move, we are done
         */
        if (move == 0) return true;

        /*
         * 4. Extract the 21-bit packed move data
         */
        const piece_t captured = extract_captured(move);
        const square_t from    = extract_from(move);
        const piece_t moved    = extract_moved(move);
        const piece_t promote  = extract_promote(move);
        const square_t to      = extract_to(move);

        /*
         * 5. Restore the piece array by returning the moved
         *    piece to its original location
         */
        _pieces[to]   = captured;
        _pieces[from] = moved;

        /*
         * 6. Initialize the change in material balance
         */
        int delta_material = 0;

        /*
         * 7. Restore the occupancy bits for the player
         *    that moved
         */
        clear_set64(to, from, _occupied[_to_move]);

        /*
         * 8. Perform piece-specific restorations
         */
        auto& tables = DataTables::get();

        if (moved == piece_t::pawn)
        {
            /*
             * 8.1.1 A pawn was moved. Reset the bit for its
             *       origin square
             */
            _pawns[_to_move] |= tables.set_mask[from];

            /*
             * 8.1.2 If the pawn was a promoted, restore the
             *       material balance
             */
            if (promote != piece_t::empty)
            {
                delta_material +=
                    tables.piece_value[promote] - pawn_value;
            }

            /*
             * 8.1.3  Clear the "to" bit in the bitboard for
             *        the piece that was promoted to
             */
            switch (promote)
            {
                case piece_t::knight:
                    _knights[_to_move]
                        &= tables.clear_mask[to];
                    break;
                case piece_t::rook:
                    _rooks[_to_move]
                        &= tables.clear_mask[to];
                    break;
                case piece_t::queen:
                    _queens[_to_move]  
                        &= tables.clear_mask[to];
                    break;
                case piece_t::bishop:
                    _bishops[_to_move] 
                        &= tables.clear_mask[to];
                    break;
                default:
                    _pawns[ _to_move ]
                        &= tables.clear_mask[to];
            }
        }
        else if (moved == piece_t::knight)
        {
            /*
             * 8.2 A knight was moved. Clear the "to" bit and
             *     set the origin bit in the knight board
             */
            clear_set64(to, from, _knights[_to_move]);
        }
        else if (moved == piece_t::rook)
        {
            /*
             * 8.3 A rook was moved. Clear the "to" bit and
             *     set the origin bit in the rook board
             */
            clear_set64( to, from, _rooks[_to_move] );
        }
        else if (moved == piece_t::bishop)
        {
            /*
             * 8.4 A bishop was moved. Clear the "to" bit and
             *     set the origin bit in the bishop board
             */
            clear_set64(to, from, _bishops[_to_move]);
        }
        else if (moved == piece_t::queen)
        {
            /*
             * 8.5 A queen was moved. Clear the "to" bit and
             *     set the origin bit in the queen board
             */
            clear_set64( to, from, _queens[_to_move]);
        }
        else if (moved == piece_t::king)
        {
            /*
             * 8.6 A king was moved. Clear the "to" bit and
             *     set the origin bit in the king board
             */
            clear_set64( to, from, _kings[_to_move] );
                _king_sq[_to_move] = from;

            /*
             * 8.6.1 Handle castling moves
             */
            if (abs(from-to) == 2)
            {
                if (to == tables.castle_OO_dest[_to_move])
                {
                    /*
                     * 8.6.1.1 This move castles short. Remove
                     *         the rook from the F1 (or F8) square
                     *         and place it on its home square
                     */
                    _pieces[to-1] = piece_t::rook;
                    _pieces[to+1] = piece_t::empty;

                    clear_set64(to+1, to-1, _rooks[_to_move]);
                    clear_set64(to+1, to-1,
                        _occupied[ _to_move ]);
                }
                else
                {
                    /*
                     * 8.6.1.2 This move castles long. Remove
                     *         the rook from the D1 (or D8) square
                     *         and place it on its home square
                     */
                    _pieces[to+2] = piece_t::rook;
                    _pieces[to-1] = piece_t::empty;

                    clear_set64(to-1, to+2, _rooks[_to_move]);
                    clear_set64(to-1, to+2,
                        _occupied[ _to_move ]);
                    
                }
            }
        }
        else // invalid piece was moved
        {
            Abort(false);
        }

        /*
         * 9. Handle captures
         */
        if (captured != piece_t::empty)
        {
            /*
             * 9.1 Add the captured piece's value back into the
             *     material balance
             */
            delta_material += tables.piece_value[captured];

            /*
             * 9.2  Restore the enemy occupancy bitboard if the
             *      captured piece was NOT a pawn, since en
             *      passant captures must be handled separately
             */
            if (captured != piece_t::pawn)
            {
                _occupied[flip(_to_move)]
                    |= tables.set_mask[ to ];
            }

            /*
             * 9.3 Captured piece-specific restorations:
             */
            if (captured == piece_t::pawn)
            {
                /*
                 * 9.3.1 A pawn was captured
                 */
                if (to == _ep_info[_ply].target)
                {
                    /*
                     * 9.3.1.1 If this was an en passant capture,
                     *         restore the captured pawn to its double
                     *         advanced location, and update the
                     *         enemy occupancy
                     */
                    const square_t vic = tables.minus_8[_to_move][to];

                    _pieces[vic] = piece_t::pawn;

                    _occupied[flip(_to_move)] |= tables.set_mask[vic];
                    _pawns[flip(_to_move)]    |= tables.set_mask[vic];

                    _pieces[to] = piece_t::empty;
                }
                else
                {
                    /*
                     * 9.3.1.2 Otherwise, this was a normal capture;
                     *         restore the captured pawn to its diagonal
                     *         offset and update the enemy occupancy
                     */
                    _occupied[flip(_to_move)]
                        |= tables.set_mask[ to ];
                    _pawns[flip(_to_move)]
                        |= tables.set_mask[ to ];
                }
            }
            else if (captured == piece_t::knight)
            {
                /*
                 * 9.3.2 If this was a knight capture, reset the "to"
                 *       bit in the enemy knights bitboard
                 */
                _knights[flip(_to_move)] |= tables.set_mask[to];
            }
            else if (captured == piece_t::queen)
            {
                /*
                 * 9.3.3 If this was a queen capture, reset the "to"
                 *       bit in the enemy queens bitboard
                 */
                _queens[flip(_to_move)] |= tables.set_mask[to];
            }
            else if (captured == piece_t::rook)
            {
                /*
                 * 9.3.4 If this was a rook capture, reset the "to"
                 *       bit in the enemy rooks bitboard
                 */
                _rooks[flip(_to_move)] |= tables.set_mask[to];
            }
            else if (captured == piece_t::bishop)
            {
                /*
                 * 9.3.5 If this was a bishop capture, reset the "to"
                 *       bit in the enemy bishops bitboard
                 */
                _bishops[flip(_to_move)] |= tables.set_mask[to];
            }
        }

        /*
         * 10. If black played, decrement the full move number
         */
        if (_to_move == player_t::black)
            _full_move--;

        /*
         * 11. Restore the material balance
         */
        _material[ _to_move ] -=
            delta_material;

        return true;
    }

    /**
     * Generates the squares attacked by a bishop located at the given
     * square
     *
     * @param[in] square   The bishop's location
     * @param[in] occupied The occupied squares bitboard
     *
     * @return A bitboard specifying all squares attacked by this piece
     */
    inline uint64 Position::_attacks_from_diag(square_t square,
        uint64 occupied ) const
    {
        auto& tables = DataTables::get();

        return tables.bishop_attacks[tables.bishop_offsets[square] +
                (((occupied & tables.bishop_attacks_mask[square])
                    * tables.diag_magics[square]) >> tables.bishop_db_shifts[square])];
    }

    /**
     * Generates the squares attacked by a queen located at the given
     * square
     *
     * @param[in] square   The queen's location
     * @param[in] occupied The occupied squares bitboard
     *
     * @return  A bitboard showing all squares attacked by this piece
     */
    inline uint64 Position::_attacks_from_queen(square_t square,
        uint64 occupied) const
    {
        return _attacks_from_rook(square, occupied)
                | _attacks_from_diag( square, occupied );
    }

    /**
     * Generates the squares attacked by a rook located at the given
     * square
     *
     * @param[in] square   The rook's location
     * @param[in] occupied The occupied squares bitboard
     *
     * @return A bitboard showing all squares attacked by this piece
     */
    inline uint64 Position::_attacks_from_rook(square_t square,
        uint64 occupied) const
    {
        auto& tables = DataTables::get();

        return tables.rook_attacks[tables.rook_offsets[square] +
                (((occupied & tables.rook_attacks_mask[square])
                    * tables.rook_magics[square]) >> tables.rook_db_shifts[square])];
    }

    /**
     * Get the mobility of a bishop on \a square. This is hashed to avoid
     * computing it on the fly
     *
     * @param[in] square   The bishop's location
     * @param[in] occupied The occupied squares bitboard
     *
     * @return The Hamming weight of \ref _attacks_from_diag()
     */
    inline int Position::_get_diag_mobility(square_t square,
        uint64 occupied) const
    {
        auto& tables = DataTables::get();

        return tables.bishop_mobility[tables.bishop_offsets[square] +
                (((occupied & tables.bishop_attacks_mask[square])
                    * tables.diag_magics[square]) >> tables.bishop_db_shifts[square])];
    }

    /**
     * Get the mobility of a queen on \a square. This is hashed to avoid
     * computing it on the fly
     *
     * @param[in] square   The queen's location
     * @param[in] occupied The occupied squares bitboard
     *
     * @return The Hamming weight of \ref attacks_from_queen()
     */
    inline int Position::_get_queen_mobility(square_t square,
        uint64 occupied) const
    {
        return _get_rook_mobility(square, occupied)
                + _get_diag_mobility(square, occupied);
    }

    /**
     * Returns the mobility of a rook on \a square. This is hashed to avoid
     * computing it on the fly
     *
     * @param[in] square   The rook's location
     * @param[in] occupied The occupied squares bitboard
     *
     * @return The Hamming weight of \ref attacks_from_rook()
     */
    inline int Position::_get_rook_mobility(square_t square,
        uint64 occupied) const
    {
        auto& tables = DataTables::get();

        return tables.rook_mobility[tables.rook_offsets[square] +
                (((occupied & tables.rook_attacks_mask[square])
                    * tables.rook_magics[square]) >> tables.rook_db_shifts[square])];
    }

    /**
     * Update the hash signature at the current ply. This should only
     * be called when making a move
     *
     * @param[in] move The last move made
     */
    inline void Position::_update_hash(int32 move)
    {
        uint64& hash = _save_hash[_ply+1];
        hash         = _save_hash[_ply];

        const piece_t captured = extract_captured(move);
        const int from         = extract_from(move);
        const piece_t moved    = extract_moved(move);
        const piece_t promote  = extract_promote(move);
        const int to           = extract_to(move);

        /*
         * If we had a valid en passant square for the current ply,
         * remove it from the hash key:
         */
        if (_ep_info[_ply].target != square_t::BAD_SQUARE)
        {
            hash ^=
              _hash_input.en_passant[get_file(_ep_info[_ply].target)];
        }

        /*
         * If this is a null move, update the hash to reflect
         * the side on move and exit
         */
        if (move == 0)
        {
            hash ^= _hash_input.to_move;
            return;
        }

        /*
         * XOR in the en passant value if we advanced a pawn by
         * two squares
         */
        if (abs(from - to) == 16)
        {
            hash ^= _hash_input.en_passant[ get_file(to) ];
        }

        const player_t& to_move = _to_move;

        /*
         * Update the hash entry to reflect the new location of
         * the piece moved:
         */
        hash ^=
            _hash_input.piece[to_move][moved][from];

        if (promote == piece_t::empty)
            hash ^= _hash_input.piece[to_move][ moved ][to];
        else
            hash ^= _hash_input.piece[to_move][promote][to];

        /*
         * Update the hash key if we moved a rook with which we
         * could have castled
         */
        if (moved == piece_t::rook)
        {
            if (get_file(from) == 0 &&
                (_castle_rights[_ply][to_move] & castle_K))
            {
                hash ^=
                    _hash_input.castle_rights[to_move][castle_K_index];
            }
            else if (get_file(from) == 7 &&
                (_castle_rights[_ply][to_move] & castle_Q))
            {
                hash ^=
                    _hash_input.castle_rights[to_move][castle_Q_index];
            }
        }

        /*
         * Update the hash key to reflect new castling rights
         * if the king was moved
         */
        else if (moved == piece_t::king)
        {
            if (_castle_rights[_ply][to_move] & castle_K)
                hash ^=
                    _hash_input.castle_rights[to_move][castle_K_index];
            if (_castle_rights[_ply][to_move] & castle_Q)
                hash ^=
                    _hash_input.castle_rights[to_move][castle_Q_index];

            /*
             * Update the hash key for the rook involved with
             * the castle
             */
            if (abs(from-to) == 2)
            {
                if (to_move == player_t::white)
                {
                    if (to == square_t::G1)
                    {
                        hash ^=
                            _hash_input.piece[player_t::white][
                                piece_t::rook][square_t::H1];
                        hash ^=
                            _hash_input.piece[player_t::white][
                                piece_t::rook][square_t::F1];
                    }
                    else
                    {
                        hash ^=
                            _hash_input.piece[player_t::white][
                                piece_t::rook][square_t::A1];
                        hash ^=
                            _hash_input.piece[player_t::white][
                                piece_t::rook][square_t::D1];
                    }
                }
                else
                {
                    if (to == square_t::G8)
                    {
                        hash ^=
                            _hash_input.piece[player_t::black][
                                piece_t::rook][square_t::H8];
                        hash ^=
                            _hash_input.piece[player_t::black][
                                piece_t::rook][square_t::F8];
                    }
                    else
                    {
                        hash ^=
                            _hash_input.piece[player_t::black][
                                piece_t::rook][square_t::A8];
                        hash ^=
                            _hash_input.piece[player_t::black][
                                piece_t::rook][square_t::D8];
                    }
                }
            }
        }

        /*
         * Next, Update the hash if an opponent's piece was captured
         */
        if (captured != piece_t::empty)
        {
            const player_t x_side = flip(to_move);

            switch (captured)
            {

            case piece_t::pawn:

                if (_pieces[to] != piece_t::empty)
                {
                    hash ^= _hash_input.piece[x_side][
                        piece_t::pawn][to];
                }
                else
                {
                    if (to_move == player_t::white)
                        hash ^= _hash_input.piece[player_t::black][
                                    piece_t::pawn][to-8];
                    else
                        hash ^= _hash_input.piece[player_t::white][
                            piece_t::pawn][to+8];
                }

                break;

            case piece_t::rook:

                /*
                 * Update the hash key if we captured a rook with which
                 * the opponent could have castled
                 */
                switch (get_file(to))
                {
                case 0:
                    if (_castle_rights[_ply][x_side] & castle_K)
                    {
                        hash ^= _hash_input.castle_rights[x_side][
                            castle_K_index];
                    }
                break;

                case 7:
                    if (_castle_rights[_ply][x_side] & castle_Q)
                    {
                        hash ^= _hash_input.castle_rights[x_side][
                            castle_Q_index];
                    }
                }

            default:

                hash ^= _hash_input.piece[
                    flip(to_move)][captured][to];
            }
        }

        /*
         * Update the hash to reflect whose turn it is
         */
        hash ^=
            _hash_input.to_move;
    }
}

#endif
