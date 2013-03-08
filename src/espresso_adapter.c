
#include "espresso_view.h"
#include "particle_data.h"
#include "cells.h"

EspressoAdapter::EspressoAdapter() {
  r_required=false;
}

void EspressoAdapter::update() {
  r.resize(cells_get_n_particles());
  Cell *cell ;
  unsigned int np;
  Particle* p;
  for (int c=0;c<local_cells.n;c++) {
    cell = local_cells.cell[c] ;
    p = cell->part ;
    np = cell->n ;
    for (unsigned int i=0;i<np;i++) { 
     	r[i]=p[i].r.p[0];
    }
  }


}
