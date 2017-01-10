////////////////////////////////////////////////////////////////////////
// Class:       ToyDump
// Module Type: analyzer
// File:        ToyDump_module.cc
// Description: Prints out information about each event.
////////////////////////////////////////////////////////////////////////

#include "art/Framework/Core/EDAnalyzer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Handle.h"
#include "canvas/Utilities/Exception.h"

#include "artdaq-core-demo/Overlays/AsciiFragment.hh"
#include "artdaq-core/Data/Fragments.hh"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <vector>
#include <iostream>

namespace demo {
  class ASCIIDump;
}

class demo::ASCIIDump : public art::EDAnalyzer {
public:
  explicit ASCIIDump(fhicl::ParameterSet const & pset);
  virtual ~ASCIIDump();

  virtual void analyze(art::Event const & evt);

private:
  std::string raw_data_label_;
  std::string frag_type_;
  uint32_t num_adcs_to_show_;
};


demo::ASCIIDump::ASCIIDump(fhicl::ParameterSet const & pset)
    : EDAnalyzer(pset),
      raw_data_label_(pset.get<std::string>("raw_data_label")),
      frag_type_(pset.get<std::string>("frag_type"))
{
}

demo::ASCIIDump::~ASCIIDump()
{
}

void demo::ASCIIDump::analyze(art::Event const & evt)
{
  art::EventNumber_t eventNumber = evt.event();

  // ***********************
  // *** ASCII Fragments ***
  // ***********************

  // look for raw ASCII data

  art::Handle<artdaq::Fragments> raw;
  evt.getByLabel(raw_data_label_, frag_type_, raw);

  if (raw.isValid()) {
    std::cout << "######################################################################" << std::endl;
    std::cout << std::endl;
    std::cout << "Run " << evt.run() << ", subrun " << evt.subRun()
              << ", event " << eventNumber << " has " << raw->size()
              << " fragment(s) of type " << frag_type_ << std::endl;

    for (size_t idx = 0; idx < raw->size(); ++idx) {
      const auto& frag((*raw)[idx]);

      AsciiFragment bb(frag);

      std::cout << std::endl;
      std::cout << "Ascii fragment " << frag.fragmentID() << " has " 
		<< bb.total_line_characters() << " characters in the line." << std::endl;
      std::cout << std::endl;

      if (frag.hasMetadata()) {
      std::cout << std::endl;
	std::cout << "Fragment metadata: " << std::endl;
        AsciiFragment::Metadata const* md =
          frag.metadata<AsciiFragment::Metadata>();
        std::cout << "Chars in line: " << md->charsInLine << std::endl;
	std::cout << std::endl;
      }

	std::ofstream output ("out.bin", std::ios::out | std::ios::app | std::ios::binary );
	for (uint32_t i_adc = 0; i_adc < bb.total_line_characters(); ++i_adc) {
	  output.write((char*)(bb.dataBegin() + i_adc),sizeof(char));
	}
        output.close();
	std::cout << std::endl;
	std::cout << std::endl;
      }
  }
  else {
    std::cout << "Run " << evt.run() << ", subrun " << evt.subRun()
              << ", event " << eventNumber << " has zero"
              << " Toy fragments." << std::endl;
  }
  std::cout << std::endl;

}

DEFINE_ART_MODULE(demo::ASCIIDump)
