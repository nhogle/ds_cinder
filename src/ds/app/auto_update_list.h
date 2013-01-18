#pragma once
#ifndef DS_APP_AUTOUPDATELIST_H_
#define DS_APP_AUTOUPDATELIST_H_

#include <vector>

namespace ds {
class AutoUpdate;

/**
 * \class ds::AutoUpdateList
 * Store a collection of auto update objects.
 */
class AutoUpdateList {
  public:
    AutoUpdateList();

    void                      update();

  private:
    friend class AutoUpdate;
    std::vector<AutoUpdate*>  mUpdate;
};

} // namespace ds

#endif // DS_APP_AUTOUPDATELIST_H_