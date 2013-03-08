
#include "particle_data.h"
#include <vector>

typedef double Vec3;

class EspressoAdapter {
  public:
    EspressoAdapter();

    int size() { return r.size(); }

  /* Positions! */
    std::vector<Vec3> r;
    std::vector<Vec3>& getR() { return r; }
    bool r_required;
    void require_r() { r_required = true; }

    void update(); 
};
