/** @file
 *
 * @par History
 */


#ifndef POLVAR_H
#define POLVAR_H
namespace Pol
{
namespace Core
{
class PolVar
{
public:
  std::string DataWrittenBy;
  bool DataWrittenBy99OrLater;
  bool DataWrittenBy93;

  [[nodiscard]] size_t estimateSize() const {
    return sizeof( PolVar ) + DataWrittenBy.capacity();
  }
};
}
}
#endif
