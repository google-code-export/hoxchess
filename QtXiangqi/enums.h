#ifndef ENUMS_H
#define ENUMS_H

/**
 * Color for both Piece and Role.
 */
enum ColorEnum
{
    HC_COLOR_UNKNOWN = -1,
        // This type indicates the absense of color or role.
        // For example, it is used to indicate the player is not even
        // at the table.

    HC_COLOR_RED,   // RED color.
    HC_COLOR_BLACK, // BLACK color.

    HC_COLOR_NONE
        // NOTE: This type actually does not make sense for 'Piece',
        //       only for "Player". It is used to indicate the role of a player
        //       who is currently only observing the game, not playing.

};

/**
 * Piece's Type.
 *
 *  King (K), Advisor (A), Elephant (E), chaRiot (R), Horse (H),
 *  Cannons (C), Pawns (P).
 */
enum PieceEnum
{
    HC_PIECE_INVALID = 0,
    HC_PIECE_KING,                 // King (or General)
    HC_PIECE_ADVISOR,              // Advisor (or Guard, or Mandarin)
    HC_PIECE_ELEPHANT,             // Elephant (or Ministers)
    HC_PIECE_CHARIOT,              // Chariot ( Rook, or Car)
    HC_PIECE_HORSE,                // Horse ( Knight )
    HC_PIECE_CANNON,               // Canon
    HC_PIECE_PAWN                  // Pawn (or Soldier)
};

/**
 * Game's status.
 */
enum GameStatusEnum
{
    HC_GAME_STATUS_UNKNOWN = -1,

    HC_GAME_STATUS_IN_PROGRESS,
    HC_GAME_STATUS_RED_WIN,        // Game Over. Red won.
    HC_GAME_STATUS_BLACK_WIN,      // Game Over. Black won.
    HC_GAME_STATUS_DRAWN,          // Game Over. Drawn.
    HC_GAME_STATUS_TOO_MANY_MOVES  // Game Over. Too many moves.
};

#endif // ENUMS_H
