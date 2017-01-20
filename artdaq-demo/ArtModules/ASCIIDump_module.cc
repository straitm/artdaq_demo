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

#include "artdaq-core-demo/Overlays/FragmentType.hh"
#include "artdaq-core-demo/Overlays/AsciiFragment.hh"
#include "artdaq-core/Data/ContainerFragment.hh"
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
};


demo::ASCIIDump::ASCIIDump(fhicl::ParameterSet const & pset)
	: EDAnalyzer(pset),
	raw_data_label_(pset.get<std::string>("raw_data_label", "daq"))
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

	artdaq::Fragments fragments;
	std::vector<std::string> fragment_type_labels{ "ASCII", "Container" };

	for (auto label : fragment_type_labels) {

		art::Handle<artdaq::Fragments> fragments_with_label;

		evt.getByLabel(raw_data_label_, label, fragments_with_label);
		if (!fragments_with_label.isValid()) continue;

		//    for (int i_l = 0; i_l < static_cast<int>(fragments_with_label->size()); ++i_l) {
		//      fragments.emplace_back( (*fragments_with_label)[i_l] );
		//    }

		if (label == "Container") {
			for (auto cont : *fragments_with_label) {
				artdaq::ContainerFragment contf(cont);
				for (size_t ii = 0; ii < contf.block_count(); ++ii)
				{
					size_t fragSize = contf.fragSize(ii);
					artdaq::Fragment thisfrag;
					thisfrag.resizeBytes(fragSize);

					//mf::LogDebug("WFViewer") << "Copying " << fragSize << " bytes from " << contf.at(ii) << " to " << thisfrag.headerAddress();
					memcpy(thisfrag.headerAddress(), contf.at(ii), fragSize);

					//mf::LogDebug("WFViewer") << "Putting new fragment into output vector";
					fragments.push_back(thisfrag);
				}
			}
		}
		else {
			for (auto frag : *fragments_with_label) {
				fragments.emplace_back(frag);
			}
		}
	}

	std::cout << "######################################################################" << std::endl;
	std::cout << std::endl;
	std::cout << "Run " << evt.run() << ", subrun " << evt.subRun()
		<< ", event " << eventNumber << " has " << fragments.size()
		<< " ASCII fragment(s)" << std::endl;

	for (const auto& frag : fragments) {

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

		std::ofstream output("out.bin", std::ios::out | std::ios::app | std::ios::binary);
		for (uint32_t i_adc = 0; i_adc < bb.total_line_characters(); ++i_adc) {
			output.write((char*)(bb.dataBegin() + i_adc), sizeof(char));
		}
		output.close();
		std::cout << std::endl;
		std::cout << std::endl;
	}
	std::cout << std::endl;

}

DEFINE_ART_MODULE(demo::ASCIIDump)
