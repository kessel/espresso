

cimport numpy as np
import numpy as np
import particle_data
cimport particle_data 
import global_variables
import pmi
#cimport myconfig
#import utils

#public enum:
#  ERROR=-1


#### This is the main Espresso Cython interface.

### Now come all the includes

cdef int instance_counter 

### Here we make a minimalistic Tcl_Interp available
cdef extern from "tcl.h":
  cdef struct Tcl_Interp:
    char *result
    int errorLine
    
  Tcl_Interp* Tcl_CreateInterp()
  int Tcl_Eval (Tcl_Interp * interp, char* script)
  ctypedef Tcl_Interp* Interp_pointer


## Let's import on_program_start
cdef extern from *:
  void mpi_init(int* argc, char*** argv)
  int this_node
  int n_nodes
  void mpi_stop()

cdef mpi_init_espresso():
  set_this_node(pmi.rank)
  set_n_nodes(pmi.size)

cdef extern from "../../src/initialize.h":
  void on_program_start(Tcl_Interp*)
  void mpi_loop()


## Now comes the real deal
cdef class EspressoHandle:
  cdef Tcl_Interp* interp
  cdef public int this_node
  def __init__(self):
    global instance_counter
    if instance_counter >= 1:
      raise Exception("Espresso shall only be instanciated once!")
    else:
      instance_counter+=1
      mpi_init_espresso()
      self.this_node=this_node
      set_reqest_handler(cython_request_handler)
      if this_node==0:
        self.interp = Tcl_CreateInterp() 
        self.Tcl_Eval('global argv; set argv ""')
        self.Tcl_Eval('set tcl_interactive 0')
        on_program_start(self.interp)
      else:
#        set_reqest_handler(<request_handler_type>NULL)
        on_program_start(NULL)

  def __del__(self):
    self.die()
    raise Exception("Espresso can not be deleted")

  def Tcl_Eval(self, string):
    result=Tcl_Eval(self.interp, string)
    if result:
      raise Exception("Tcl reports an error", self.interp.result)
    return self.interp.result

  def die(self):
    mpi_stop()

cdef extern from "communication.h":
  ctypedef void (*request_handler_type)(int* req)
  ctypedef void (*SlaveCallback)(int node, int param)
  void set_reqest_handler(request_handler_type _request_handler)
  void set_this_node(int _this_node) 
  void set_n_nodes(int _n_nodes)
  SlaveCallback slave_callbacks[]
  char* names[]
  void mpi_bcast_parameter(int n)

cdef void c_interpret_request(int* request):
#  print "%d this is cython trying to invoke %s %d %d!" % (pmi.rank, names[request[0]],request[1], request[2])
  slave_callbacks[request[0]](request[1], request[2]);

def interpret_request(req):
#  print "%d is interpreting request %d" % ( pmi.rank, req[0] )
  cdef int temp[3]
  temp[0]=req[0]
  temp[1]=req[1]
  temp[2]=req[2]
  if pmi.rank != 0:
    c_interpret_request(temp)

def testfun(asdf):
#  print "testfun calling from %d!" % pmi.rank
  print asdf

cdef void cython_request_handler(int* req):
#  print "%d You reqest the issue %s %d %d"  %( pmi.rank, names[req[0]], req[1], req[2] )
  temp_request =  [ req[0], req[1], req[2] ]
  python_request_handler(temp_request)

def python_request_handler(req):
#  print "Let's try to make the others work"
  pmi.call(interpret_request, req)
#  pmi.call(testfun, 5)

glob=global_variables.GlobalsHandle()
part=particle_data.particleList()

def create_Espresso():
  global _espressoHandle
#  print "%d is creating Espresso" % pmi.rank
  _espressoHandle=EspressoHandle()
#  print "%d is ready" % pmi.rank

pmi.startWorkerLoop()
pmi.registerAtExit()


pmi.call(create_Espresso)
mpi_bcast_parameter(19)
if this_node==0:
  glob=global_variables.GlobalsHandle()
else:
  exit()

#  print "I am a python callback"
#  print "You Reqested ", cb, node, param
#
#
#      
#  def __init__(self, id):
#    self.id=id
#   





