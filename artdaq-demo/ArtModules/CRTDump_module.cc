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
#include <fstream>
#include <vector>
#include <iostream>

// XXX probably does not make sense to have in the demo namespace
namespace demo
{
  class CRTDump;
}

/**
 * \brief art::EDAnalyzer to display the data from CRTFragment objects
 */
class demo::CRTDump : public art::EDAnalyzer
{
public:
  /**
   * \brief CRTDump constructor
   * \param pset ParamterSet used to configure CRTDump
   *
   * CRTDump accepts the following parameters:
   * "raw_data_label" (Default: "daq"): The input module label
   * "raw_data_name"  (Default: "CRT"): The input product instance name
   */
  explicit CRTDump(fhicl::ParameterSet const& pset);

  virtual ~CRTDump();

  virtual void analyze(art::Event const& evt);

private:
  std::string raw_data_label_;
  std::string raw_data_name_;
};

demo::CRTDump::CRTDump(fhicl::ParameterSet const& pset)
  : EDAnalyzer(pset)
  , raw_data_label_(pset.get<std::string>("raw_data_label", "daq"))
  , raw_data_name_ (pset.get<std::string>("raw_data_name",  "CRT"))
{
}

demo::CRTDump::~CRTDump() {}

void demo::CRTDump::analyze(art::Event const& evt)
{
  art::Handle<artdaq::Fragments> fragments;

  evt.getByLabel(raw_data_label_, raw_data_name_, fragments);
  if(!fragments.isValid()){
    fprintf(stderr, "No product with label \"%s\" and name \"%s\".\n",
            raw_data_label_.c_str(), raw_data_name_.c_str());
    return;
  }

  for(unsigned int i = 0; i < fragments->size(); i++){
    CRTFragment mod((*fragments)[i]); // module packet

    if(!mod.good_event()) continue;
    mod.print_header();
    mod.print_hits();
  }
}

DEFINE_ART_MODULE(demo::CRTDump)
