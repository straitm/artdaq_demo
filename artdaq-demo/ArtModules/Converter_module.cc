////////////////////////////////////////////////////////////////////////
// Class:       Converter
// Module Type: producer
// File:        Converter_module.cc
//
// Generated at Thu Mar 21 20:32:50 2013 by James Kowalkowski using artmod
// from art v1_03_03.
////////////////////////////////////////////////////////////////////////

#include "art/Framework/Core/EDProducer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Handle.h"

#ifdef CANVAS
#include "canvas/Utilities/Exception.h"
#include "canvas/Utilities/InputTag.h"
#else
#include "art/Utilities/Exception.h"
#include "art/Utilities/InputTag.h"
#endif

#include "artdaq-demo/Products/Channel.hh"
#include "artdaq-demo/ArtModules/converter_algs.hh"

#include "artdaq-core-demo/Overlays/FragmentType.hh"
#include "artdaq-core-demo/Overlays/V172xFragment.hh"

#include "artdaq-core/Data/Fragment.hh"
#include "artdaq-core/Data/Fragments.hh"

#include <memory>
#include <utility>

//----------------------------------------------------------------------------
// Class Converter is an EDProducer that creates two products, each of
// type darkart::Channels (a vector of channels) from two inputs: V1720
// and V1724 Fragments.
//----------------------------------------------------------------------------

class Converter : public art::EDProducer {
public:
  explicit Converter(fhicl::ParameterSet const & p);
  virtual ~Converter();
  void produce(art::Event & e) override;

private:
  art::InputTag v1720_tag_;
  art::InputTag v1724_tag_;
};


//----------------------------------------------------------------------------
// The implementation of the module member functions begins here.

Converter::Converter(fhicl::ParameterSet const & ps ) :
  v1720_tag_(ps.get<std::string>("v1720_tag")),
  v1724_tag_(ps.get<std::string>("v1724_tag"))
{
  produces<darkart::Channels>("V1720");
  produces<darkart::Channels>("V1724");
}

Converter::~Converter()
{ }

void Converter::produce(art::Event & e)
{
  // need to get module_label="daq" and instance_name="1720" or "1724"
  auto p_v1720 = e.getValidHandle<artdaq::Fragments>(v1720_tag_);
  auto p_v1724 = e.getValidHandle<artdaq::Fragments>(v1724_tag_);

  // Make our products, which begin empty.
  std::unique_ptr<darkart::Channels> v1720(new darkart::Channels);
  std::unique_ptr<darkart::Channels> v1724(new darkart::Channels);

  // Call the algorithm that will fill the products. We have to pass in
  // the enumeration value that tells us the type of board we're dealing
  // with, because the data don't carry that information directly.
  darkart::convert_fragments(*p_v1720, *v1720, demo::FragmentType::V1720);
  darkart::convert_fragments(*p_v1724, *v1724, demo::FragmentType::V1724);

  // Put our complete products into the Event.
  e.put(std::move(v1720), "V1720");
  e.put(std::move(v1724), "V1724");
}


DEFINE_ART_MODULE(Converter)
