// This file is part of Butane. See README.md and LICENSE.md for details.
// Copyright (c) 2012 Michael Williams <devbug@bitbyte.ca>

#ifndef _BUTANE_RESOURCE_ID_H_
#define _BUTANE_RESOURCE_ID_H_

class Id final
  : public foundation::Hash<uint64_t>
{
  public:
    Id(
      const Type& type,
      const char* path );

    Id(
      const Type& type,
      const String& path );

    Id(
      const Id& id );

    Id& operator= (
      const Id& id );

  public:
    operator uint64_t() const override;

  private:
    uint64_t _hash;
};

#endif // _BUTANE_RESOURCE_ID_H_
