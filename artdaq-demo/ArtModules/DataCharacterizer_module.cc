////////////////////////////////////////////////////////////////////////
// Class:       DataCharacterizer
// Module Type: analyzer
// File:        DataCharacterizer_module.cc
// Description: Produce a table of frequencies for each adc value.
//
// Generated at Mon Apr 30 14:02:31 2012 by Christopher Green using artmod
// from art_openmp v0_00_04.
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

namespace demo {
  class DataCharacterizer;
}

class demo::DataCharacterizer : public art::EDAnalyzer {
public:
  explicit DataCharacterizer(fhicl::ParameterSet const & p);
  virtual ~DataCharacterizer();

  virtual void analyze(art::Event const & e);

  virtual void endJob();

private:
  typedef std::vector<size_t> FragHist_t;
  typedef std::vector<FragHist_t> Hist_t;

  std::string const data_label_;
  std::string const dist_file_;
  Hist_t data_hist_;
  std::set<size_t> used_board_ids_;
};


demo::DataCharacterizer::DataCharacterizer(fhicl::ParameterSet const & p)
    : EDAnalyzer(p),
  data_label_(p.get<std::string>("data_label")),
  dist_file_(p.get<std::string>("dist_file")),
  data_hist_(),
  used_board_ids_()
{
}

demo::DataCharacterizer::~DataCharacterizer()
{
}

void demo::DataCharacterizer::analyze(art::Event const & e)
{
  art::Handle<artdaq::Fragments> handle;
  e.getByLabel(data_label_, handle);
  size_t len = handle->size();
  data_hist_.resize(std::max(len, data_hist_.size()));
  for (size_t i = 0; i < len; ++i) {
    auto const & frag((*handle)[i]);
    V172xFragment b(frag);
    size_t board_id(b.board_id());
    assert(board_id < data_hist_.size());
    used_board_ids_.insert(board_id);
    auto & hist(data_hist_[board_id]);
    hist.resize(V172xFragment::adc_range(12));
    std::for_each(b.dataBegin(),
                  b.dataEnd(),
                  [&hist](V172xFragment::adc_type val) {
                    assert(val < V172xFragment::adc_range(12));
                    ++hist[val];
                  }
                 );
  }
}

void
demo::DataCharacterizer::endJob()
{
  std::ofstream fs(dist_file_);
  if (!fs) {
    throw art::Exception(art::errors::FileOpenError)
      << "Unable to open "
      << dist_file_
      << " for write.";
  }
  std::vector<bool> seen_nonzero(used_board_ids_.size(), false);
  static size_t const adcVal_max = V172xFragment::adc_range(12) - 1;
  size_t adcVal = adcVal_max;
  do {
    if (adcVal == adcVal_max) {
      fs << std::setw(5) << "adc";
      for (auto board_id : used_board_ids_) {
        std::ostringstream os("b", std::ios::out | std::ios::app);
        os << board_id;
        fs << " "
           << std::setw(10)
           << os.str();
      }
      fs << std::endl;
    }
    fs << std::setw(5) << adcVal;
    for (auto board_id : used_board_ids_) {
      auto hItem(data_hist_[board_id][adcVal]);
      if (hItem > 0) {
        seen_nonzero[board_id] = true;
      }
      if ((hItem == 0) && seen_nonzero[board_id]) {
        // All values are at leat possible once the pedestal has started
        // ramping.
        hItem = 1;
      }
      fs << " "
         << std::setw(10)
         << hItem;
    }
    fs << std::endl;
  } while (adcVal-- > 0); // For loop won't work for unsigned!
  fs.close();
}

DEFINE_ART_MODULE(demo::DataCharacterizer)
