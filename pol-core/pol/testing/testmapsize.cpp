/** @file
 *
 * @par History
 */

#include "../../plib/mul/map.h"
#include "testenv.h"


namespace Pol::Testing
{
// Regression tests for uoconvert's UOP map-size handling. Two bugs used to break
// conversion of several standard maps:
//   * the UOP size estimator dropped one block from the final chunk, undercounting
//     maps whose block count isn't a multiple of 4096 (Ilshenar/map2, Tokuno/map4)
//     so their true size was rejected as "smaller than the given width and height";
//   * neither validation nor map0/map1 dimension auto-detection tolerated the single
//     trailing padding block some clients append, which is size + one block (196).
void mapsize_validation_test()
{
  using Pol::Plib::MUL::Map;
  using Pol::Plib::MUL::MapInfo;

#define T_MAPSIZE( f, res, msg ) UnitTest( []() { return f; }, res, msg )

  // Ilshenar (2304x1600) and Tokuno (1448x1448) both have a partial final chunk;
  // their exact size must validate (the estimator previously undercounted them).
  constexpr size_t map2_size = 11289600;  // 2304/8 * 1600/8 * 196
  constexpr size_t map4_size = 6421156;   // 1448/8 * 1448/8 * 196
  constexpr size_t map0_size = 89915392;  // 7168/8 * 4096/8 * 196
  constexpr size_t blk = Map::blockSize;

  T_MAPSIZE( Map::valid_size( map2_size, 2304, 1600 ), true, "map2 exact size valid" );
  T_MAPSIZE( Map::valid_size( map4_size, 1448, 1448 ), true, "map4 exact size valid" );

  // The undersized value the old estimator produced (one block short) must stay
  // invalid -- this is exactly what caused the abort.
  T_MAPSIZE( Map::valid_size( map2_size - blk, 2304, 1600 ), false,
             "map2 size minus a block invalid" );
  T_MAPSIZE( Map::valid_size( map4_size - blk, 1448, 1448 ), false,
             "map4 size minus a block invalid" );

  // A single trailing padding block (modern clients) must validate.
  T_MAPSIZE( Map::valid_size( map2_size + blk, 2304, 1600 ), true, "map2 padded size valid" );
  T_MAPSIZE( Map::valid_size( map4_size + blk, 1448, 1448 ), true, "map4 padded size valid" );
  T_MAPSIZE( Map::valid_size( map0_size + blk, 7168, 4096 ), true, "map0 padded size valid" );

  // Two padding blocks is not a case we accept.
  T_MAPSIZE( Map::valid_size( map2_size + 2 * blk, 2304, 1600 ), false,
             "map2 double-padded invalid" );

  // map0/map1 dimensions are guessed from the file size; both the exact and the
  // padded size must resolve to 7168x4096 (the padded case previously failed to guess).
  T_MAPSIZE( MapInfo( 0, map0_size ).guessed(), true, "map0 exact size guesses dimensions" );
  T_MAPSIZE( MapInfo( 0, map0_size ).width(), 7168, "map0 exact guessed width" );
  T_MAPSIZE( MapInfo( 0, map0_size ).height(), 4096, "map0 exact guessed height" );

  T_MAPSIZE( MapInfo( 1, map0_size + blk ).guessed(), true, "map1 padded size guesses dimensions" );
  T_MAPSIZE( MapInfo( 1, map0_size + blk ).width(), 7168, "map1 padded guessed width" );
  T_MAPSIZE( MapInfo( 1, map0_size + blk ).height(), 4096, "map1 padded guessed height" );

  // A genuinely wrong size (not the grid, not the grid plus one pad block) is rejected.
  T_MAPSIZE( MapInfo( 0, map0_size - blk ).guessed(), false, "map0 undersized fails to guess" );

  // map2..map5 use fixed dimensions from the mapid, never guessed from size.
  T_MAPSIZE( MapInfo( 2, map2_size ).guessed(), false, "map2 dimensions are fixed, not guessed" );
  T_MAPSIZE( MapInfo( 2, map2_size ).width(), 2304, "map2 fixed width" );
  T_MAPSIZE( MapInfo( 4, map4_size ).height(), 1448, "map4 fixed height" );

#undef T_MAPSIZE
}

}  // namespace Pol::Testing
