#include "cr_base/logging/logging.h"
#include "cr_winrt/hook.h"

namespace ca {
namespace tw {

// 服务器封包处理点:
// CA.exe+5DD66D - 8B 8F D0000000        - mov ecx,[edi+000000D0]
// CA.exe+5DD673 - 56                    - push esi // esi = 服务器包
// CA.exe+5DD674 - 8B 01                 - mov eax,[ecx]
// CA.exe+5DD676 - FF 90 CC000000        - call dword ptr [eax+000000CC]

// 00DE40A0 00000000 0000000D FFFFFFFF 
// 0000000D 00000002 00000003 00000002 
// 00000006 00000001 00DE52A0 00000000 
// 00000040 0ECB8B08 00000000 00000000 
// 00000000 0000000D 00000001 00000002 
// 00000003 00000000 0000112F 7312C3B5 
// 00000000 00000000 00000000 00000000

/* packet init handle */

void Handle_PacketInit_00B4B660(cr::winrt::Hooker::ContextRegister* reg) {
  void* eip2 = reinterpret_cast<void**>(reg->ebp)[1];
  void* eip = reinterpret_cast<void*>(reg->esp[0]);
  uintptr_t packet_type = reg->esp[1];
  CR_LOG(Info) << "GeneralClientPakcet(00B4B660)[" << eip << "]:" 
               << "[ "<< eip2 <<"]:"
               << packet_type;
}

void Handle_PacketInit_00B4B740(cr::winrt::Hooker::ContextRegister* reg) {
  void* eip2 = reinterpret_cast<void**>(reg->ebp)[1];
  void* eip = reinterpret_cast<void*>(reg->esp[0]);
  uintptr_t packet_type = reg->esp[1];
  CR_LOG(Info) << "GameClientPakcet(00B4B740)[" << eip << "]:"
               << "[ "<< eip2 <<"]:"
               << packet_type;;
}

void Handle_PacketRecvInit_009DD676(cr::winrt::Hooker::ContextRegister* reg) {
  unsigned packet_type = *(unsigned*)(reg->esi + 0x8);
  unsigned packet_len = *(unsigned*)(reg->esi + 0x10);
  void* handle_ip = *(void**)(reg->eax + 0xCC);
  CR_LOG(Info) << "RecvPacket["<< handle_ip <<"]: type=" << packet_type
               << ", len = " << packet_len;
}

void InitializeNetMessageHook() {
  using cr::winrt::Hooker;

  Hooker* hooker = Hooker::GetInstance();
  CR_LOG(Info) << "Hooker:" << hooker;

  hooker->CreateContextHook(
      (void *)0x00B4B660, Handle_PacketInit_00B4B660, true);
  hooker->CreateContextHook(
      (void *)0x00B4B740, Handle_PacketInit_00B4B740, true);
  hooker->CreateContextHook(
      (void *)0x009DD676, Handle_PacketRecvInit_009DD676, true);
}

}  // namespace tw
}  // namespace ca
