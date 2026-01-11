/** @file
 *
 * @par History
 */


#ifndef H_UNIEMOD_H
#define H_UNIEMOD_H

#ifndef POL_POLMODL_H
#include "../polmodl.h"
#endif

#include "../../clib/rawtypes.h"
#include "../network/pktdef.h"
#include "../uoexec.h"

namespace Pol
{
namespace Bscript
{
class BObjectImp;
}  // namespace Bscript
namespace Network
{
class Client;
}
namespace Core
{
void send_unicode_prompt( Network::Client* client, u32 serial );
class UOExecutor;
class UObject;
}  // namespace Core
namespace Items
{
class Item;
}  // namespace Items
namespace Mobile
{
class Character;
}  // namespace Mobile
}  // namespace Pol

namespace Pol
{
namespace Module
{
class UnicodeExecutorModule
    : public Bscript::TmplExecutorModule<UnicodeExecutorModule, Core::PolModule>
{
public:
  explicit UnicodeExecutorModule( Core::UOExecutor& exec );
  ~UnicodeExecutorModule() override;

  [[nodiscard]] Bscript::BObjectImp* mf_PrintTextAboveUC();  // OverObject, Text, Font, Color
  [[nodiscard]] Bscript::BObjectImp*
  mf_PrintTextAbovePrivateUC();  // OverObject, Text, ToChar, Font, Color

  [[nodiscard]] Bscript::BObjectImp* mf_BroadcastUC();       // Text
  [[nodiscard]] Bscript::BObjectImp* mf_SendSysMessageUC();  // Character, Text

  [[nodiscard]] Bscript::BObjectImp* mf_RequestInputUC();  // ToChar, Item, TextPrompt
  [[nodiscard]] Bscript::BObjectImp*
  mf_SendTextEntryGumpUC();  // ToChar, Text1, Cancel, Style, MaxLen, Text2

  Mobile::Character* prompt_chr;

  size_t sizeEstimate() const override;
};
}  // namespace Module
}  // namespace Pol
#endif
