// Filename: clientBase.h
// Created by:  jason (04Aug00)
// 
////////////////////////////////////////////////////////////////////

#include "clientBase.h"
#include "config_device.h"

TypeHandle ClientBase::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: ClientBase::constructor
//       Access: Protected
//  Description: 
////////////////////////////////////////////////////////////////////
ClientBase::
ClientBase() {
  _forked = false;
  _last_poll_time = 0.0;
  _last_poll_frame = 0;

#ifdef HAVE_IPC
  _client_thread = (thread *)NULL;
  _shutdown = false;
#endif
}


////////////////////////////////////////////////////////////////////
//     Function: ClientBase::destructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
ClientBase::
~ClientBase() {
  // We have to disconnect all of our devices before destructing.
  Devices::iterator di;
  Devices devices_copy = _devices;
  for (di = devices_copy.begin(); di != devices_copy.end(); ++di) {
    DevicesByName &dbn = (*di).second;
    DevicesByName::iterator dbni;
    for (dbni = dbn.begin(); dbni != dbn.end(); ++dbni) {
      ClientDevice *device = (*dbni).second;
      device->disconnect();
    }
  }

#ifdef HAVE_IPC
  if (_forked) {
    _shutdown = true;

    // Join the loader thread - calling process blocks until the loader
    // thread returns.
    void *ret;
    _client_thread->join(&ret);
  }
#endif
}

////////////////////////////////////////////////////////////////////
//     Function: ClientBase::fork_asynchronous_thread
//       Access: Public
//  Description: Forks a separate thread to do all the polling of
//               connected devices.  The forked thread will poll after
//               every poll_time seconds has elapsed.  Returns true if
//               the fork was successful, or false otherwise (for
//               instance, because we were already forked, or because
//               asynchronous threads are disabled).
////////////////////////////////////////////////////////////////////
bool ClientBase::
fork_asynchronous_thread(double poll_time) {
#ifdef HAVE_IPC
  if (_forked) {
    device_cat.error()
      << "Attempt to fork client thread twice.\n";
    return false;
  }

  if (asynchronous_clients) {
    _sleep_time = (int)(1000000 * poll_time);

    _client_thread = thread::create(&st_callback, this);
    _forked = true;
    if (device_cat.is_debug()) {
      device_cat.debug()
	<< "fork_asynchronous_thread() - forking client thread"
	<< endl;
    }
    return true;
  }
#endif

  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: ClientBase::get_device
//       Access: Public
//  Description: Returns a ClientDevice pointer that corresponds to
//               the named device of the indicated device type.  The
//               device_type should be one of ClientTrackerDevice,
//               ClientAnalogDevice, etc.; the device_name is
//               implementation defined.
//
//               Normally, the user does not need to call this
//               function directly; it is called automatically by
//               creating a TrackerNode or AnalogNode or some such
//               data graph node.
//
//               The return value is the pointer to the created device
//               (which might be the same pointer returned by a
//               previous call to this function with the same
//               parameters).  When the pointer destructs (i.e. its
//               reference count reaches zero) it will automatically
//               be disconnected.
//
//               If the named device does not exist or cannot be
//               connected for some reason, NULL is returned.
////////////////////////////////////////////////////////////////////
PT(ClientDevice) ClientBase::
get_device(TypeHandle device_type, const string &device_name) {
  DevicesByName &dbn = _devices[device_type];

  DevicesByName::iterator dbni;
  dbni = dbn.find(device_name);
  if (dbni != dbn.end()) {
    // This device was previously connected.  Return it again.
    return (*dbni).second;
  }

  // We need to create a new device for this name.
  PT(ClientDevice) device = make_device(device_type, device_name);

  if (device != (ClientDevice *)NULL) {
    dbn.insert(DevicesByName::value_type(device_name, device));
    device->_is_connected = true;
  }

  return device;
}

////////////////////////////////////////////////////////////////////
//     Function: ClientBase::disconnect_device
//       Access: Protected, Virtual
//  Description: Removes the device, which is presumably about to
//               destruct, from the list of connected devices, and
//               frees any data required to support it.  This device
//               will no longer receive automatic updates with each
//               poll.
//
//               The return value is true if the device was
//               disconnected, or false if it was unknown (e.g. it was
//               disconnected previously).
////////////////////////////////////////////////////////////////////
bool ClientBase::
disconnect_device(TypeHandle device_type, const string &device_name,
		  ClientDevice *device) {
  DevicesByName &dbn = _devices[device_type];

  DevicesByName::iterator dbni;
  dbni = dbn.find(device_name);
  if (dbni != dbn.end()) {
    if ((*dbni).second == device) {
      // We found it!
      dbn.erase(dbni);
      return true;
    }
  }

  // The device was unknown.
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: ClientBase::do_poll
//       Access: Protected, Virtual
//  Description: Implements the polling and updating of connected
//               devices, if the ClientBase requires this.  This may
//               be called in a sub-thread if
//               fork_asynchronous_thread() was called; otherwise, it
//               will be called once per frame.
////////////////////////////////////////////////////////////////////
void ClientBase::
do_poll() {
  ClockObject *global_clock = ClockObject::get_global_clock();
  _last_poll_frame = global_clock->get_frame_count();
  _last_poll_time = global_clock->get_time();
}

#ifdef HAVE_IPC
////////////////////////////////////////////////////////////////////
//     Function: ClientBase::st_callback
//       Access: Private, static
//  Description: Call back function for thread (if thread has been
//               spawned).  A call back function must be static, so
//               this merely calls the non-static member callback In 
//               addition, the function has a void* return type even
//               though we don't actually return anything.  This is
//               necessary because ipc assumes a function that does 
//               not return anything indicates that the associated 
//               thread should  be created as unjoinable (detached).
////////////////////////////////////////////////////////////////////
void *ClientBase::
st_callback(void *arg) {
  nassertr(arg != NULL, NULL);
  ((ClientBase *)arg)->callback();
  return NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: ClientBase::callback
//       Access: Private
//  Description: This is the main body of the sub-thread.  It sleeps
//               a certain time and then polls all devices currently
//               being watched
////////////////////////////////////////////////////////////////////
void ClientBase::
callback() {
  while (true) {
    if (_shutdown) {
      break;
    }
    do_poll();
    ipc_traits::sleep(0, _sleep_time);
  }
}
#endif // HAVE_IPC
