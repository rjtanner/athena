#ifndef OUTPUTS_HPP
#define OUTPUTS_HPP
//======================================================================================
/* Athena++ astrophysical MHD code
 * Copyright (C) 2014 James M. Stone  <jmstone@princeton.edu>
 * See LICENSE file for full public license information.
 *====================================================================================*/
/*! \file outputs.hpp
 *  \brief provides multiple classes to handle ALL types of data output (fluid, bfield,
 *  gravity, radiation, particles, etc.)
 *====================================================================================*/

class Mesh;
class ParameterInput;

//! \struct OutputDataHeader
//  \brief metadata describing OutputData contained within an OutputType

struct OutputDataHeader {
  std::string descriptor; // time, cycle, variables in output
  std::string transforms; // list of any transforms (sum, slice) applied to variables
  int il,iu,jl,ju,kl,ku;  // range of data arrays
};

//! \struct OutputVariableHeader
//  \brief metadata describing each OutputVariable contained in OutputData class

struct OutputVariableHeader {
  std::string type; // one of (SCALARS,VECTORS)
  std::string name;
};

//! \struct OutputParameters
//  \brief  control parameter values read from <output> block in the input file

struct OutputParameters {
  Real next_time, dt;
  int block_number;
  int file_number;
  int islice, jslice, kslice;
  int isum, jsum, ksum;
  std::string block_name;
  std::string file_basename;
  std::string file_id;
  std::string variable;
  std::string file_type;
  std::string data_format;
};

//! \class OutputVariable
//  \brief node in a linked list of output variables in OutputData class

class OutputVariable {
public:
  OutputVariable(AthenaArray<Real> *parray, OutputVariableHeader vhead);
  ~OutputVariable();

  OutputVariableHeader var_header;
  AthenaArray<Real> *pdata;

  OutputVariable *pnext, *pprev; // ptrs to next and previous nodes in list
};

//! \class OutputData
//  \brief container for output data, composed of a header (metadata describing output),
//  a linked list of OutputVariables, and functions to add and replace nodes

class OutputData {
public:
  OutputData();
  ~OutputData();

  OutputDataHeader data_header;
  OutputVariable *pfirst_var;  // ptr to first variable (node) in linked list
  OutputVariable *plast_var;   // ptr to last  variable (node) in linked list

  void AppendNode(AthenaArray<Real> *parray, OutputVariableHeader vhead);
  void ReplaceNode(OutputVariable *pold, OutputVariable *pnew);
};


//-------------------------- OutputTypes base and derived classes ----------------------
//! \class OutputType
//  \brief abstract base class for different output types (modes), designed to be a node
//  in a linked list created and stored in objects of the Outputs class.

class OutputType {
public:
  OutputType(OutputParameters oparams, MeshBlock *pmb);
  ~OutputType();
  MeshBlock *pmy_block;           // ptr to MeshBlock containing this OutputType
  OutputParameters output_params; // control data read from <output> block 
  OutputType *pnext;              // ptr to next node in linked list of OutputTypes

// functions that operate on OutputData container

  virtual void LoadOutputData(OutputData *pod);
  virtual void TransformOutputData(OutputData *pod);
  virtual void WriteOutputFile(OutputData *pod) = 0;  // pure virtual function!

// functions that implement useful transforms applied to each variable in OutputData

  void Slice(OutputData* pod, int dim);
  void Sum(OutputData* pod, int dim);
};

//! \class FormattedTableOutput
//  \brief derived OutputType class for formatted table (tabular) data

class FormattedTableOutput : public OutputType {
public:
  FormattedTableOutput(OutputParameters oparams, MeshBlock *pmb);
  ~FormattedTableOutput() {};

  void WriteOutputFile(OutputData *pod);

private:
};

//! \class HistoryOutput
//  \brief derived OutputType class for history dumps

class HistoryOutput : public OutputType {
public:
  HistoryOutput(OutputParameters oparams, MeshBlock *pmb);
  ~HistoryOutput() {};

  void LoadOutputData(OutputData *pod);  // overloads base class function
  void WriteOutputFile(OutputData *pod);
};

//! \class VTKOutput
//  \brief derived OutputType class for vtk dumps

class VTKOutput : public OutputType {
public:
  VTKOutput(OutputParameters oparams, MeshBlock *pmb);
  ~VTKOutput() {};

  void WriteOutputFile(OutputData *pod);
};
//--------------------- end of OutputTypes base and derived classes --------------------

//! \class Outputs
//  \brief root class for all Athena++ outputs.  Provides a linked list of OutputTypes,
//  with each node representing one mode of output to be made during a simulation.

class Outputs {
public:
  Outputs(MeshBlock *pmb, ParameterInput *pin);
  ~Outputs();
  MeshBlock *pmy_block;  // ptr to MeshBlock containing this Outputs

  void InitOutputTypes(ParameterInput *pin);
  void MakeOutputs();

private:
  OutputType *pfirst_out_; // ptr to first OutputType in linked list
};
#endif
