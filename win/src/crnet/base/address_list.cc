// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "crnet/base/address_list.h"

#include <utility>
#include <map>

///#include "crbase/containers/flat_map.h"
#include "crbase/logging.h"
#include "crnet/base/sys_addrinfo.h"

namespace crnet {

AddressList::AddressList() = default;

AddressList::AddressList(const AddressList&) = default;

AddressList& AddressList::operator=(const AddressList&) = default;

AddressList::~AddressList() = default;

AddressList::AddressList(const IPEndPoint& endpoint) {
  push_back(endpoint);
}

// static
AddressList AddressList::CreateFromIPAddress(const IPAddress& address,
                                             uint16_t port) {
  return AddressList(IPEndPoint(address, port));
}

// static
AddressList AddressList::CreateFromIPLiteral(const cr::StringPiece& address,
                                             uint16_t port) {
  IPAddress ip_address;
  ip_address.AssignFromIPLiteral(address);
  return AddressList(IPEndPoint(ip_address, port));
}

// static
AddressList AddressList::CreateFromIPAddressList(
    const IPAddressList& addresses,
    const std::string& canonical_name) {
  AddressList list;
  list.set_canonical_name(canonical_name);
  for (auto iter = addresses.begin(); iter != addresses.end(); ++iter) {
    list.push_back(IPEndPoint(*iter, 0));
  }
  return list;
}

// static
AddressList AddressList::CreateFromAddrinfo(const struct addrinfo* head) {
  CR_DCHECK(head);
  AddressList list;
  if (head->ai_canonname)
    list.set_canonical_name(std::string(head->ai_canonname));
  for (const struct addrinfo* ai = head; ai; ai = ai->ai_next) {
    IPEndPoint ipe;
    // NOTE: Ignoring non-INET* families.
    if (ipe.FromSockAddr(ai->ai_addr, static_cast<socklen_t>(ai->ai_addrlen)))
      list.push_back(ipe);
    else
      CR_DLOG(Warning) << "Unknown family found in addrinfo: " << ai->ai_family;
  }
  return list;
}

// static
AddressList AddressList::CopyWithPort(const AddressList& list, uint16_t port) {
  AddressList out;
  out.set_canonical_name(list.canonical_name());
  for (size_t i = 0; i < list.size(); ++i)
    out.push_back(IPEndPoint(list[i].address(), port));
  return out;
}

void AddressList::SetDefaultCanonicalName() {
  CR_DCHECK(!empty());
  set_canonical_name(front().ToStringWithoutPort());
}

///void AddressList::Deduplicate() {
///  if (size() > 1) {
///    std::vector<std::pair<IPEndPoint, int>> make_me_into_a_map(size());
///    for (auto& addr : *this)
///      make_me_into_a_map.emplace_back(addr, 0);
///    std::map<IPEndPoint, int> inserted(std::move(make_me_into_a_map));
///
///    std::vector<IPEndPoint> deduplicated_addresses;
///    deduplicated_addresses.reserve(inserted.size());
///    for (const auto& addr : *this) {
///      int& count = inserted[addr];
///      if (!count) {
///        deduplicated_addresses.push_back(addr);
///        ++count;
///      }
///    }
///    endpoints_.swap(deduplicated_addresses);
///  }
///}

}  // namespace crnet