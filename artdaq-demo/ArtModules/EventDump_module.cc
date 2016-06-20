////////////////////////////////////////////////////////////////////////
// Class:       EventDump
// Module Type: analyzer
// File:        EventDump_module.cc
// Description: Prints out information about each event.
////////////////////////////////////////////////////////////////////////

#include "art/Framework/Core/EDAnalyzer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Handle.h"

#ifdef CANVAS
#include "canvas/Utilities/Exception.h"
#else
#include "art/Utilities/Exception.h"
#endif

#include "artdaq-core-demo/Overlays/V172xFragment.hh"
#include "artdaq-core/Data/Fragments.hh"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <vector>
#include <iostream>

namespace demo {
  class EventDump;
}

class demo::EventDump : public art::EDAnalyzer {
public:
  explicit EventDump(fhicl::ParameterSet const & pset);
  virtual ~EventDump();

  virtual void analyze(art::Event const & evt);

private:
  std::string raw_data_label_;
  std::string uncompressed_V1720_label_;
  std::string uncompressed_V1724_label_;
};


demo::EventDump::EventDump(fhicl::ParameterSet const & pset)
    : EDAnalyzer(pset),
  raw_data_label_(pset.get<std::string>("raw_data_label")),
  uncompressed_V1720_label_(pset.get<std::string>("uncompressed_V1720_label")),
  uncompressed_V1724_label_(pset.get<std::string>("uncompressed_V1724_label"))
{
}

demo::EventDump::~EventDump()
{
}

void demo::EventDump::analyze(art::Event const & evt)
{
  art::EventNumber_t eventNumber = evt.event();

  // ***********************
  // *** V1720 Fragments ***
  // ***********************

  // look for raw V1720 data first, but also check for uncompressed
  // V1720 data
  art::Handle<artdaq::Fragments> raw;
  evt.getByLabel(raw_data_label_, "V1720", raw);
  if (! raw.isValid()) {
    evt.getByLabel(uncompressed_V1720_label_, "V1720", raw);
  }

  if (raw.isValid()) {
    std::cout << "Run " << evt.run() << ", subrun " << evt.subRun()
              << ", event " << eventNumber << " has " << raw->size()
              << " V1720 fragment(s)." << std::endl;

    for (size_t idx = 0; idx < raw->size(); ++idx) {
      const auto& frag((*raw)[idx]);

      std::cout << "  Fragment " << frag.fragmentID() << " has size "
                << frag.size() << " words." << std::endl;

      V172xFragment bb(frag);
      size_t board_id = bb.board_id ();
      std::cout << "    board ID " << board_id << ", data size = "
                << bb.event_size() << ", total ADC values = "
                << bb.total_adc_values() << ", event counter = "
                << bb.event_counter() << "." << std::endl;

      if (frag.hasMetadata()) {
        V172xFragment::metadata const* md =
          frag.metadata<V172xFragment::metadata>();
        std::cout << std::showbase << "    model number = "
                  << ((int)md->v17xx_model) << ", sample bits = "
                  << ((int)md->sample_bits) << ", sample rate = "
                  << ((int)md->sample_rate_MHz) << " MHz"
                  << std::endl;
      }
    }
  }
  else {
    std::cout << "Run " << evt.run() << ", subrun " << evt.subRun()
              << ", event " << eventNumber << " has zero"
              << " V1720 fragments." << std::endl;
  }
  std::cout << std::endl;

  // ***********************
  // *** V1724 Fragments ***
  // ***********************

  // look for raw V1724 data first, but also check for uncompressed
  // V1724 data
  evt.getByLabel(raw_data_label_, "V1724", raw);
  if (! raw.isValid()) {
    evt.getByLabel(uncompressed_V1724_label_, "V1724", raw);
  }

  if (raw.isValid()) {
    std::cout << "Run " << evt.run() << ", subrun " << evt.subRun()
              << ", event " << eventNumber << " has " << raw->size()
              << " V1724 fragment(s)." << std::endl;

    for (size_t idx = 0; idx < raw->size(); ++idx) {
      const auto& frag((*raw)[idx]);

      std::cout << "  Fragment " << frag.fragmentID() << " has size "
                << frag.size() << " words." << std::endl;

      V172xFragment bb(frag);
      size_t board_id = bb.board_id ();
      std::cout << "    board ID " << board_id << ", data size = "
                << bb.event_size() << ", total ADC values = "
                << bb.total_adc_values() << ", event counter = "
                << bb.event_counter() << "." << std::endl;

      if (frag.hasMetadata()) {
        V172xFragment::metadata const* md =
          frag.metadata<V172xFragment::metadata>();
        std::cout << std::showbase << "    model number = "
                  << ((int)md->v17xx_model) << ", sample bits = "
                  << ((int)md->sample_bits) << ", sample rate = "
                  << ((int)md->sample_rate_MHz) << " MHz"
                  << std::endl;
      }
    }
  }
  else {
    std::cout << "Run " << evt.run() << ", subrun " << evt.subRun()
              << ", event " << eventNumber << " has zero"
              << " V1724 fragments." << std::endl;
  }
  std::cout << std::endl;
}

DEFINE_ART_MODULE(demo::EventDump)
