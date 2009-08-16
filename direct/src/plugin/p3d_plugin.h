/* Filename: p3d_plugin.h
 * Created by:  drose (28May09)
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef P3D_PLUGIN_H
#define P3D_PLUGIN_H

/* This file defines the C-level API to Panda's core plugin API.  This
   API is intended to provide basic functionality for loading and
   running Panda's .p3d files, particularly within a browser.

   This core API is intended to be loaded and run within a browser, as
   a standalone DLL.  It will in turn be responsible for fetching and
   installing the appropriate version of Panda and Python, as well as
   any required supporting libraries.

   Note that this code defines only the interface between the actual
   browser plugin and the Panda code.  It contains no code to directly
   interface with any browser.  The actual plugin itself will be a
   separate piece of code, written in ActiveX or NPAPI or whatever API
   is required for a given browser; and this code will be designed to
   download and link with this DLL.

   The browser or launching application will be referred to as the
   "host" in this documentation.  The host should load this core API
   DLL only once, but may then use it to create multiple simultaneous
   different instances of Panda windows.

   Filenames passed through this interface are in native OS-specific
   form, e.g. with a leading drive letter and backslashes, not in
   Panda's Unix-like form (except on Unix-based OSes, of course).
*/

#include <sys/types.h>

#ifdef _WIN32
#include <windows.h>

#ifdef BUILDING_P3D_PLUGIN
#define EXPCL_P3D_PLUGIN __declspec(dllexport)
#else
#define EXPCL_P3D_PLUGIN __declspec(dllimport)
#endif

#else  /* _WIN32 */
#define EXPCL_P3D_PLUGIN

#endif  /* _WIN32 */

#ifdef __APPLE__
#include <Carbon/Carbon.h>
#endif  /* __APPLE__ */

#ifdef __cplusplus
extern "C" {
#endif

/* In the following function prototypes, all functions are declared
   initially as typedefs only, and then the actual function references
   are finally declared at the end of this file, but only if
   P3D_PLUGIN_PROTOTYPES is defined.  This is intended to allow
   including this file without building an implicit reference to the
   functions themselves, allowing the core API library to be loaded
   via an explicit LoadLibrary() or equivalent call. */


/* This symbol serves to validate that runtime and compile-time
   libraries match.  It should be passed to P3D_initialize()
   (below). This number will be incremented whenever there are changes
   to any of the interface specifications defined in this header
   file. */
#define P3D_API_VERSION 7

/************************ GLOBAL FUNCTIONS **************************/

/* The following interfaces are global to the core API space, as
   opposed to being specific to a particular instance. */

/* This function should be called immediately after the core API is
   loaded.  You should pass P3D_API_VERSION as the first parameter, so
   the DLL can verify that it has been built with the same version of
   the API as the host.

   The contents_filename, if not NULL or empty, names a contents.xml
   file that has already been downloaded and verified from the server.
   If this is NULL, a new file will be downloaded as needed.

   If download_url is not NULL or empty, it specifies the root URL of
   the download server; otherwise, the compiled-in default is used.

   If platform is not NULL or empty, it specifies the current platform
   string; otherwise, the compiled-in default is used.

   This function returns true if the core API is valid and uses a
   compatible API, false otherwise.  If it returns false, the host
   should not call any more functions in this API, and should
   immediately unload the DLL and (if possible) download a new one. */
typedef bool 
P3D_initialize_func(int api_version, const char *contents_filename,
                    const char *download_url, const char *platform);

/* This function should be called to unload the core API.  It will
   release all internally-allocated memory and return the core API to
   its initial state. */
typedef void
P3D_finalize_func();

/********************** INSTANCE MANAGEMENT **************************/

/* The following interfaces define the API to manage individual
   Panda3D instances.  Each instance can display a separate 3-D
   graphics window simultaneously on the host or on the desktop.  The
   instances operate generally independently of each other. */

/* This structure defines the handle to a single instance.  The host
   may access any members appearing here. */
typedef struct {
  /* true if the host should call P3D_instance_get_request().*/
  bool _request_pending;

  /* an opaque pointer the host may use to store private data that the
     core API does not interpret.  This pointer can be directly set, or
     it can be initialized in the P3D_new_instance() call. */
  void *_user_data;

  /* Additional opaque data may be stored here. */
} P3D_instance;

/* This structure abstracts out the various window handle types for
   the different platforms. */
typedef struct {
#ifdef _WIN32
  HWND _hwnd;

#elif defined(__APPLE__)
  GrafPtr _port;

#elif defined(HAVE_X11)
  unsigned long _xwindow;
  void *_xdisplay;
#endif
} P3D_window_handle;

/* This enum lists the different kinds of window types that may be
   requested for the instance.  These define the way that the instance
   will create its main Panda3D window.  The instance will treat this
   as a request only; it is always free to create whatever kind of
   window it likes. */
typedef enum {
  /* Embedded: the Panda window is embedded within the host window.
     This is the normal kind of window for an object embedded within a
     browser page.  Pass a valid window handle in for parent_window,
     and valid coordinates on the parent window for win_x, win_y,
     win_width, win_height. */
  P3D_WT_embedded,

  /* Toplevel: the Panda window is a toplevel window on the user's
     desktop.  Pass valid desktop coordinates in for win_x, win_y,
     win_width, and win_height.  If all of these are zero, the core
     API will create a window wherever it sees fit. */
  P3D_WT_toplevel,

  /* Fullscreen: the Panda window is a fullscreen window, completely
     overlaying the entire screen and changing the desktop resolution.
     Pass a valid desktop size in for win_width and win_height (win_x
     and win_y are ignored).  If win_width and win_height are zero,
     the core API will create a fullscreen window of its own preferred
     size. */
  P3D_WT_fullscreen,

  /* Hidden: there is no window at all for the instance. */
  P3D_WT_hidden,

} P3D_window_type;


/* This function pointer must be passed to P3D_new_instance(), below.
   The host must pass in a pointer to a valid function in the host's
   address space, or NULL.  If not NULL, this function will be called
   asynchronously by the core API when it needs to make a request from
   the host.  After this notification has been received, the host
   should call P3D_instance_get_request() (at its convenience) to
   retrieve the actual Panda request.  If the host passes NULL for
   this function pointer, asynchronous notifications will not be
   provided, and the host must be responsible for calling
   P3D_instance_get_request() from time to time. */

/* Note that, unlike the other func typedefs in this header file, this
   declaration is not naming a function within the core API itself.
   Instead, it is a typedef for a function pointer that must be
   supplied by the host. */

typedef void
P3D_request_ready_func(P3D_instance *instance);

/* This structure is used to represent a single keyword/value pair
   that appears within the embed syntax on the HTML page.  An array of
   these values is passed to the P3D instance to represent all of the
   additional keywords that may appear within this syntax; it is up to
   the Panda instance to interpret these additional keywords
   correctly. */
typedef struct {
  const char *_keyword;
  const char *_value;
} P3D_token;

/* This function creates a new Panda3D instance.  

   For tokens, pass an array of P3D_token elements (above), which
   correspond to the user-supplied keyword/value pairs that may appear
   in the embed token within the HTML syntax; the host is responsible
   for allocating this array, and for deallocating it after this call
   (the core API will make its own copy of the array).  The tokens are
   passed to the application, who is free to decide how to interpret
   them; they have no meaning at the system level.

   The user_data pointer is any arbitrary pointer value; it will be
   copied into the _user_data member of the new P3D_instance object.
   This pointer is intended for the host to use to store private data
   associated with each instance; the core API will not do anything with
   this data.
 */

typedef P3D_instance *
P3D_new_instance_func(P3D_request_ready_func *func, 
                      const P3D_token tokens[], size_t num_tokens,
                      void *user_data);

/* This function should be called at some point after
   P3D_new_instance(); it actually starts the instance running.
   Before this call, the instance will be in an indeterminate state.

   If is_local is true, then p3d_filename contains the name of a local
   p3d file on disk.  If false, then p3d_filename contains a URL that
   should be downloaded to retrieve the p3d file.

   The return value is true on success, false on failure. */
typedef bool
P3D_instance_start_func(P3D_instance *instance, bool is_local,
                        const char *p3d_filename);


/* Call this function to interrupt a particular instance and stop it
   from rendering, for instance when the user navigates away from the
   page containing it.  After calling this function, you should not
   reference the P3D_instance pointer again. */
typedef void 
P3D_instance_finish_func(P3D_instance *instance);

/* Call this function after creating an instance in order to set its
   window size and placement.  You must call this at least once in
   order to actually manifest the instance onscreen.  This may also be
   called to reposition a window after its initial placement (e.g. if
   the browser window has changed width and needs reformatting). */
typedef void
P3D_instance_setup_window_func(P3D_instance *instance,
                               P3D_window_type window_type,
                               int win_x, int win_y,
                               int win_width, int win_height,
                               P3D_window_handle parent_window);


/********************** SCRIPTING SUPPORT **************************/

/* The following interfaces are provided to support controlling the
   Panda instance via JavaScript or related interfaces on the browser. */

/* We require an "object" that contains some number of possible
   different interfaces.  An "object" might be a simple primitive like
   an int, float, or string; or it might be a class object with
   methods and properties.  Instances of P3D_object are passed around
   as parameters into and return values from functions.
  
   To implement a P3D_object, we need to first define a class
   definition, which is a table of methods.  Most classes are defined
   internally by the core API, but the host must define at least one
   class type as well, which provides callbacks into host-provided
   objects.

   These function types define the methods available on a class.
   These are function type declarations only; they do not correspond
   to named functions within the core API DLL.  Instead, the function
   pointers themselves are stored within the P3D_class_definition
   structure, below. */

/* A forward declaration of P3D_object. */
typedef struct _P3D_object P3D_object;

/* A list of fundamental object types. */
typedef enum {
  P3D_OT_undefined,
  P3D_OT_none,
  P3D_OT_bool,
  P3D_OT_int,
  P3D_OT_float,
  P3D_OT_string,
  P3D_OT_object,
} P3D_object_type;

/* Methods and functions that return a P3D_object return it with its
   reference count already incremented by one for the benefit of the
   caller, leaving the caller the owner of the implicit reference
   count.  This is referred to as returning a "new reference", using
   the Python naming convention.  Similarly, methods that receive a
   P3D_object as a parameter will generally *not* adjust that object's
   reference count (except to increment it the object is stored
   internally), leaving the caller still the owner of the original
   reference.  Thus, it is the caller's responsibility to call
   P3D_OBJECT_DECREF() on any P3D_objects it has received but no
   longer wishes to keep. */

/* This method is called internally to deallocate the object and all
   of its internal structures.  Do not call it directly; call
   P3D_OBJECT_DECREF() instead. */
typedef void
P3D_object_finish_method(P3D_object *object);

/* Returns the fundamental type of the object.  This should be treated
   as a hint to suggest how the object can most accurately be
   represented; it does not limit the actual interfaces available to
   an object.  For instance, you may call P3D_OBJECT_GET_PROPERTY()
   even if the object's type is not "object". */
typedef P3D_object_type
P3D_object_get_type_method(P3D_object *object);

/* Each of the following methods returns the object's value expressed
   as the corresponding type.  If the object is not precisely that
   type, a coercion is performed. */

/* Return the object as a bool. */
typedef bool
P3D_object_get_bool_method(P3D_object *object);

/* Return the object as an integer. */
typedef int
P3D_object_get_int_method(P3D_object *object);

/* Return the object as a floating-point number. */
typedef double
P3D_object_get_float_method(P3D_object *object);

/* Get the object as a string.  This method copies the string into the
   provided buffer, and returns the actual length of the internal
   string (not counting any terminating null character).  If the
   return value is larger than buffer_size, the string has been
   truncated.  If it is equal, there is no null character written to
   the buffer (like strncpy).  You may call this method first with
   buffer = NULL and buffer_size = 0 to return just the required
   size of the buffer. */
typedef int
P3D_object_get_string_method(P3D_object *object, 
                             char *buffer, int buffer_size);

/* As above, but instead of the literal object data, returns a
   user-friendly reprensentation of the object as a string.  For
   instance, a string object returns the string itself to
   P3D_OBJECT_GET_STRING(), but returns the string with quotation
   marks and escape characters from P3D_OBJECT_GET_REPR().
   Mechanically, this function works the same way as get_string(). */
typedef int
P3D_object_get_repr_method(P3D_object *object, 
                           char *buffer, int buffer_size);

/* Looks up a property on the object by name, i.e. a data member or a
   method.  The return value is a new-reference P3D_object if the
   property exists, or NULL if it does not. */
typedef P3D_object *
P3D_object_get_property_method(P3D_object *object, const char *property);

/* Changes the value at the indicated property.  The new value is
   assigned to the indicating property, and its reference count is
   correspondingly incremented.  Any existing object previously
   assigned to the corresponding property is replaced, and its
   reference count decremented.  If the value pointer is NULL, the
   property is removed altogether.  Returns true on success, false on
   failure.  */
typedef bool
P3D_object_set_property_method(P3D_object *object, const char *property,
                               P3D_object *value);

/* Returns true if the indicated method name exists on the object,
   false otherwise.  In the Python case, this actually returns true if
   the object has the indicated property, and that property represents
   a callable object.  If method_name is empty or NULL, returns true
   if the object itself is callable. */
typedef bool
P3D_object_has_method_method(P3D_object *object, const char *method_name);

/* Invokes a named method on the object.  If method_name is empty or
   NULL, invokes the object itself as a function.  You must pass an
   array of P3D_objects as the list of parameters, and ownership of
   these objects' reference counts is not transferred with the call
   (you must still DECREF these objects afterwards).

   If needs_response is true, the return value is a new-reference
   P3D_object on success, or NULL on failure.  If needs_response is
   false, the return value is always NULL, and there is no way to
   determine success or failure. */
typedef P3D_object *
P3D_object_call_method(P3D_object *object, const char *method_name,
                       bool needs_response,
                       P3D_object *params[], int num_params);

/* Evaluates an arbitrary JavaScript expression in the context of the
   object.  Python objects do not support this method.

   The return value is a new-reference P3D_object on success, or NULL
   on failure. */
typedef P3D_object *
P3D_object_eval_method(P3D_object *object, const char *expression);

/* This defines the class structure that implements all of the above
   methods. */
typedef struct _P3D_class_definition {
  P3D_object_finish_method *_finish;

  P3D_object_get_type_method *_get_type;
  P3D_object_get_bool_method *_get_bool;
  P3D_object_get_int_method *_get_int;
  P3D_object_get_float_method *_get_float;
  P3D_object_get_string_method *_get_string;
  P3D_object_get_repr_method *_get_repr;

  P3D_object_get_property_method *_get_property;
  P3D_object_set_property_method *_set_property;

  P3D_object_has_method_method *_has_method;
  P3D_object_call_method *_call;
  P3D_object_eval_method *_eval;

} P3D_class_definition;

/* And this structure defines the actual instances of P3D_object. */
struct _P3D_object {
  const P3D_class_definition *_class;
  int _ref_count;

  /* Additional opaque data may be stored here. */
};

/* These macros are defined for the convenience of invoking any of the
   above method functions on an object.

   CAUTION!  None of these macros are thread-safe; you may use them
   only in a single-threaded application (or when only a single thread
   of the application makes any calls into this API).  For thread-safe
   variants, see the similarly-named function calls below. */

#define P3D_OBJECT_GET_TYPE(object) ((object)->_class->_get_type((object)))
#define P3D_OBJECT_GET_BOOL(object) ((object)->_class->_get_bool((object)))
#define P3D_OBJECT_GET_INT(object) ((object)->_class->_get_int((object)))
#define P3D_OBJECT_GET_FLOAT(object) ((object)->_class->_get_float((object)))
#define P3D_OBJECT_GET_STRING(object, buffer, buffer_size) ((object)->_class->_get_string((object), (buffer), (buffer_size)))
#define P3D_OBJECT_GET_REPR(object, buffer, buffer_size) ((object)->_class->_get_repr((object), (buffer), (buffer_size)))

#define P3D_OBJECT_GET_PROPERTY(object, property) ((object)->_class->_get_property((object), (property)))
#define P3D_OBJECT_SET_PROPERTY(object, property, value) ((object)->_class->_set_property((object), (property), (value)))

#define P3D_OBJECT_HAS_METHOD(object, method_name) ((object)->_class->_has_method((object), (method_name)))
#define P3D_OBJECT_CALL(object, method_name, needs_response, params, num_params) ((object)->_class->_call((object), (method_name), (needs_response), (params), (num_params)))
#define P3D_OBJECT_EVAL(object, expression) ((object)->_class->_eval((object), (expression)))

/* These macros are provided to manipulate the reference count of the
   indicated object.  As above, these macros are NOT thread-safe.  If
   you need a thread-safe reference count adjustment, see the
   similarly-named function calls below.

   Following Python's convention, XDECREF is provided to decrement the
   reference count for a pointer that might be NULL (it does nothing
   in the case of a NULL pointer). */

#define P3D_OBJECT_INCREF(object) (++(object)->_ref_count)
#define P3D_OBJECT_DECREF(object) { if (--(object)->_ref_count <= 0) { (object)->_class->_finish((object)); } }
#define P3D_OBJECT_XDECREF(object) { if ((object) != (P3D_object *)NULL) { P3D_OBJECT_DECREF(object); } }

/* Use these functions for thread-safe variants of the above macros. */
typedef P3D_object_type
P3D_object_get_type_func(P3D_object *object);
typedef bool
P3D_object_get_bool_func(P3D_object *object);
typedef int
P3D_object_get_int_func(P3D_object *object);
typedef double
P3D_object_get_float_func(P3D_object *object);
typedef int
P3D_object_get_string_func(P3D_object *object, char *buffer, int buffer_size);
typedef int
P3D_object_get_repr_func(P3D_object *object, char *buffer, int buffer_size);
typedef P3D_object *
P3D_object_get_property_func(P3D_object *object, const char *property);
typedef bool
P3D_object_set_property_func(P3D_object *object, const char *property, 
                             P3D_object *value);
typedef bool
P3D_object_has_method_func(P3D_object *object, const char *method_name);
typedef P3D_object *
P3D_object_call_func(P3D_object *object, const char *method_name, 
                     bool needs_response,
                     P3D_object *params[], int num_params);
typedef P3D_object *
P3D_object_eval_func(P3D_object *object, const char *expression);

/* A NULL pointer passed into either incref or decref is safe and will
   be quietly ignored. */
typedef void 
P3D_object_incref_func(P3D_object *object);
typedef void 
P3D_object_decref_func(P3D_object *object);


/* End of method pointer definitions.  The following function types
   are once again meant to define actual function pointers to be found
   within the core API DLL. */

/* Returns a new P3D_class_definition object, filled with generic
   function pointers that have reasonable default behavior for all
   methods.  The host should use this function to get a clean
   P3D_class_definition object before calling
   P3D_instance_set_browser_script_object() (see below).  Note that
   this pointer will automatically be freed when P3D_finalize() is
   called. */
typedef P3D_class_definition *
P3D_make_class_definition_func();

/* Returns a new-reference P3D_object of type "undefined".  This
   corresponds to the undefined or void type on JavaScript.  It is
   similar to Python's None, but has a subtly different shade of
   meaning; we map it to an explicit Undefined instance in
   runp3d.py. */
typedef P3D_object *
P3D_new_undefined_object_func();

/* Returns a new-reference P3D_object of type none.  This value has no
   particular value and corresponds to Python's None type or
   JavaScript's null type. */
typedef P3D_object *
P3D_new_none_object_func();

/* Returns a new-reference P3D_object of type bool. */
typedef P3D_object *
P3D_new_bool_object_func(bool value);

/* Returns a new-reference P3D_object of type int. */
typedef P3D_object *
P3D_new_int_object_func(int value);

/* Returns a new-reference P3D_object of type float. */
typedef P3D_object *
P3D_new_float_object_func(double value);

/* Returns a new-reference P3D_object of type string.  The supplied
   string is copied into the object and stored internally. */
typedef P3D_object *
P3D_new_string_object_func(const char *string, int length);

/* Returns a new-reference pointer to the top-level scriptable object
   of the instance.  Scripts running on the host may use this object
   to communicate with the instance, by using the above methods to set
   or query properties, and/or call methods, on the instance. */
typedef P3D_object *
P3D_instance_get_panda_script_object_func(P3D_instance *instance);

/* The inverse functionality: this supplies an object pointer to the
   instance to allow the Panda instance to control the browser.  In
   order to enable browser scriptability, the host must call this
   method shortly after creating the instance, preferably before
   calling P3D_instance_start().

   The object parameter must have been created by the host.  It will
   have a custom P3D_class_definition pointer, which also must have
   been created by the host.  The best way to create an appropriate
   class definition is call P3D_make_class_definition(), and then
   replace the function pointers for at least _finish, _get_property,
   _set_property, _has_method, and _call.  Set these pointers to the
   host's own functions that make the appropriate changes in the DOM,
   or invoke the appropriate JavaScript functions.

   If this function is never called, the instance will not be able to
   make outcalls to the DOM or to JavaScript, but scripts may still be
   able to control the instance via P3D_instance_get_panda_script_object(),
   above. 

   Note that the object's constructor should initialize its reference
   count to 1.  The instance will increment reference count as a
   result of this call; the caller is responsible for calling DECREF
   on the object after this call to remove its own reference. */
typedef void
P3D_instance_set_browser_script_object_func(P3D_instance *instance, 
                                            P3D_object *object);


/********************** REQUEST HANDLING **************************/

/* The core API may occasionally have an asynchronous request to pass
   up to the host.  The following structures implement this interface.
   The design is intended to support single-threaded as well as
   multi-threaded implementations in the host; there is only the one
   callback function, P3D_request_ready (above), which may be called
   asynchronously by the core API.  The host should be careful that
   this callback function is protected from mutual access.  The
   callback function implementation may be as simple as setting a flag
   that the host will later check within its main processing loop.

   Once P3D_request_ready() has been received, the host should call
   P3D_instance_get_request() to query the nature of the request.
   This call may be made synchronously, i.e. within the host's main
   processing loop.  After each request is serviced, the host should
   release the request via P3D_request_finish() and then call
   P3D_instance_get_request() again until that function returns NULL.

   The requests themselves are implemented via a hierarchy of structs.
   Each request is stored in a different kind of struct, allowing the
   different requests to store a variety of data.  An enumerated value
   indicates the particular request type retrieved. */

/* This represents the type of a request returned by
   P3D_instance_get_request.  More types may be added later. */
typedef enum {
  P3D_RT_stop,
  P3D_RT_get_url,
  P3D_RT_post_url,
  P3D_RT_notify,
  P3D_RT_refresh,
} P3D_request_type;

/* Structures corresponding to the request types in the above enum. */

/* A stop request.  The instance would like to stop itself.  No
   additional data is required.  The host should respond by calling
   P3D_instance_finish(). */
typedef struct {
} P3D_request_stop;

/* A get_url request.  The core API would like to retrieve data for a
   particular URL.  The core API is responsible for supplying a valid
   URL string, and a unique integer ID.  The unique ID is needed to
   feed the results of the URL back to the core API.  If possible, the
   host should be prepared to handle multiple get_url requests in
   parallel, but it is allowed to handle them all one at a time if
   necessary.  As data comes in from the url, the host should call
   P3D_instance_feed_url_stream().
*/
typedef struct {
  const char *_url;
  int _unique_id;
} P3D_request_get_url;

/* A post_url request.  Similar to get_url, but additional data is to
   be sent via POST to the indicated URL.  The result of the POST is
   returned in a mechanism similar to get_url.
*/
typedef struct {
  const char *_url;
  const char *_post_data;
  size_t _post_data_size;
  int _unique_id;
} P3D_request_post_url;

/* A general notification.  This is just a message of some event
   having occurred within the Panda3D instance.  It may be safely
   ignored.
*/
typedef struct {
  const char *_message;
} P3D_request_notify;

/* A refresh request.  The instance would like to get a repaint event
   on its own window.  Only relevant for windowless plugins, e.g. on
   OSX. */
typedef struct {
} P3D_request_refresh;

/* This is the overall structure that represents a single request.  It
   is returned by P3D_instance_get_request(). */
typedef struct {
  P3D_instance *_instance;
  P3D_request_type _request_type;
  union {
    P3D_request_stop _stop;
    P3D_request_get_url _get_url;
    P3D_request_post_url _post_url;
    P3D_request_notify _notify;
    P3D_request_refresh _refresh;
  } _request;
} P3D_request;

/* After a call to P3D_request_ready(), or from time to time in
   general, the host should call this function to see if there are any
   pending requests from the core API.  The function will return a
   freshly-allocated request if there is a request ready, or NULL if
   there are no requests.  After a receipt of P3D_request_ready(),
   the host should call this function repeatedly until it returns NULL
   (there might be multiple requests for a single receipt of
   P3D_request_ready()).  Each request should be processed, then
   released via P3D_request_finish(). */
typedef P3D_request *
P3D_instance_get_request_func(P3D_instance *instance);

/* This method may also be used to test whether a request is ready.
   If any open instance has a pending request, this function will
   return a pointer to one of them (which you may then pass to
   P3D_instance_get_request_func).  If no instances have a pending
   request, this function will return NULL.  If wait is true, this
   function will never return NULL unless there are no instances open;
   instead, it will wait indefinitely until there is a request
   available.

   Note that, due to race conditions, it is possible for this function
   to return a P3D_instance that does not in fact have any requests
   pending (another thread may have checked the request first).  You
   should always verify that the return value of
   P3D_instance_get_request() is not NULL. */
typedef P3D_instance *
P3D_check_request_func(bool wait);

/* A request retrieved by P3D_instance_get_request() should eventually
   be passed here, after it has been handled, to deallocate its
   resources and prevent a memory leak.  The 'handled' flag should be
   passed true if the host has handled the request, or false if it has
   ignored it (e.g. because it does not implement support for this
   particular request type).  After calling this function, you should
   not reference the P3D_request pointer again. */
typedef void
P3D_request_finish_func(P3D_request *request, bool handled);

/* This code is passed to P3D_instance_feed_url_stream, below, as data
   is retrieved from the URL. */
typedef enum {
  /* in progress: the query is still in progress, and another call
     will be made in the future. */
  P3D_RC_in_progress,

  /* done: the query is done, and all data has been retrieved without
     error.  This call represents the last of the data. */
  P3D_RC_done,

  /* generic_error: some error other than an HTTP error has occurred,
     for instance, lack of connection to the server, or malformed URL.
     No more data will be forthcoming. */
  P3D_RC_generic_error,

  /* An HTTP error has occurred, for instance 404.  The particular
     status code will be supplied in the http_status_code parameter.
     There may or may not be data associated with this error as well.
     However, no more data will be delivered after this call. */
  P3D_RC_http_error,
} P3D_result_code;

/* This function is used by the host to handle a get_url request,
   above.  As it retrieves data from the URL, it should call this
   function from time to time to feed that data to the core API.

   instance and unique_id are from the original get_url() request.

   result_code and http_status_code indicates the current status of
   the request, as described above; the call will be made again in the
   future if its result_code is P3D_RC_in_progress.

   total_expected_data represents the host's best guess at the total
   amount of data that will be retrieved.  It is acceptable if this
   guess doesn't match the actual data received at all.  Set it to 0
   if the host has no idea.  This value may change from one call to
   the next.

   this_data and this_data_size describe the most recent block of data
   retrieved from the URL.  Each chunk of data passed to this function
   is appended together by the core API to define the total set of
   data retrieved from the URL.  For a particular call to
   feed_url_stream, this may contain no data at all
   (e.g. this_data_size may be 0).

   The return value of this function is true if there are no problems
   and the download should continue, false if there was an error
   accepting the data and the host should abort.  If this function
   returns false on a P3D_RC_in_progress, there is no need to call
   the function again with any future updates.
 */
typedef bool
P3D_instance_feed_url_stream_func(P3D_instance *instance, int unique_id,
                                  P3D_result_code result_code,
                                  int http_status_code, 
                                  size_t total_expected_data,
                                  const void *this_data, 
                                  size_t this_data_size);

/* This structure abstracts out the event pointer data types for the
   different platforms, as passed to P3D_instance_handle_event(),
   below. */
typedef struct {
#ifdef _WIN32
  // Not used for Win32.

#elif defined(__APPLE__)
  EventRecord *_event;

#elif defined(HAVE_X11)
  //  XEvent *_event; ?
#endif
} P3D_event_data;

/* Use this function to supply a new os-specific window event to the
   plugin.  This is presently used only on OSX, where the window
   events are needed for proper plugin handling; and possibly also on
   X11.  On Windows, window events are handled natively by the plugin.

   The return value is true if the handler has processed the event,
   false if it has been ignored. */
typedef bool
P3D_instance_handle_event_func(P3D_instance *instance, P3D_event_data event);

#ifdef P3D_FUNCTION_PROTOTYPES

/* Define all of the actual prototypes for the above functions. */
EXPCL_P3D_PLUGIN P3D_initialize_func P3D_initialize;
EXPCL_P3D_PLUGIN P3D_finalize_func P3D_finalize;

EXPCL_P3D_PLUGIN P3D_new_instance_func P3D_new_instance;
EXPCL_P3D_PLUGIN P3D_instance_start_func P3D_instance_start;
EXPCL_P3D_PLUGIN P3D_instance_finish_func P3D_instance_finish;
EXPCL_P3D_PLUGIN P3D_instance_setup_window_func P3D_instance_setup_window;

EXPCL_P3D_PLUGIN P3D_object_get_type_func P3D_object_get_type;
EXPCL_P3D_PLUGIN P3D_object_get_bool_func P3D_object_get_bool;
EXPCL_P3D_PLUGIN P3D_object_get_int_func P3D_object_get_int;
EXPCL_P3D_PLUGIN P3D_object_get_float_func P3D_object_get_float;
EXPCL_P3D_PLUGIN P3D_object_get_string_func P3D_object_get_string;
EXPCL_P3D_PLUGIN P3D_object_get_repr_func P3D_object_get_repr;
EXPCL_P3D_PLUGIN P3D_object_get_property_func P3D_object_get_property;
EXPCL_P3D_PLUGIN P3D_object_set_property_func P3D_object_set_property;
EXPCL_P3D_PLUGIN P3D_object_has_method_func P3D_object_has_method;
EXPCL_P3D_PLUGIN P3D_object_call_func P3D_object_call;
EXPCL_P3D_PLUGIN P3D_object_eval_func P3D_object_eval;
EXPCL_P3D_PLUGIN P3D_object_incref_func P3D_object_incref;
EXPCL_P3D_PLUGIN P3D_object_decref_func P3D_object_decref;

EXPCL_P3D_PLUGIN P3D_make_class_definition_func P3D_make_class_definition;
EXPCL_P3D_PLUGIN P3D_new_undefined_object_func P3D_new_undefined_object;
EXPCL_P3D_PLUGIN P3D_new_none_object_func P3D_new_none_object;
EXPCL_P3D_PLUGIN P3D_new_bool_object_func P3D_new_bool_object;
EXPCL_P3D_PLUGIN P3D_new_int_object_func P3D_new_int_object;
EXPCL_P3D_PLUGIN P3D_new_float_object_func P3D_new_float_object;
EXPCL_P3D_PLUGIN P3D_new_string_object_func P3D_new_string_object;
EXPCL_P3D_PLUGIN P3D_instance_get_panda_script_object_func P3D_instance_get_panda_script_object;
EXPCL_P3D_PLUGIN P3D_instance_set_browser_script_object_func P3D_instance_set_browser_script_object;

EXPCL_P3D_PLUGIN P3D_instance_get_request_func P3D_instance_get_request;
EXPCL_P3D_PLUGIN P3D_check_request_func P3D_check_request;
EXPCL_P3D_PLUGIN P3D_request_finish_func P3D_request_finish;
EXPCL_P3D_PLUGIN P3D_instance_feed_url_stream_func P3D_instance_feed_url_stream;
EXPCL_P3D_PLUGIN P3D_instance_handle_event_func P3D_instance_handle_event;

#endif  /* P3D_FUNCTION_PROTOTYPES */

#ifdef __cplusplus
};  /* end of extern "C" */
#endif

#endif  /* P3D_PLUGIN_H */


