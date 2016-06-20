////////////////////////////////////////////////////////////////////////
// Class:       CompressionChecker
// Module Type: analyzer
// File:        CompressionChecker_module.cc
//
// Generated at Mon Apr 16 11:46:47 2012 by Christopher Green using artmod
// from art v0_00_02.
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

#include "artdaq-core/Data/Fragments.hh"

#include <algorithm>

namespace demo {
  class CompressionChecker;
}

class demo::CompressionChecker : public art::EDAnalyzer {
public:
  explicit CompressionChecker(fhicl::ParameterSet const & p);
  virtual ~CompressionChecker();

  virtual void analyze(art::Event const & e);

private:

  std::string raw_label_;
  std::string uncompressed_label_;
  std::string inst_;
};


demo::CompressionChecker::CompressionChecker(fhicl::ParameterSet const & p)
    : EDAnalyzer(p),
  raw_label_(p.get<std::string>("raw_label")),
  uncompressed_label_(p.get<std::string>("uncompressed_label")),
  inst_(p.get<std::string>("instance"))
{
}

demo::CompressionChecker::~CompressionChecker()
{
}

void demo::CompressionChecker::analyze(art::Event const & e)
{
  art::Handle<artdaq::Fragments> raw, uncomp;
  if (!e.getByLabel(raw_label_, inst_, raw)) {
    throw art::Exception(art::errors::Configuration)
      << "Unable to find raw data product "
      << raw_label_
      << "."
      << inst_
      << " as specified.\n";
  }
  if (!e.getByLabel(uncompressed_label_, inst_, uncomp)) {
    throw art::Exception(art::errors::Configuration)
      << "Unable to find decompressed data product "
      << uncompressed_label_
      << "."
      << inst_
      << " as specified.\n";
  }
  if (raw->size() != uncomp->size()) {
    throw art::Exception(art::errors::DataCorruption)
      << "Raw and decompressed data have different numbers of fragments:\n"
      << raw->size()
      << " != "
      << uncomp->size()
      << ".\n";
  }
  size_t len = raw->size();
  for (size_t i = 0; i < len; ++i) {
    artdaq::Fragment const & rf ((*raw)[i]);
    artdaq::Fragment const & uf ((*uncomp)[i]);
    if (rf.size() != uf.size()) {
      throw art::Exception(art::errors::DataCorruption)
        << "Raw and decompressed versions of fragment "
        << i + 1
        << " / "
        << len
        << " are different sizes:\n"
        << rf.size()
        << " != "
        << uf.size()
        << ".\n";
    }
    if (!std::equal(rf.dataBegin(), rf.dataEnd(), uf.dataBegin())) {
      throw art::Exception(art::errors::DataCorruption)
        << "Content of raw and decompressed versions of fragment "
        << i + 1
        << " / "
        << len
        << " does not match.\n";
    }
  }
}

DEFINE_ART_MODULE(demo::CompressionChecker)
