// Filename: vrpnButton.h
// Created by:  drose (26Jan01)
// 
////////////////////////////////////////////////////////////////////

#ifndef VRPNBUTTON_H
#define VRPNBUTTON_H

#include <pandabase.h>

#include "vrpn_interface.h"

#include <vector>

class VrpnButtonDevice;

////////////////////////////////////////////////////////////////////
//       Class : VrpnButton
// Description : This is the actual interface to a particular VRPN
//               button device, and all of its numbered buttons.  A
//               pointer to this object is stored in the VrpnClient
//               class for each differently-named VRPN button device
//               we connect to.
//
//               The VRPN callbacks go here, which in turn get
//               vectored out to any VrpnButtonDevice objects that
//               register with this.  When the last VrpnButtonDevice
//               object unregisters, the VrpnButton will be deleted
//               by the VrpnClient.
//
//               This class does not need to be exported from the DLL.
////////////////////////////////////////////////////////////////////
class VrpnButton {
public:
  VrpnButton(const string &button_name, vrpn_Connection *connection);
  ~VrpnButton();

  INLINE const string &get_button_name() const;
  INLINE bool is_empty() const;

  void mark(VrpnButtonDevice *device);
  void unmark(VrpnButtonDevice *device);

  INLINE void poll();

  void output(ostream &out) const;
  void write(ostream &out, int indent_level = 0) const;

private:
  static void
  vrpn_button_callback(void *userdata, const vrpn_BUTTONCB info);

private:
  string _button_name;
  vrpn_Button_Remote *_button;

  typedef vector<VrpnButtonDevice *> Devices;
  Devices _devices;
};

INLINE ostream &operator << (ostream &out, const VrpnButton &button) {
  button.output(out);
  return out;
}

#include "vrpnButton.I"

#endif
