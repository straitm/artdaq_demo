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
#include "artdaq-core-demo/Overlays/ToyFragment.hh"
#include "artdaq-core/Data/ContainerFragment.hh"
#include "artdaq-core/Data/Fragment.hh"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <vector>
#include <iostream>

namespace demo
{
	class ToyDump;
}

/**
 * \brief An art::EDAnalyzer module designed to display the data from demo::ToyFragment objects
 */
class demo::ToyDump : public art::EDAnalyzer
{
public:
	/**
	 * \brief ToyDump Constructor
	 * \param pset ParamterSet used to configure ToyDump
	 * 
	 * ToyDump accepts the following Parameters:
	 * "raw_data_label" (Default: "daq"): The label used to identify artdaq data
	 * "num_adcs_to_show" (Default: 0): How many ADCs to print from each ToyFragment
	 * "dump_to_file" (Default: true): Whether to write data to a binary file "out.bin"
	 * "dump_to_screen" (Default: false): Whether to write data to stdout
	 * "columns_to_display_on_screen" (Default: 10): How many ADC values to print in each row when writing to stdout
	 */
	explicit ToyDump(fhicl::ParameterSet const& pset);

	/**
	 * \brief ToyDump Destructor
	 */
	virtual ~ToyDump();

	/**
	* \brief Analyze an event. Called by art for each event in run (based on command line options)
	* \param evt The art::Event object to dump ToyFragments from
	*/
	virtual void analyze(art::Event const& evt);

private:
	std::string raw_data_label_;
	uint32_t num_adcs_to_show_;
	bool dump_to_file_;
	bool dump_to_screen_;
	uint32_t columns_to_display_on_screen_;
};


demo::ToyDump::ToyDump(fhicl::ParameterSet const& pset)
	: EDAnalyzer(pset)
	, raw_data_label_(pset.get<std::string>("raw_data_label", "daq"))
	, num_adcs_to_show_(pset.get<uint32_t>("num_adcs_to_show", 0))
	, dump_to_file_(pset.get<bool>("dump_to_file", true))
	, dump_to_screen_(pset.get<bool>("dump_to_screen", false))
	, columns_to_display_on_screen_(pset.get<uint32_t>("columns_to_display_on_screen", 10)) {}

demo::ToyDump::~ToyDump() {}

void demo::ToyDump::analyze(art::Event const& evt)
{
	art::EventNumber_t eventNumber = evt.event();

	// ***********************
	// *** Toy Fragments ***
	// ***********************

	artdaq::Fragments fragments;
	std::vector<std::string> fragment_type_labels{"TOY1", "TOY2", "Container"};

	for (auto label : fragment_type_labels)
	{
		art::Handle<artdaq::Fragments> fragments_with_label;

		evt.getByLabel(raw_data_label_, label, fragments_with_label);
		if (!fragments_with_label.isValid()) continue;

		//    for (int i_l = 0; i_l < static_cast<int>(fragments_with_label->size()); ++i_l) {
		//      fragments.emplace_back( (*fragments_with_label)[i_l] );
		//    }

		if (label == "Container")
		{
			for (auto cont : *fragments_with_label)
			{
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
		else
		{
			for (auto frag : *fragments_with_label)
			{
				fragments.emplace_back(frag);
			}
		}
	}

	// look for raw Toy data

	std::cout << "######################################################################" << std::endl;
	std::cout << std::endl;
	std::cout << "Run " << evt.run() << ", subrun " << evt.subRun()
		<< ", event " << eventNumber << " has " << fragments.size()
		<< " fragment(s) of type TOY1 or TOY2" << std::endl;

	for (const auto& frag : fragments)
	{
		ToyFragment bb(frag);

		std::cout << std::endl;
		std::cout << fragmentTypeToString(static_cast<demo::detail::FragmentType>(frag.type())) << " fragment " << frag.fragmentID() << " w/ seqID " << frag.sequenceID() << " has total ADC counts = "
			<< bb.total_adc_values() << std::endl;
		//std::cout << std::endl;

		if (frag.hasMetadata())
		{
			std::cout << std::endl;
			std::cout << "Fragment metadata: " << std::endl;
			ToyFragment::Metadata const* md =
				frag.metadata<ToyFragment::Metadata>();
			std::cout << std::showbase << "Board serial number = "
				<< ((int)md->board_serial_number) << ", sample bits = "
				<< ((int)md->num_adc_bits)
				<< " -> max ADC value = "
				<< bb.adc_range((int)md->num_adc_bits)
				<< std::endl;
			//std::cout << std::endl;
		}

		if (num_adcs_to_show_ == 0)
		{
			num_adcs_to_show_ = bb.total_adc_values();
		}

		if (num_adcs_to_show_ > 0)
		{
			if (num_adcs_to_show_ > bb.total_adc_values())
			{
				throw cet::exception("num_adcs_to_show is larger than total number of adcs in fragment");
			}
			else
			{
				std::cout << std::endl;
				std::cout << "First " << num_adcs_to_show_
					<< " ADC values in the fragment: "
					<< std::endl;
			}

			if (dump_to_file_)
			{
				std::ofstream output("out.bin", std::ios::out | std::ios::app | std::ios::binary);
				for (uint32_t i_adc = 0; i_adc < num_adcs_to_show_; ++i_adc)
				{
					output.write((char*)(bb.dataBeginADCs() + i_adc), sizeof(ToyFragment::adc_t));
				}
				output.close();
			}

			if (dump_to_screen_)
			{
				std::cout << std::right;
				int rows = 1 + (int)((num_adcs_to_show_ - 1) / columns_to_display_on_screen_);
				uint32_t adc_counter = 0;
				for (int idx = 0; idx < rows; ++idx)
				{
					std::cout << std::setw(4) << std::setfill('.');
					std::cout << (idx * columns_to_display_on_screen_) << ": ";
					for (uint32_t jdx = 0; jdx < columns_to_display_on_screen_; ++jdx)
					{
						if (adc_counter >= num_adcs_to_show_) { break; }
						std::cout << std::setw(6) << std::setfill(' ');
						std::cout << bb.adc_value(adc_counter);
						++adc_counter;
					}
					std::cout << std::endl;
				}
			}

			std::cout << std::endl;
		}
	}
	std::cout << std::endl;
}

DEFINE_ART_MODULE(demo::ToyDump)
