//======================================================================================
/* Athena++ astrophysical MHD code
 * Copyright (C) 2014 James M. Stone  <jmstone@princeton.edu>
 *
 * This program is free software: you can redistribute and/or modify it under the terms
 * of the GNU General Public License (GPL) as published by the Free Software Foundation,
 * either version 3 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A 
 * PARTICULAR PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of GNU GPL in the file LICENSE included in the code
 * distribution.  If not see <http://www.gnu.org/licenses/>.
 *====================================================================================*/

// Primary header
#include "athena.hpp"

// C headers
#include <stdint.h>  // int64_t

// C++ headers
#include <ctime>      // clock(), CLOCKS_PER_SEC, clock_t
#include <exception>  // exception
#include <iomanip>    // setprecision()
#include <iostream>   // cout, endl
#include <new>        // bad_alloc
#include <string>     // string

// Athena headers
#include "mesh.hpp"             // Mesh
#include "parameter_input.hpp"  // ParameterInput

//======================================================================================
/* //////////////////////////////// Athena++ Main Program \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\
 *! \file main.cpp
 *  \brief Athena++ main program file.
 *
 *  Based on the Athena MHD code in C.  History:
 *    - 2003-2013: development of v1.0-v4.2 of Athena code in C
 *    - Jan 2014: start of Athena++ code project
 *====================================================================================*/

//--------------------------------------------------------------------------------------
/*! \fn int main(int argc, char *argv[]) 
 *  \brief Athena++ main program   */

int main(int argc, char *argv[])
{
  std::string athena_version = "version 0.1 - February 2014";
  std::string input_file = "athinput";
  char *prestart_file = NULL;
  char *prundir = NULL;
  int res_flag=0;     // gets set to 1 if -r        argument is on cmdline
  int narg_flag=0;    // gets set to 1 if -n        argument is on cmdline
  int iarg_flag=0;    // gets set to 1 if -i <file> argument is on cmdline

// local variables used for timing and performance measures

  clock_t tstart, tstop;

//--- Step 1. --------------------------------------------------------------------------
// Check for command line options and respond. 

  for (int i=1; i<argc; i++) {

// If argv[i] is a 2 character string of the form "-?" then:

    if(*argv[i] == '-'  && *(argv[i]+1) != '\0' && *(argv[i]+2) == '\0'){
      switch(*(argv[i]+1)) {
      case 'i':                      // -i <input_file>
	input_file = argv[++i];
        iarg_flag = 1;
	break;
      case 'r':                      // -r <restart_file>
	res_flag = 1;
	prestart_file = argv[++i];
	if(iarg_flag) input_file = prestart_file; // use restart if input file not set 
	break;
      case 'd':                      // -d <run_directory>
	prundir = argv[++i];
	break;
      case 'n':
	narg_flag = 1;
	break;
      case 'c':
//	ShowConfig();
	return(0);
	break;
      case 'h':
      default:
        std::cout<<"Athena++ "<< athena_version << std::endl;
        std::cout<<"Last configure: CONFIGURE_DATE"<< std::endl;
        std::cout<<"Usage: "<< argv[0] <<" [options] [block/par=value ...]"<< std::endl;
        std::cout<<"Options:" << std::endl;
        std::cout<<"  -i <file>       specify input file [athinput]"<< std::endl;
        std::cout<<"  -r <file>       restart with this file"<< std::endl;
        std::cout<<"  -d <directory>  specify run dir [current dir]"<< std::endl;
        std::cout<<"  -n              parse input file and quit"<< std::endl;
        std::cout<<"  -c              show configuration and quit"<< std::endl;
        std::cout<<"  -h              this help"<< std::endl;
//	ShowConfig();
        return(0);
	break;
      }
    } // else if argv[i] not of form "-?" ignore it
  }

//--- Step 2. --------------------------------------------------------------------------
// Construct object to store input parameters, then parse input file and command line
// Note memory allocations and parameter input are protected by a simple error handler

  ParameterInput *inputs;
  try {
    inputs = new ParameterInput;
    inputs->LoadFromFile(input_file);
    inputs->ModifyFromCmdline(argc,argv);
  } 
  catch(std::bad_alloc& ba) {
    std::cout << "### FATAL ERROR in main" << std::endl
              << "memory allocation failed initializing class ParameterInput: " 
              << ba.what() << std::endl;
    return(0);
  }
  catch(std::exception const& ex) {
    std::cout << ex.what() << std::endl;  // prints diagnostic message  
    return(0);
  }

// Dump input parameters and quit if code was run with -n option.

  if (narg_flag){
    inputs->ParameterDump(std::cout);
    return(0);
  }

//--- Step 3. --------------------------------------------------------------------------
// Initialize MPI environment, distribute input parameters to all ranks

//  g_comm_world_id = 0;

// Note steps 4-6 are protected by a simple error handler
//--- Step 4. --------------------------------------------------------------------------
// Construct and initialize Mesh

  Mesh *mesh;
  try {
    mesh = new Mesh(inputs);
  }
  catch(std::bad_alloc& ba) {
    std::cout << "### FATAL ERROR in main" << std::endl
              << "memory allocation failed initializing class Mesh: " 
              << ba.what() << std::endl;
    return(0);
  }
  catch(std::exception const& ex) {
    std::cout << ex.what() << std::endl;  // prints diagnostic message
    return(0);
  }

//--- Step 5. --------------------------------------------------------------------------
// Set initial conditions by calling problem generator on each MeshDomain/MeshBlock

  try {
    mesh->ForAllDomains(init_fluid,inputs);
  } 
  catch(std::bad_alloc& ba) {
    std::cout << "### FATAL ERROR in main" << std::endl
              << "memory allocation failed initializing class Fluid: "
              << ba.what() << std::endl;
    return(0);
  }
  catch(std::exception const& ex) {
    std::cout << ex.what() << std::endl;  // prints diagnostic message 
    return(0);
  }

// apply BCs, compute primitive from conserved variables, compute first timestep

  std::cout << "here 1" << std::endl;

  mesh->ForAllDomains( fluid_bcs_n,inputs);
  std::cout << "here 2" << std::endl;
  mesh->ForAllDomains(bfield_bcs_n,inputs);
  mesh->ForAllDomains(primitives_n,inputs);
  std::cout << "here 3" << std::endl;
  mesh->ForAllDomains(new_timestep,inputs);
  std::cout << "here 4" << std::endl;

//--- Step 6. --------------------------------------------------------------------------
// Initialize output object on each Block, and make outputs of initial conditions

  try {
    mesh->ForAllDomains(init_outputs,inputs);
    mesh->ForAllDomains(make_outputs,inputs);
  } 
  catch(std::bad_alloc& ba) {
    std::cout << "### FATAL ERROR in main" << std::endl
              << "memory allocation failed setting initial conditions: " 
              << ba.what() << std::endl;
    return(0);
  }
  catch(std::exception const& ex) {
    std::cout << ex.what() << std::endl;  // prints diagnostic message  
    return(0);
  }
  std::cout << "here 5" << std::endl;

//--- Step 9. === START OF MAIN INTEGRATION LOOP =======================================
// For performance, there is no error handler protecting this step

  std::cout<<std::endl<< "Setup complete, entering main loop..." <<std::endl<<std::endl;
  tstart = clock();

  while ((mesh->time < mesh->tlim) && (mesh->nlim < 0 || mesh->ncycle < mesh->nlim)){
    std::cout << "cycle=" << mesh->ncycle << std::scientific << std::setprecision(5)
              << " time=" << mesh->time << " dt=" << mesh->dt << std::endl;

// predict step

    mesh->ForAllDomains( fluid_predict  ,inputs);
    mesh->ForAllDomains( fluid_bcs_nhalf,inputs);

    mesh->ForAllDomains(bfield_predict  ,inputs);
    mesh->ForAllDomains(bfield_bcs_nhalf,inputs);

    mesh->ForAllDomains(primitives_nhalf,inputs);

// correct step

    mesh->ForAllDomains( fluid_correct,inputs);
    mesh->ForAllDomains( fluid_bcs_n,  inputs);

    mesh->ForAllDomains(bfield_correct,inputs);
    mesh->ForAllDomains(bfield_bcs_n,  inputs);

    mesh->ForAllDomains(primitives_n,  inputs);

// new time step, outputs, diagnostics

    mesh->ncycle++;
    mesh->time  += mesh->dt;

    try {
      mesh->ForAllDomains(make_outputs,inputs);
    } 
    catch(std::bad_alloc& ba) {
      std::cout << "### FATAL ERROR in main" << std::endl
                << "memory allocation failed during outputs: " << ba.what() << std::endl;
      return(0);
    }
    catch(std::exception const& ex) {
      std::cout << ex.what() << std::endl;  // prints diagnostic message  
      return(0);
    }

    mesh->ForAllDomains(new_timestep,inputs);

  } // END OF MAIN INTEGRATION LOOP ====================================================

// print diagnostic messages

  std::cout << "cycle=" << mesh->ncycle << std::scientific << std::setprecision(5)
            << " time=" << mesh->time << " dt=" << mesh->dt << std::endl;
  if (mesh->ncycle == mesh->nlim) {
    std::cout << std::endl << "Terminating on cycle limit" << std::endl;
  } else {
    std::cout << std::endl << "Terminating on time limit" << std::endl;
  }
  std::cout << "time=" << mesh->time << " cycle=" << mesh->ncycle << std::endl;
  std::cout << "tlim=" << mesh->tlim << " nlim=" << mesh->nlim << std::endl;

// Calculate and print the zone-cycles/cpu-second on this processor

  tstop = clock();
  float cpu_time = (tstop>tstart ? (float)(tstop-tstart) : 1.0)/(float)CLOCKS_PER_SEC;
  int64_t zones = (mesh->mesh_size.nx1)*(mesh->mesh_size.nx2)*(mesh->mesh_size.nx3);
  float zcs = (float)(zones*mesh->ncycle)/cpu_time;
  std::cout << std::endl << "cpu time used = " << cpu_time << std::endl;
  std::cout << "zone-cycles/second = " << zcs << std::endl;


//  if(time(&stop_time)>0) /* current calendar time (UTC) is available */
//    ath_pout(0,"\nSimulation terminated on %s",ctime(&stop_time));

  return(0); 

} // END OF MAIN
