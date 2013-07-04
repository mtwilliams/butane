// This file is part of Butane. See README.md and LICENSE.md for details.
// Copyright (c) 2012 Michael Williams <devbug@bitbyte.ca>

#ifndef _BUTANE_SCENE_GRAPH_NODE_H_
#define _BUTANE_SCENE_GRAPH_NODE_H_

/*! */
class Node final {
  public:
    /*! */
    typedef uint8_t Id;

    /*! */
    enum Type {
      /*! An empty, logical node. */
      EMPTY  = 0,
      /*! An orthographic or perspective camera. */
      CAMERA = 1,
      /*! A mesh. */
      MESH   = 2
    };

    /*! */
    typedef Id Link;

    /*! */
    typedef Hash<uint32_t, murmur_hash> Name;

    /*! */
    enum Flags {
      /*! Node::_world_transform will be updated using Node::_local_pose during
          the next SceneGraph::update(). */
      DIRTY = (1u << 0u)
    };

    /*! */
    struct Pose {
      Vec3f position;
      Quatf rotation;
      Vec3f scale;
    };

  public:
    #include <butane/scene_graph/empty.h>
    #include <butane/scene_graph/camera.h>
    #include <butane/scene_graph/mesh.h>

  public:
    struct Serialized final {
      Node::Id id;
      Node::Type type;
      Node::Link link;
      Node::Name name;
      Node::Pose pose;
      union Data {
        // uint8_t empty[sizeof(Node::Empty::Serialized)];
        uint8_t camera[sizeof(Node::Camera::Serialized)];
        uint8_t mesh[sizeof(Node::Mesh::Serialized)];
      } as;
    };

  public:
    Node();

    Node(
      const Serialized& serialized );

    Node(
      const Node& node );

    Node& operator= (
      const Node& node );

    ~Node();

  public:
    /*! */
    SceneGraph& scene_graph() {
      assert(id() != invalid);
      return *((SceneGraph*)((this - sizeof(Node) * id()) - offsetof(SceneGraph, _nodes)));
    }

    /*! */
    const SceneGraph& scene_graph() const {
      assert(id() != invalid);
      return *((const SceneGraph*)((this - sizeof(Node) * id()) - offsetof(SceneGraph, _nodes)));
    }

    /*! */
    static const Id invalid;

    /*! */
    FOUNDATION_INLINE Id id() const
    { return _id; }

    /*! */
    FOUNDATION_INLINE Type type() const
    { return (Type)_type; }

    /*! */
    static const Id unlinked;

    /*! */
    FOUNDATION_INLINE Link link() const {
      assert(id() != invalid);
      return scene_graph().links()[id()];
    }

    /*! */
    FOUNDATION_INLINE Link parent() const
    { return link(); }

    /*! */
    FOUNDATION_INLINE Name name() const {
      assert(id() != invalid);
      return scene_graph().names()[id()];
    }

    /*! */
    FOUNDATION_INLINE uint32_t& flags() {
      assert(id() != invalid);
      return scene_graph().flags()[id()];
    }

    /*! */
    FOUNDATION_INLINE uint32_t flags() const {
      assert(id() != invalid);
      return scene_graph().flags()[id()];
    }

    /*! */
    FOUNDATION_INLINE Pose& local_pose() {
      assert(id() != invalid);
      return scene_graph().local_poses()[id()];
    }

    /*! */
    FOUNDATION_INLINE const Pose& local_pose() const {
      assert(id() != invalid);
      return scene_graph().local_poses()[id()];
    }

    /*! */
    FOUNDATION_INLINE Mat4& world_transform() {
      assert(id() != invalid);
      return scene_graph().world_transforms()[id()];
    }

    /*! */
    FOUNDATION_INLINE const Mat4& world_transform() const {
      assert(id() != invalid);
      return scene_graph().world_transforms()[id()];
    }

  public:
    /*! */
    FOUNDATION_INLINE bool is_empty() const
    { return (type() == EMPTY); }

    /*! */
    FOUNDATION_INLINE Empty* empty() {
      assert(is_empty());
      return (Empty*)&_as;
    }

    /*! */
    FOUNDATION_INLINE const Empty* empty() const {
      assert(is_empty());
      return (const Empty*)&_as;
    }

    /*! */
    FOUNDATION_INLINE bool is_camera() const
    { return (type() == CAMERA); }

    /*! */
    FOUNDATION_INLINE Camera* camera() {
      assert(is_camera());
      return (Camera*)&_as;
    }

    /*! */
    FOUNDATION_INLINE const Camera* camera() const {
      assert(is_camera());
      return (const Camera*)&_as;
    }

    /*! */
    FOUNDATION_INLINE bool is_mesh() const
    { return (type() == MESH); }

    /*! */
    FOUNDATION_INLINE Mesh* mesh() {
      assert(is_mesh());
      return (Mesh*)&_as;
    }

    /*! */
    FOUNDATION_INLINE const Mesh* mesh() const {
      assert(is_mesh());
      return (const Mesh*)&_as;
    }

  public:
    /*! */
    FOUNDATION_INLINE bool is_dirty() const {
      assert(id() != invalid);
      return (scene_graph().flags()[id()] & DIRTY);
    }

  private:
    Node::Id _id;
    Node::Type _type;
    uint8_t __padding[2];
    union Data {
      // uint8_t empty[sizeof(Node::Empty)];
      uint8_t camera[sizeof(Node::Camera)];
      uint8_t mesh[sizeof(Node::Mesh)];
    } _as;
};

#endif // _BUTANE_SCENE_GRAPH_NODE_H_