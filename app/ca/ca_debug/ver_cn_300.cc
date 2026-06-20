#include "cr_base/logging/logging.h"
#include "cr_winrt/hook.h"

namespace ca {
namespace cn {

// 018122D4 00000000 00000004 FFFFFFFF 
// 000006BD 00000002 00000003 00000002 
// 00000000 00000001 01812E18 00000384 
// 000006BD 4CE385E8 00000000 00000000 
// 00000000 000006BD 00000078 00000002 
// 00000003 00000000 00000000 00000001

/* packet init handle */

void Handle_PacketInit_0114F550(cr::winrt::Hooker::ContextRegister* reg) {
  void* eip2 = reinterpret_cast<void**>(reg->ebp)[1];
  void* eip = reinterpret_cast<void*>(reg->esp[0]);
  uintptr_t packet_type = reg->esp[1];
  CR_LOG(Info) << "GeneralClientPacket(0114F550)[" << eip << "]:" 
               << "[ "<< eip2 <<"]:"
               << packet_type;
}

void Handle_PacketInit_0114F5D0(cr::winrt::Hooker::ContextRegister* reg) {
  void* eip2 = reinterpret_cast<void**>(reg->ebp)[1];
  void* eip = reinterpret_cast<void*>(reg->esp[0]);
  uintptr_t packet_type = reg->esp[1];
  CR_LOG(Info) << "GameClientPacket(0114F5D0)[" << eip << "]:"
               << "[ "<< eip2 <<"]:"
               << packet_type;;
}

void Handle_PacketInit_0114F6B0(cr::winrt::Hooker::ContextRegister* reg) {
  void* eip2 = reinterpret_cast<void**>(reg->ebp)[1];
  void* eip = reinterpret_cast<void*>(reg->esp[0]);
  uintptr_t packet_type = reg->esp[1];
  CR_LOG(Info) << "GeneralClientPacket(0114F6B0)[" << eip << "]:" 
               << "[ "<< eip2 <<"]:"
               << packet_type;;
}

void Handle_PacketInit_0114F6F0(cr::winrt::Hooker::ContextRegister* reg) {
  void* eip2 = reinterpret_cast<void**>(reg->ebp)[1];
  void* eip = reinterpret_cast<void*>(reg->esp[0]);
  uintptr_t packet_type = reg->esp[1];
  CR_LOG(Info) << "GameClientPacket(0114F6F0)[" << eip << "]:" 
               << "[ "<< eip2 <<"]:"
               << packet_type;;
}

void InitializeNetMessageHook() {
  using cr::winrt::Hooker;

  Hooker* hooker = Hooker::GetInstance();
  CR_LOG(Info) << "Hooker:" << hooker;

  hooker->CreateContextHook(
      (void *)0x0114F550, Handle_PacketInit_0114F550, true);
  hooker->CreateContextHook(
      (void *)0x0114F5D0, Handle_PacketInit_0114F5D0, true);
  hooker->CreateContextHook(
      (void *)0x0114F6B0, Handle_PacketInit_0114F6B0, true);
  hooker->CreateContextHook(
      (void *)0x0114F6F0, Handle_PacketInit_0114F6F0, true);
}

}  // namespace cn
}  // namespace ca
