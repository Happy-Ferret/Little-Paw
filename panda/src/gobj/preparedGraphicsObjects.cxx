// Filename: preparedGraphicsObjects.cxx
// Created by:  drose (19Feb04)
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

#include "preparedGraphicsObjects.h"
#include "textureContext.h"
#include "mutexHolder.h"

#ifndef CPPPARSER
PStatCollector PreparedGraphicsObjects::_total_texusage_pcollector("Texture usage");
#endif  // CPPPARSER

////////////////////////////////////////////////////////////////////
//     Function: PreparedGraphicsObjects::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
PreparedGraphicsObjects::
PreparedGraphicsObjects() {
}

////////////////////////////////////////////////////////////////////
//     Function: PreparedGraphicsObjects::Destructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
PreparedGraphicsObjects::
~PreparedGraphicsObjects() {
  // There may be texture objects that are still prepared when we
  // destruct.  If this is so, then all of the GSG's that own them
  // have already destructed, so we can assume their resources were
  // internally cleaned up.  Quietly erase these remaining textures.

  MutexHolder holder(_lock);

  Textures::iterator tci;
  for (tci = _prepared_textures.begin();
       tci != _prepared_textures.end();
       ++tci) {
    TextureContext *tc = (*tci);
    _total_texusage_pcollector.sub_level(tc->estimate_texture_memory());
    tc->_texture->clear_prepared(this);
  }

  _prepared_textures.clear();
  _released_textures.clear();
  _enqueued_textures.clear();
}


////////////////////////////////////////////////////////////////////
//     Function: PreparedGraphicsObjects::enqueue_texture
//       Access: Public
//  Description: Indicates that a texture would like to be put on the
//               list to be prepared when the GSG is next ready to
//               do this (presumably at the next frame).
////////////////////////////////////////////////////////////////////
void PreparedGraphicsObjects::
enqueue_texture(Texture *tex) {
  MutexHolder holder(_lock);

  _enqueued_textures.insert(tex);
}

////////////////////////////////////////////////////////////////////
//     Function: PreparedGraphicsObjects::dequeue_texture
//       Access: Public
//  Description: Removes a texture from the queued list of textures to
//               be prepared.  Normally it is not necessary to call
//               this, unless you change your mind about preparing it
//               at the last minute, since the texture will
//               automatically be dequeued and prepared at the next
//               frame.
//
//               The return value is true if the texture is
//               successfully dequeued, false if it had not been
//               queued.
////////////////////////////////////////////////////////////////////
bool PreparedGraphicsObjects::
dequeue_texture(Texture *tex) {
  MutexHolder holder(_lock);

  EnqueuedTextures::iterator qi = _enqueued_textures.find(tex);
  if (qi != _enqueued_textures.end()) {
    _enqueued_textures.erase(qi);
    return true;
  }
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: PreparedGraphicsObjects::release_texture
//       Access: Public
//  Description: Indicates that a texture context, created by a
//               previous call to prepare_texture(), is no longer
//               needed.  The driver resources will not be freed until
//               some GSG calls update(), indicating it is at a
//               stage where it is ready to release textures--this
//               prevents conflicts from threading or multiple GSG's
//               sharing textures (we have no way of knowing which
//               graphics context is currently active, or what state
//               it's in, at the time release_texture is called).
////////////////////////////////////////////////////////////////////
void PreparedGraphicsObjects::
release_texture(TextureContext *tc) {
  MutexHolder holder(_lock);

  tc->_texture->clear_prepared(this);
  _total_texusage_pcollector.sub_level(tc->estimate_texture_memory());

  // We have to set the Texture pointer to NULL at this point, since
  // the Texture itself might destruct at any time after it has been
  // released.
  tc->_texture = (Texture *)NULL;

  bool removed = (_prepared_textures.erase(tc) != 0);
  nassertv(removed);

  _released_textures.insert(tc);
}

////////////////////////////////////////////////////////////////////
//     Function: PreparedGraphicsObjects::release_all_textures
//       Access: Public
//  Description: Releases all textures at once.  This will force them
//               to be reloaded into texture memory for all GSG's that
//               share this object.  Returns the number of textures
//               released.
////////////////////////////////////////////////////////////////////
int PreparedGraphicsObjects::
release_all_textures() {
  MutexHolder holder(_lock);

  int num_textures = (int)_prepared_textures.size();

  Textures::iterator tci;
  for (tci = _prepared_textures.begin();
       tci != _prepared_textures.end();
       ++tci) {
    TextureContext *tc = (*tci);
    tc->_texture->clear_prepared(this);
    _total_texusage_pcollector.sub_level(tc->estimate_texture_memory());
    tc->_texture = (Texture *)NULL;

    _released_textures.insert(tc);
  }

  _prepared_textures.clear();

  return num_textures;
}

////////////////////////////////////////////////////////////////////
//     Function: PreparedGraphicsObjects::prepare_texture_now
//       Access: Public
//  Description: Immediately creates a new TextureContext for the
//               indicated texture and returns it.  This assumes that
//               the GraphicsStateGuardian is the currently active
//               rendering context and that it is ready to accept new
//               textures.  If this is not necessarily the case, you
//               should use enqueue_texture() instead.
//
//               Normally, this function is not called directly.  Call
//               Texture::prepare_now() instead.
//
//               The TextureContext contains all of the pertinent
//               information needed by the GSG to keep track of this
//               one particular texture, and will exist as long as the
//               texture is ready to be rendered.
//
//               When either the Texture or the
//               PreparedGraphicsObjects object destructs, the
//               TextureContext will be deleted.
////////////////////////////////////////////////////////////////////
TextureContext *PreparedGraphicsObjects::
prepare_texture_now(Texture *tex, GraphicsStateGuardianBase *gsg) {
  MutexHolder holder(_lock);

  // Ask the GSG to create a brand new TextureContext.  There might
  // be several GSG's sharing the same set of textures; if so, it
  // doesn't matter which of them creates the context (since they're
  // all shared anyway).
  TextureContext *tc = gsg->prepare_texture(tex);

  bool prepared = _prepared_textures.insert(tc).second;
  nassertr(prepared, tc);

  _total_texusage_pcollector.add_level(tc->estimate_texture_memory());
  return tc;
}

////////////////////////////////////////////////////////////////////
//     Function: PreparedGraphicsObjects::update
//       Access: Public
//  Description: This is called by the GraphicsStateGuardian to
//               indicate that it is in a state to load or release
//               textures.
//
//               Any texture contexts that were previously passed to
//               release_texture() are actually passed to the GSG to
//               be freed at this point; textures that were previously
//               passed to prepare_texture are actually loaded.
////////////////////////////////////////////////////////////////////
void PreparedGraphicsObjects::
update(GraphicsStateGuardianBase *gsg) {
  MutexHolder holder(_lock);

  // First, release all the textures awaiting release.
  Textures::iterator tci;
  for (tci = _released_textures.begin();
       tci != _released_textures.end();
       ++tci) {
    TextureContext *tc = (*tci);
    gsg->release_texture(tc);
  }

  _released_textures.clear();

  // Now prepare all the textures awaiting preparation.
  EnqueuedTextures::iterator qi;
  for (qi = _enqueued_textures.begin();
       qi != _enqueued_textures.end();
       ++qi) {
    Texture *tex = (*qi);
    tex->prepare_now(this, gsg);
  }

  _enqueued_textures.clear();
}
