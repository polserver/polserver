#ifndef UO_CONVERT_MAIN_H
#define UO_CONVERT_MAIN_H

#include <set>
#include <string>

#include "../clib/Program/ProgramMain.h"
#include "../plib/mapwriter.h"

namespace Pol
{
namespace UoConvert
{
class UoConvertMain final : public Pol::Clib::ProgramMain
{
public:
  UoConvertMain();
  virtual ~UoConvertMain();

  std::set<unsigned int> HouseTypes;
  std::set<unsigned int> BoatTypes;
  std::set<unsigned int> StairTypes;
  std::set<unsigned int> MountTypes;


  bool cfg_use_no_shoot;
  bool cfg_LOS_through_windows;

  void display_flags();

  void write_flags( FILE* fp, unsigned int flags );

  void create_tiles_cfg();
  void create_landtiles_cfg();

  void create_multis_cfg();
  void create_multis_cfg( FILE* multi_idx, FILE* multi_mul, FILE* multis_cfg );

  void write_multi( FILE* multis_cfg, unsigned id, FILE* multi_mul, unsigned int offset,
                    unsigned int length );

  void create_map( const std::string& realm, unsigned short width, unsigned short height );
  void update_map( const std::string& realm, unsigned short x, unsigned short y );

  void create_maptile( const std::string& realmname );

  void ProcessSolidBlock( unsigned short x_base, unsigned short y_base,
                          Plib::MapWriter& mapwriter );

  unsigned empty = 0, nonempty = 0;
  unsigned total_statics = 0;
  unsigned with_more_solids = 0;

protected:
  virtual int main();

  bool convert_uop_to_mul();

  void setup_uoconvert();
  void load_uoconvert_cfg();

private:
  virtual void showHelp();
};
}  // namespace UoConvert
}  // namespace Pol

#endif  // UO_CONVERT_MAIN_H
