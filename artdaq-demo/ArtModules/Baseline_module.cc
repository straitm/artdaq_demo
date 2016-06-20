////////////////////////////////////////////////////////////////////////
// Class:       Baseline
// Module Type: producer
// File:        Baseline_module.cc
//
// Generated at Mon Apr  1 16:10:36 2013 by Marc Paterno using artmod
// from art v1_03_08.
////////////////////////////////////////////////////////////////////////

#include <memory>

#include "art/Framework/Core/EDProducer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Handle.h"
#include "art/Framework/Principal/Run.h"
#include "art/Framework/Principal/SubRun.h"

#ifdef CANVAS
#include "canvas/Utilities/InputTag.h"
#else
#include "art/Utilities/InputTag.h"
#endif

#include "fhiclcpp/ParameterSet.h"

#include "artdaq-demo/Products/Channel.hh"

#include "artdaq-demo/ArtModules/channel_algs.hh"

//----------------------------------------------------------------------------
// Class Baseline is an EDProducer that creates two Channel objects,
// each containing the baseline-subtracted waveform data. It reads a
// single darkart::Channel object, which represents the sum of the
// samples from all the channels in an event.
// ----------------------------------------------------------------------------

class Baseline : public art::EDProducer {
public:
  explicit Baseline(fhicl::ParameterSet const & p);
  virtual ~Baseline();

  void produce(art::Event & e) override;

private:
  // We will average the first num_avg_ elements of the input Channel to
  // determine the baseline.
  std::size_t   num_avg_;
  art::InputTag v1720_tag_;
  art::InputTag v1724_tag_;
};

using darkart::Channel;
using darkart::make_baseline_subtracted;

//----------------------------------------------------------------------------
// The implementation of the module member functions begins here.

Baseline::Baseline(fhicl::ParameterSet const & ps) :
  num_avg_(ps.get<std::size_t>("num_avg")),
  v1720_tag_(ps.get<std::string>("v1720_tag")),
  v1724_tag_(ps.get<std::string>("v1724_tag"))
{
  produces<darkart::Channel>("V1720");
  produces<darkart::Channel>("V1724");
}

Baseline::~Baseline()
{ }

void Baseline::produce(art::Event & e)
{
  // Get the inputs
  auto p_v1720 = e.getValidHandle<Channel>(v1720_tag_);
	auto p_v1724 = e.getValidHandle<Channel>(v1724_tag_);

  // Create the baseline subtracted products, and put them into the
  // event.
  e.put(make_baseline_subtracted(*p_v1720, num_avg_), "V1720");
  e.put(make_baseline_subtracted(*p_v1724, num_avg_), "V1724");
}

DEFINE_ART_MODULE(Baseline)
