////////////////////////////////////////////////////////////////////////
// Class:       CheckIntegrity
// Module Type: analyzer
// File:        CheckIntegrity_module.cc
// Description: Prints out information about each event.
////////////////////////////////////////////////////////////////////////

#include "art/Framework/Core/EDAnalyzer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Handle.h"
#include "canvas/Utilities/Exception.h"

#include "artdaq-core-demo/Overlays/CRTFragment.hh"
#include "artdaq-core/Data/Fragment.hh"

#include "messagefacility/MessageLogger/MessageLogger.h"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <vector>
#include <iostream>

namespace CRT
{
	class CheckIntegrity;
}

class CRT::CheckIntegrity : public art::EDAnalyzer
{
public:
	/**
	 * \brief CheckIntegrity Constructor
	 * \param pset ParameterSet used to configure CheckIntegrity
	 * 
	 * CheckIntegrity has the following paramters:
	 * "raw_data_label": The label applied to data (usually "daq")
	 * "frag_type": The fragment type to analyze ("TOY1" or "TOY2")
	 */
	explicit CheckIntegrity(fhicl::ParameterSet const& pset);

	/**
	 * \brief Default destructor
	 */
	virtual ~CheckIntegrity() = default;

	/**
	* \brief Analyze an event. Called by art for each event in run (based on command line options)
	* \param evt The art::Event object containing Fragments to check
	*/
	virtual void analyze(art::Event const& evt);

private:
	std::string raw_data_label_;
	std::string frag_type_;
};


CRT::CheckIntegrity::CheckIntegrity(fhicl::ParameterSet const& pset)
	: EDAnalyzer(pset)
	, raw_data_label_(pset.get<std::string>("raw_data_label"))
	, frag_type_(pset.get<std::string>("frag_type")) {}

void CRT::CheckIntegrity::analyze(art::Event const& evt)
{
	art::Handle<artdaq::Fragments> raw;
	evt.getByLabel(raw_data_label_, frag_type_, raw);

	if (raw.isValid())
	{
		for (size_t idx = 0; idx < raw->size(); ++idx){
			const auto& frag((*raw)[idx]);

      CRT::Fragment mod(frag);

      printf("First byte of the fragment is %c\n", ((const char *)&frag)[0]);

      printf("Number of hits: %lu\n", mod.num_hits());
		}
	}
	else
	{
		mf::LogError("CheckIntegrity") << "In run " << evt.run() << ", subrun " << evt.subRun() <<
			", event " << evt.event() << ", raw.isValid() returned false";
	}
}

DEFINE_ART_MODULE(CRT::CheckIntegrity)
