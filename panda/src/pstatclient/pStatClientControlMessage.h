// Filename: pStatClientControlMessage.h
// Created by:  drose (09Jul00)
// 
////////////////////////////////////////////////////////////////////

#ifndef PSTATCLIENTCONTROLMESSAGE_H
#define PSTATCLIENTCONTROLMESSAGE_H

#include <pandabase.h>

#include "pStatCollectorDef.h"

#include <vector>

class Datagram;
class PStatClientVersion;

////////////////////////////////////////////////////////////////////
// 	 Class : PStatClientControlMessage
// Description : This kind of message is sent from the client to the
//               server on the TCP socket to establish critical
//               control information.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA PStatClientControlMessage {
public:
  PStatClientControlMessage();

  void encode(Datagram &datagram) const;
  bool decode(const Datagram &datagram, PStatClientVersion *version);

  enum Type {
    T_datagram = 0,
    T_hello,
    T_define_collectors,
    T_define_threads,
    T_invalid
  };

  Type _type;

  // Used for T_hello
  string _client_hostname;
  string _client_progname;
  int _major_version;
  int _minor_version;

  // Used for T_define_collectors
  vector<PStatCollectorDef *> _collectors;

  // Used for T_define_threads
  int _first_thread_index;
  vector<string> _names;
};


#endif

