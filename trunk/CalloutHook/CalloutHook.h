#ifndef CALLOUTHOOK_H
#define CALLOUTHOOK_H
#ifdef __cplusplus
extern "C" {
#endif
#ifdef DHCPCALLOUTHOOK_EXPORTS
#define HOOK_API //__declspec(dllexport)
#else
#define HOOK_API __declspec(dllimport)
#endif

HOOK_API DWORD CALLBACK DhcpServerCalloutEntry(
  __in   LPWSTR ChainDlls,
  __in   DWORD CalloutVersion,
  __out  LPDHCP_CALLOUT_TABLE CalloutTbl
);

HOOK_API DWORD CALLBACK DhcpControlHook(
  __in  DWORD dwControlCode,
  __in  LPVOID lpReserved
);

HOOK_API DWORD CALLBACK DhcpNewPktHook(
  __inout  LPBYTE *Packet,
  __inout  DWORD *PacketSize,
  __in     DWORD IpAddress,
  __in     LPVOID Reserved,
  __inout  LPVOID *PktContext,
  __out    LPBOOL ProcessIt
);

HOOK_API DWORD CALLBACK DhcpAddressOfferHook(
  __in  LPBYTE Packet,
  __in  DWORD PacketSize,
  __in  DWORD ControlCode,
  __in  DWORD IpAddress,
  __in  DWORD AltAddress,
  __in  DWORD AddrType,
  __in  DWORD LeaseTime,
  __in  LPVOID Reserved,
  __in  LPVOID PktContext
);

DWORD CALLBACK DhcpAddressDelHook(
  __in  LPBYTE Packet,
  __in  DWORD PacketSize,
  __in  DWORD ControlCode,
  __in  DWORD IpAddress,
  __in  DWORD AltAddress,
  __in  LPVOID Reserved,
  __in  LPVOID PktContext
);

DWORD CALLBACK DhcpDeleteClientHook(
  __in  DWORD IpAddress,
  __in  LPBYTE HwAddress,
  __in  ULONG HwAddressLength,
  __in  DWORD Reserved,
  __in  DWORD ClientType
);

#ifdef __cplusplus
}
#endif
#endif