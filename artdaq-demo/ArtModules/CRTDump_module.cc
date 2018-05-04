////////////////////////////////////////////////////////////////////////
// Class:       CRTDump
// Module Type: analyzer
// File:        CRTDump_module.cc
// Description: Prints out information about each event.
////////////////////////////////////////////////////////////////////////

#define TRACE_NAME "CRTDump"

#include "art/Framework/Core/EDAnalyzer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Handle.h"
#include "canvas/Utilities/Exception.h"

#include "artdaq-core-demo/Overlays/CRTFragment.hh"
#include "artdaq-core/Data/ContainerFragment.hh"
#include "artdaq-core/Data/Fragment.hh"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <vector>
#include <iostream>

// XXX probably does not make sense to have in the demo namespace
namespace demo
{
  class CRTDump;
}

/**
 * \brief An art::EDAnalyzer module designed to display the data from demo::CRTFragment objects
 */
class demo::CRTDump : public art::EDAnalyzer
{
public:
  /**
   * \brief CRTDump Constructor
   * \param pset ParamterSet used to configure CRTDump
   *
   * \verbatim
   * CRTDump accepts the following Parameters:
   * "raw_data_label" (Default: "daq"): The label used to identify artdaq data
   * "num_adcs_to_show" (Default: 0): How many ADCs to print from each CRTFragment
   * "dump_to_file" (Default: true): Whether to write data to a binary file "out.bin"
   * "dump_to_screen" (Default: false): Whether to write data to stdout
   * "columns_to_display_on_screen" (Default: 10): How many ADC values to print in each row when writing to stdout
   * \endverbatim
   */
  explicit CRTDump(fhicl::ParameterSet const& pset);

  /**
   * \brief CRTDump Destructor
   */
  virtual ~CRTDump();

  /**
  * \brief Analyze an event. Called by art for each event in run (based on command line options)
  * \param evt The art::Event object to dump CRTFragments from
  */
  virtual void analyze(art::Event const& evt);

private:
  std::string raw_data_label_;
  int num_adcs_to_write_;
  int num_adcs_to_print_;
  bool dump_to_file_;
  bool dump_to_screen_;
  uint32_t columns_to_display_on_screen_;
  std::string output_file_name_;
};


demo::CRTDump::CRTDump(fhicl::ParameterSet const& pset)
  : EDAnalyzer(pset)
  , raw_data_label_(pset.get<std::string>("raw_data_label", "daq"))
  , num_adcs_to_write_(pset.get<int>("num_adcs_to_write", 0))
  , num_adcs_to_print_(pset.get<int>("num_adcs_to_print", 10))
  , columns_to_display_on_screen_(pset.get<uint32_t>("columns_to_display_on_screen", 10))
  , output_file_name_(pset.get<std::string>("output_file_name", "out.bin"))
{}

demo::CRTDump::~CRTDump() {}

void demo::CRTDump::analyze(art::Event const& evt)
{
  artdaq::Fragments fragments;
  artdaq::FragmentPtrs containerFragments;
  std::vector<std::string> fragment_type_labels{ "CRT", "ContainerCRT" };

  for (auto label : fragment_type_labels)
  {
    art::Handle<artdaq::Fragments> fragments_with_label;

    evt.getByLabel(raw_data_label_, label, fragments_with_label);
    if (!fragments_with_label.isValid()) continue;

    for (auto cont : *fragments_with_label)
    {
      artdaq::ContainerFragment contf(cont);
      for (size_t ii = 0; ii < contf.block_count(); ++ii)
      {
        containerFragments.push_back(contf[ii]);
        fragments.push_back(*containerFragments.back());
      }
    }
  }

  for(const auto& frag : fragments) {
    CRTFragment mod(frag); // module packet

    printf("Number of hits: %lu\n", mod.num_hits());

  }
}

DEFINE_ART_MODULE(demo::CRTDump)
