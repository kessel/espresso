/*
  Copyright (C) 2010 The ESPResSo project
  Copyright (C) 2002,2003,2004,2005,2006,2007,2008,2009,2010 Max-Planck-Institute for Polymer Research, Theory Group, PO Box 3148, 55021 Mainz, Germany
  
  This file is part of ESPResSo.
  
  ESPResSo is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.
  
  ESPResSo is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
  
  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>. 
*/
/** \file blockfile_tcl.c
    Implementation of \ref blockfile_tcl.h "blockfile_tcl.h".
*/

#include "utils.h"
#include "blockfile.h"
#include "communication.h"

int tclcommand_blockfile(ClientData data, Tcl_Interp *interp,
	      int argc, char *argv[])
{
  char title[MAXBLOCKTITLE];
  char buffer[1024], *name;
  int tcl_file_mode;
  Tcl_Channel channel;
  Tcl_CmdInfo cmdInfo;
  int openbrackets, len, exists;

  if (argc < 4) {
    Tcl_AppendResult(interp, "wrong # args:  should be \"",
		     argv[0], " <file> read|write <what> <param>? ...\"",
		     (char *) NULL);
    return (TCL_ERROR);
  }

  if ((channel = Tcl_GetChannel(interp, argv[1], &tcl_file_mode)) == NULL)
    return (TCL_ERROR);

  /* ------------------ the write commands ------------------------------------ */
  if (!strncmp(argv[2], "write", strlen(argv[2]))) {
    if (!(tcl_file_mode & TCL_WRITABLE)) {
      Tcl_AppendResult(interp, "\"", argv[1], "\" not writeable", (char *) NULL);
      return (TCL_ERROR);
    }

    /* ------------------ write start tag ------------------------------------ */
    if (!strncmp(argv[3], "start", strlen(argv[3]))) {
      if (argc != 5 || strlen(argv[4]) > MAXBLOCKTITLE - 1) {
	Tcl_AppendResult(interp, "please give a short (< 63 bytes) title for your block and nothing more",
			 (char *) NULL);
	return (TCL_ERROR);
      }
      if (block_writestart(channel, argv[4]) != 0) {
	Tcl_AppendResult(interp, "\"", argv[1], "\" could not write start tag",
			 (char *) NULL);
	return (TCL_ERROR);
      }
      return (TCL_OK);
    }
    /* ------------------ write end tag ------------------------------------ */
    else if (!strncmp(argv[3], "end", strlen(argv[3]))) {
      if (argc != 4) {
	Tcl_AppendResult(interp, "\"", argv[0],
			 " write end\" takes no parameters",
			 (char *) NULL);
	return (TCL_ERROR);
      }
      if (block_writeend(channel) != 0) {
	Tcl_AppendResult(interp, "\"", argv[1], "\" could not write end tag",
			 (char *) NULL);
	return (TCL_ERROR);
      }
      return (TCL_OK);
    }
  }
  /* ------------------- the read commands ------------------------------------ */
  else if (!strncmp(argv[2], "read", strlen(argv[2]))) {
    if (!(tcl_file_mode & TCL_READABLE)) {
      Tcl_AppendResult(interp, "\"", argv[1], "\" not readable", (char *) NULL);
      return (TCL_ERROR);
    }
    /* ----------------- read start tag only ------------------------------- */
    if (!strncmp(argv[3], "start", strlen(argv[3]))) {
      if (argc != 4) {
	Tcl_AppendResult(interp, "\"", argv[0],
			 " read start\" takes no parameters",
			 (char *) NULL);
	return (TCL_ERROR);
      }
      if ((openbrackets = block_startread(channel, title)) == 1) {
	Tcl_AppendResult(interp, title, (char *)NULL);	
	return (TCL_OK);
      }
      else {
	Tcl_AppendResult(interp, "\"", argv[1], "\" could not read block start",
			 (char *) NULL);
	return (TCL_ERROR);
      }
    }
    /* -------------- read next block and auto execute or return data --*/
    else if (!strncmp(argv[3], "auto", strlen(argv[3]))) {
      if (argc != 4) {
	Tcl_AppendResult(interp, "\"", argv[0],
			 " read auto\" takes no parameters",
			 (char *) NULL);
	return (TCL_ERROR);
      }
      switch (openbrackets = block_startread(channel, title)) {
      case 0:	
      case 1:
	break;
      case RETURN_CODE_EOF:
	Tcl_AppendResult(interp, "eof", (char *)NULL);
	return (TCL_OK);
      case RETURN_CODE_FILE_FORMAT:
	Tcl_AppendResult(interp, "illstring ", title, (char *)NULL);
	return (TCL_OK);
      default:
	Tcl_AppendResult(interp, "\"", argv[1], "\" could not read block start",
			 (char *) NULL);
	return (TCL_ERROR);
      }
      /* -------------- read unknown field ---------------------------- */
      len = 21; /* blockfile_read_auto_ + \0 */
      len += strlen(title);
      name = malloc(len);
      strcpy(name, "blockfile_read_auto_");
      strcat(name, title);
      exists = Tcl_GetCommandInfo(interp, name, &cmdInfo);
      free(name);

      if (exists) {
	int err = cmdInfo.proc(cmdInfo.clientData, interp,
			       argc, argv);
	return mpi_gather_runtime_errors(interp, err);
      }

      Tcl_AppendResult(interp, "usertag ", title, (char *) NULL);
      if (openbrackets != 0) {
	Tcl_AppendResult(interp, " ", (char *) NULL);	  
	while ((openbrackets =
		block_continueread(channel, openbrackets,
				   buffer, sizeof(buffer), 0)) > 0) {
	  Tcl_AppendResult(interp, buffer, (char *) NULL);
	}
	if (openbrackets < 0) {
	  Tcl_AppendResult(interp, "\"", argv[1], "\" could not read data",
			   (char *) NULL);
	  return (TCL_ERROR);
	}
	Tcl_AppendResult(interp, buffer, (char *) NULL);
      }
      return (TCL_OK);	
    }
    /* ------------------- read until end tag ------------------------------ */
    else if (!strncmp(argv[3], "toend", strlen(argv[3]))) {
      if (argc != 4) {
	Tcl_AppendResult(interp, "\"", argv[0],
			 " read  toend\" takes no parameters",
			 (char *) NULL);
	return (TCL_ERROR);
      }
      openbrackets = 1;
      while ((openbrackets =
	      block_continueread(channel, openbrackets,
				 buffer, sizeof(buffer), 0)) > 0) {
	Tcl_AppendResult(interp, buffer, (char *) NULL);
      }
      if (openbrackets < 0) {
	Tcl_AppendResult(interp, "\"", argv[1], "\" could not read data",
			 (char *) NULL);
	return (TCL_ERROR);
      }
      Tcl_AppendResult(interp, buffer, (char *) NULL);
      return (TCL_OK);
    }
    /* -------- (probably) read any tag implemented in tcl ----------------- */
    else {
      len = 21; /* blockfile_read_auto_ + \0 */
      len += strlen(argv[3]);
      name = malloc(len);
      strcpy(name, "blockfile_read_auto_");
      strcat(name, argv[3]);
      exists = Tcl_GetCommandInfo(interp, name, &cmdInfo);
      free(name);
      if (exists) {
	int err;
	if (!((openbrackets = block_startread(channel, title)) == 1) ||
	    strncmp(title,argv[3], strlen(argv[3]))) {
	  Tcl_AppendResult(interp, "\"", argv[1], "\" did not contain block\"", argv[3],"\" you indicated",
			   (char *) NULL);
	  return (TCL_ERROR);
	}

        err = cmdInfo.proc(cmdInfo.clientData, interp, argc, argv);
	return mpi_gather_runtime_errors(interp, err);
      }
    }
  }

  /* not a native action, try script support */
  len = 12; /* blockfile_ + _ + \0 */
  len += strlen(argv[2]) + strlen(argv[3]);
  name = malloc(len);
  strcpy(name, "blockfile_");
  strcat(name, argv[2]);
  strcat(name, "_");
  strcat(name, argv[3]);
  exists = Tcl_GetCommandInfo(interp, name, &cmdInfo);
  free(name);
  if (exists) {
    int err = cmdInfo.proc(cmdInfo.clientData, interp, argc, argv);
    return mpi_gather_runtime_errors(interp, err);
  }
  Tcl_AppendResult(interp, "unknown action \"", argv[2], " ", argv[3],
		   "\"", (char *)NULL);  
  return (TCL_ERROR);
}
