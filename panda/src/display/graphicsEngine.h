// Filename: graphicsEngine.h
// Created by:  drose (24Feb02)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#ifndef GRAPHICSENGINE_H
#define GRAPHICSENGINE_H

#include "pandabase.h"
#include "graphicsWindow.h"
#include "graphicsBuffer.h"
#include "frameBufferProperties.h"
#include "graphicsThreadingModel.h"
#include "sceneSetup.h"
#include "pointerTo.h"
#include "thread.h"
#include "pmutex.h"
#include "conditionVar.h"
#include "pset.h"
#include "pStatCollector.h"

class Pipeline;
class DisplayRegion;
class GraphicsPipe;
class FrameBufferProperties;
class Texture;

////////////////////////////////////////////////////////////////////
//       Class : GraphicsEngine
// Description : This class is the main interface to controlling the
//               render process.  There is typically only one
//               GraphicsEngine in an application, and it synchronizes
//               rendering to all all of the active windows; although
//               it is possible to have multiple GraphicsEngine
//               objects if multiple synchronicity groups are
//               required.
//
//               The GraphicsEngine is responsible for managing the
//               various cull and draw threads.  The application
//               simply calls engine->render_frame() and considers it
//               done.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA GraphicsEngine {
PUBLISHED:
  GraphicsEngine(Pipeline *pipeline = NULL);
  ~GraphicsEngine();

  void set_frame_buffer_properties(const FrameBufferProperties &properties);
  FrameBufferProperties get_frame_buffer_properties() const; 

  void set_threading_model(const GraphicsThreadingModel &threading_model);
  GraphicsThreadingModel get_threading_model() const;

  INLINE void set_auto_flip(bool auto_flip);
  INLINE bool get_auto_flip() const;

  INLINE PT(GraphicsStateGuardian) make_gsg(GraphicsPipe *pipe);
  PT(GraphicsStateGuardian) make_gsg(GraphicsPipe *pipe,
                                     const FrameBufferProperties &properties,
                                     const GraphicsThreadingModel &threading_model);

  INLINE GraphicsWindow *make_window(GraphicsPipe *pipe,
                                     GraphicsStateGuardian *gsg);
  GraphicsWindow *make_window(GraphicsPipe *pipe,
                              GraphicsStateGuardian *gsg,
                              const GraphicsThreadingModel &threading_model);
  INLINE GraphicsBuffer *make_buffer(GraphicsPipe *pipe,
                                     GraphicsStateGuardian *gsg,
                                     int x_size, int y_size, bool want_texture);
  GraphicsBuffer *make_buffer(GraphicsPipe *pipe,
                              GraphicsStateGuardian *gsg,
                              int x_size, int y_size, bool want_texture,
                              const GraphicsThreadingModel &threading_model);

  bool remove_window(GraphicsOutput *window);
  void remove_all_windows();
  void reset_all_windows(bool swapchain);
  bool is_empty() const;

  void render_frame();
  void sync_frame();
  void flip_frame();
  
  void render_subframe(GraphicsStateGuardian *gsg, DisplayRegion *dr,
                       bool cull_sorting);

public:
  enum ThreadState {
    TS_wait,
    TS_do_frame,
    TS_do_flip,
    TS_do_release,
    TS_terminate
  };

private:
  typedef pset< PT(GraphicsOutput) > Windows;
  typedef pset< PT(GraphicsStateGuardian) > GSGs;

  void cull_and_draw_together(const Windows &wlist);
  void cull_and_draw_together(GraphicsStateGuardian *gsg, DisplayRegion *dr);

  void cull_bin_draw(const Windows &wlist);
  void cull_bin_draw(GraphicsStateGuardian *gsg, DisplayRegion *dr);

  void process_events(const GraphicsEngine::Windows &wlist);
  void flip_windows(const GraphicsEngine::Windows &wlist);
  void do_sync_frame();
  void do_flip_frame();
  INLINE void close_gsg(GraphicsPipe *pipe, GraphicsStateGuardian *gsg);

  PT(SceneSetup) setup_scene(const NodePath &camera, 
                             GraphicsStateGuardian *gsg);
  void do_cull(CullHandler *cull_handler, SceneSetup *scene_setup,
               GraphicsStateGuardian *gsg);
  void do_draw(CullResult *cull_result, SceneSetup *scene_setup,
               GraphicsStateGuardian *gsg, DisplayRegion *dr);

  bool setup_gsg(GraphicsStateGuardian *gsg, SceneSetup *scene_setup);

  void do_add_window(GraphicsOutput *window, GraphicsStateGuardian *gsg,
                     const GraphicsThreadingModel &threading_model);
  void do_remove_window(GraphicsOutput *window);
  void terminate_threads();

  // The WindowRenderer class records the stages of the pipeline that
  // each thread (including the main thread, a.k.a. "app") should
  // process, and the list of windows for each stage.
  class WindowRenderer {
  public:
    void add_gsg(GraphicsStateGuardian *gsg);
    void add_window(Windows &wlist, GraphicsOutput *window);
    void remove_window(GraphicsOutput *window);
    void do_frame(GraphicsEngine *engine);
    void do_flip(GraphicsEngine *engine);
    void do_release(GraphicsEngine *engine);
    void do_close(GraphicsEngine *engine);
    void do_pending(GraphicsEngine *engine);
    bool any_done_gsgs() const;

    Windows _cull;    // cull stage
    Windows _cdraw;   // cull-and-draw-together stage
    Windows _draw;    // draw stage
    Windows _window;  // window stage, i.e. process windowing events 
    Windows _pending_release; // moved from _draw, pending release_gsg.
    Windows _pending_close;   // moved from _window, pending close.
    GSGs _gsgs;       // draw stage
    Mutex _wl_lock;
  };

  class RenderThread : public Thread, public WindowRenderer {
  public:
    RenderThread(const string &name, GraphicsEngine *engine);
    virtual void thread_main();

    GraphicsEngine *_engine;
    Mutex _cv_mutex;
    ConditionVar _cv;
    ThreadState _thread_state;
  };

  WindowRenderer *get_window_renderer(const string &name);

  Pipeline *_pipeline;
  Windows _windows;

  WindowRenderer _app;
  typedef pmap<string, PT(RenderThread) > Threads;
  Threads _threads;
  FrameBufferProperties _frame_buffer_properties;
  GraphicsThreadingModel _threading_model;
  bool _auto_flip;

  enum FlipState {
    FS_draw,  // Still drawing.
    FS_sync,  // All windows are done drawing.
    FS_flip,  // All windows are done drawing and have flipped.
  };
  FlipState _flip_state;
  Mutex _lock;

  static PStatCollector _app_pcollector;
  static PStatCollector _yield_pcollector;
  static PStatCollector _cull_pcollector;
  static PStatCollector _draw_pcollector;
  static PStatCollector _sync_pcollector;
  static PStatCollector _flip_pcollector;
  static PStatCollector _transform_states_pcollector;
  static PStatCollector _transform_states_unused_pcollector;
  static PStatCollector _render_states_pcollector;
  static PStatCollector _render_states_unused_pcollector;
  friend class WindowRenderer;
};

#include "graphicsEngine.I"

#endif

