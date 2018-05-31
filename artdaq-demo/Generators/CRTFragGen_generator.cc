/* Author: Matthew Strait <mstrait@fnal.gov> */

#include "artdaq-demo/Generators/CRTFragGen.hh"

#include "canvas/Utilities/Exception.h"

#include "artdaq/Application/GeneratorMacros.hh"
#include "artdaq-core/Utilities/SimpleLookupPolicy.hh"

#include "fhiclcpp/ParameterSet.h"

#include <fstream>
#include <iomanip>
#include <iterator>
#include <iostream>

#include <unistd.h>
#include "cetlib_except/exception.h"

CRT::FragGen::FragGen(fhicl::ParameterSet const& ps) :
    CommandableFragmentGenerator(ps)
  , hardware_interface_(new CRTInterface(ps))
  , timestamp_(0)
  , readout_buffer_(nullptr)
{
  hardware_interface_->AllocateReadoutBuffer(&readout_buffer_);
}

CRT::FragGen::~FragGen()
{
  hardware_interface_->FreeReadoutBuffer(readout_buffer_);
}

bool CRT::FragGen::getNext_(
  std::list< std::unique_ptr<artdaq::Fragment> > & frags)
{
  if(should_stop()) return false;

  std::size_t bytes_read = 0;
  hardware_interface_->FillBuffer(readout_buffer_, &bytes_read);

  if(bytes_read == 0){
    // Pause for a little bit if we didn't get anything to keep load down.
    usleep(1000);
    return true; // this means "keep taking data"
  }

  assert(sizeof timestamp_ == 8);

  // A module packet must at least have the magic number (1B), hit count
  // (1B), module number (2B) and timestamps (8B).
  const std::size_t minsize = 4 + sizeof(timestamp_);
  if(bytes_read < 4 + sizeof(timestamp_)){
    fprintf(stderr, "Bad result with only %lu < %lu bytes from "
            "CRTInterface::FillBuffer.\n", bytes_read, minsize);
    return false; // means "stop taking data"
  }

  // The Unix time stamp concatenated with the 50MHz counter
  memcpy(&timestamp_, readout_buffer_ + 4, sizeof(timestamp_));

  std::unique_ptr<artdaq::Fragment> fragptr(
    // See $ARTDAQ_DIR/Data/Fragment.hh
    artdaq::Fragment::FragmentBytes(
      bytes_read,
      ev_counter(), // from base CommandableFragmentGenerator
      fragment_id(), // ditto
      artdaq::Fragment::FirstUserFragmentType, // only one
      0, // metadata.  We have none.
      timestamp_
  ));

  frags.emplace_back(std::move(fragptr));

  memcpy(frags.back()->dataBeginBytes(), readout_buffer_, bytes_read);

  if (metricMan /* What is this? */ != nullptr)
    metricMan->sendMetric("Fragments Sent", ev_counter(), "Events", 3,
        artdaq::MetricMode::LastPoint);

  ev_counter_inc(); // from base CommandableFragmentGenerator

  return true;
}

void CRT::FragGen::start()
{
  hardware_interface_->StartDatataking();
}

void CRT::FragGen::stop()
{
  hardware_interface_->StopDatataking();
}

// The following macro is defined in artdaq's GeneratorMacros.hh header
DEFINE_ARTDAQ_COMMANDABLE_GENERATOR(CRT::FragGen)
