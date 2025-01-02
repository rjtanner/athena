#ifndef TASK_LIST_GRAV_TASK_LIST_HPP_
#define TASK_LIST_GRAV_TASK_LIST_HPP_
//========================================================================================
// Athena++ astrophysical MHD code
// Copyright(C) 2014 James M. Stone <jmstone@princeton.edu> and other code contributors
// Licensed under the 3-clause BSD License, see LICENSE file for details
//========================================================================================
//! \file grav_task_list.hpp
//! \brief define GravityBoundaryTaskList

// C headers

// C++ headers
#include <cstdint>      // std::uint64_t

// Athena++ headers
#include "../athena.hpp"
#include "task_list.hpp"

// forward declarations
class Mesh;
class MeshBlock;

//----------------------------------------------------------------------------------------
//! \class GravityBoundaryTaskList
//! \brief data and function definitions for GravityBoundaryTaskList derived class

class GravityBoundaryTaskList : public TaskList {
 public:
  GravityBoundaryTaskList(ParameterInput *pin, Mesh *pm);

  // functions
  TaskStatus ClearGravityBoundary(MeshBlock *pmb, int stage);
  TaskStatus SendGravityBoundary(MeshBlock *pmb, int stage);
  TaskStatus ReceiveGravityBoundary(MeshBlock *pmb, int stage);
  TaskStatus SetGravityBoundary(MeshBlock *pmb, int stage);
  TaskStatus ProlongateGravityBoundary(MeshBlock *pmb, int stage);
  TaskStatus PhysicalBoundary(MeshBlock *pmb, int stage);

 private:
  void AddTask(const TaskID& id, const TaskID& dep) override;
  void StartupTaskList(MeshBlock *pmb, int stage) override;
};


//----------------------------------------------------------------------------------------
//! 64-bit integers with "1" in different bit positions used to ID  each hydro task.
namespace GravityBoundaryTaskNames {
const TaskID NONE(0);
const TaskID CLEAR_GRAV(1);

const TaskID SEND_GRAV_BND(2);
const TaskID RECV_GRAV_BND(3);
const TaskID SETB_GRAV_BND(4);
const TaskID PROLONG_GRAV_BND(5);
const TaskID GRAV_PHYS_BND(6);
}
#endif // TASK_LIST_GRAV_TASK_LIST_HPP_
