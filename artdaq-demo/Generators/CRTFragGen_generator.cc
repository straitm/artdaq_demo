/* Ok, I am morphing this slowly into the CommandableFragmentGenerator
needed for the CRT.  When it has some semblance of working, I should rename it
(it would be nice if the source file matched the header...) and figure out how
to keep it properly. */

#include "artdaq-demo/Generators/CRTFragGen.hh"

#include "canvas/Utilities/Exception.h"

#include "artdaq/Application/GeneratorMacros.hh"
#include "artdaq-core/Utilities/SimpleLookupPolicy.hh"
#include "artdaq-core-demo/Overlays/ToyFragment.hh"

#include "fhiclcpp/ParameterSet.h"

#include <fstream>
#include <iomanip>
#include <iterator>
#include <iostream>

#include <unistd.h>
#include "cetlib_except/exception.h"

demo::CRTFragGen::CRTFragGen(fhicl::ParameterSet const& ps) :
    CommandableFragmentGenerator(ps)
  , hardware_interface_(new CRTInterface(ps))
  , timestamp_(0)
  , readout_buffer_(nullptr)
{
  hardware_interface_->AllocateReadoutBuffer(&readout_buffer_);
}

demo::CRTFragGen::~CRTFragGen()
{
  hardware_interface_->FreeReadoutBuffer(readout_buffer_);
}

bool demo::CRTFragGen::getNext_(
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

  std::unique_ptr<artdaq::Fragment> fragptr(
    // See $ARTDAQ_DIR/Data/Fragment.hh
    artdaq::Fragment::FragmentBytes(
      bytes_read,
      ev_counter(), // from base CommandableFragmentGenerator
      fragment_id(), // ditto
      artdaq::Fragment::FirstUserFragmentType, // only one
      0, // metadata.  What happens to this?
      timestamp_ // This is a uint64_t.  What timestamp? In what units?  Who reads this?
  ));

  frags.emplace_back(std::move(fragptr));

  memcpy(frags.back()->dataBeginBytes(), readout_buffer_, bytes_read);

  if (metricMan /* What is this? */ != nullptr)
    metricMan->sendMetric("Fragments Sent", ev_counter(), "Events", 3,
        artdaq::MetricMode::LastPoint);

  ev_counter_inc(); // from base CommandableFragmentGenerator

  timestamp_++; // XXX currently nonsense

  return true;
}

void demo::CRTFragGen::start()
{
  hardware_interface_->StartDatataking();
}

void demo::CRTFragGen::stop()
{
  hardware_interface_->StopDatataking();
}

// The following macro is defined in artdaq's GeneratorMacros.hh header
DEFINE_ARTDAQ_COMMANDABLE_GENERATOR(demo::CRTFragGen)
