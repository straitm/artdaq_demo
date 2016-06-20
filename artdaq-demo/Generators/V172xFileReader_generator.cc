

#include "artdaq-demo/Generators/V172xFileReader.hh"

#ifdef CANVAS
#include "canvas/Utilities/Exception.h"
#else
#include "art/Utilities/Exception.h"
#endif

#include "artdaq-core-demo/Overlays/V172xFragment.hh"
#include "artdaq-core-demo/Overlays/V172xFragmentWriter.hh"
#include "artdaq/DAQdata/Debug.hh"
#include "artdaq/Application/GeneratorMacros.hh"
#include "cetlib/exception.h"
#include "fhiclcpp/ParameterSet.h"

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <random>
#include <string>
#include <vector>

using fhicl::ParameterSet;

namespace {
  demo::V172xFragment::adc_type
  init_seed(fhicl::ParameterSet const & ps)
  {
    std::random_device rd;
    return ps.get<demo::V172xFragment::adc_type>
           ("seed",
            std::uniform_int_distribution<demo::V172xFragment::adc_type>()(rd));
  }
}

demo::V172xFileReader::V172xFileReader(ParameterSet const & ps)
  :
  CommandableFragmentGenerator(ps),
  fileNames_(ps.get<std::vector<std::string>>("fileNames")),
  max_set_size_bytes_(ps.get<double>("max_set_size_gib",
                                     14.0) * 1024 * 1024 * 1024),
  max_events_(ps.get<int>("max_events", -1)),
  primary_type_(toFragmentType(ps.get<std::string>("primary_fragment_type",
                                                   "V1720"))),
  secondary_types_(),
  size_in_words_(ps.get<bool>("size_in_words", true)),
  seed_(init_seed(ps)),
  events_read_(0),
  next_point_ {fileNames_.begin(), 0},
  should_stop_(0),
  twoBits_(seed_)
{
  auto st_strings =
    ps.get<std::vector<std::string>>("secondary_fragment_type", { });
  std::transform(st_strings.begin(),
                 st_strings.end(),
                 std::back_inserter(secondary_types_),
                 &toFragmentType);
  size_t expected_ids = 1 + secondary_types_.size();
  if (fragmentIDs().size() != expected_ids) {
    throw art::Exception(art::errors::Configuration)
        << "Incorrect number of fragment IDs ("
        << fragmentIDs().size()
        << ", expected "
        << expected_ids
        << "), first file "
        << *fileNames_.begin()
        << ".\n";
  }
}

bool demo::V172xFileReader::getNext_(artdaq::FragmentPtrs & frags)
{
  if (should_stop()) { return false; }
  artdaq::FragmentPtrs::size_type incoming_size = frags.size();
  if (next_point_.first == fileNames_.end() ||
      !(max_events_ == -1 || static_cast<size_t>(max_events_) > events_read_)) {
    return false; // Nothing to do.
  }
  // Useful constants for byte arithmetic.
  static size_t const words_per_frag_word =
    sizeof(artdaq::Fragment::value_type) /
    sizeof(V172xFragment::Header::data_t);
  V172xFragment::Header vHead; // To receive header.
  static size_t const header_size_bytes = sizeof(vHead);
  static size_t const header_size_frag_words =
    ceil(header_size_bytes /
         static_cast<double>(sizeof(artdaq::Fragment::value_type)));
  // Open file.
  std::ifstream in_data;
  uint64_t read_bytes = 0;
  // Container into which to retrieve the header and interrogate with a
  // V172xFragment overlay.
  artdaq::Fragment header_frag(header_size_frag_words);
  while (!((max_set_size_bytes_ < read_bytes) ||
           next_point_.first == fileNames_.end()) &&
         (max_events_ == -1 || static_cast<size_t>(max_events_) > events_read_)) {
    if (!in_data.is_open()) {
      in_data.open((*next_point_.first).c_str(),
                   std::ios::in | std::ios::binary);
      if (!in_data) {
        throw cet::exception("FileOpenFailure")
            << "Unable to open file "
            << *next_point_.first
            << ".";
      }
      // Find where we left off.
      in_data.seekg(next_point_.second);
      if (!in_data) {
        throw cet::exception("FileSeekFailure")
            << "Unable to seek to last known point "
            << next_point_.second
            << " in file "
            << *next_point_.first
            << ".";
      }
    }
    // Read DS50 header.
    in_data.read(reinterpret_cast<char *>(&vHead), header_size_bytes);
    if (!in_data) {
      if (in_data.gcount() == 0 && in_data.eof()) {
        // eof() at fragment boundary.
        in_data.close();
        // Move to next file and reset.
        ++next_point_.first;
        next_point_.second = 0;
        continue;
      }
      else {
        // Failed stream.
        throw cet::exception("FileReadFailure")
            << "Unable to read header from file "
            << *next_point_.first
            << " after "
            << read_bytes
            << ".\n";
      }
    }
    read_bytes += header_size_bytes;
    if (!size_in_words_) { // File is incorrectly formatted: fix.
      vHead.event_size /= V172xFragment::Header::size_words;
    }
    size_t const final_payload_size =
      ceil(vHead.event_size / // Includes header.
           static_cast<double>(words_per_frag_word));
    frags.emplace_back(new artdaq::Fragment(final_payload_size));
    artdaq::Fragment & frag = *frags.back();
    // Copy the header info in from header_frag.
    memcpy(&*frag.dataBegin(),
           &vHead,
           header_size_bytes);
    char * buf_ptr = reinterpret_cast<char *>(&*frag.dataBegin()) +
                     header_size_bytes;
    // Read rest of board data.
    uint64_t const bytes_left_to_read =
      (vHead.event_size * sizeof(V172xFragment::Header::data_t)) - header_size_bytes;
    in_data.read(buf_ptr, bytes_left_to_read);
    if (!in_data) {
      throw cet::exception("FileReadFailure")
          << "Unable to read data from file "
          << *next_point_.first
          << " after "
          << read_bytes
          << " bytes.";
    }
    assert((frag.dataEnd() - frag.dataBegin()) * sizeof(artdaq::RawDataType) ==
           bytes_left_to_read + header_size_bytes);
    read_bytes += bytes_left_to_read;
    // Update fragment header.
    frag.setFragmentID(fragmentIDs()[0]);
    frag.setSequenceID(vHead.event_counter);
    frag.setUserType(primary_type_);
    ++events_read_;
    produceSecondaries_(frags);
  }
  // Update counter for next time.
  if (in_data.is_open()) {
    next_point_.second = in_data.tellg();
  }
  Debug << "Read successfully from file "
        << frags.size() - incoming_size
        << " fragments in "
        << std::fixed
        << std::setprecision(1)
        << read_bytes / 1024.0 / 1024.0 << " MiB ("
        << read_bytes
        << " b)."
        << flusher;
  return true;
}

void
demo::V172xFileReader::
produceSecondaries_(artdaq::FragmentPtrs & frags)
{
  auto const & pFrag = *frags.back();
  size_t id_index { 1 }; // First secondary fragmentID.
  for (auto i = secondary_types_.cbegin(),
       e = secondary_types_.cend();
       i != e;
       ++i, ++id_index) {
    frags.emplace_back(convertFragment_(pFrag, *i, fragmentIDs()[id_index]));
  }
}

artdaq::FragmentPtr
demo::V172xFileReader::
convertFragment_(artdaq::Fragment const & source,
                 demo::FragmentType dType,
                 artdaq::Fragment::fragment_id_t id)
{
  artdaq::FragmentPtr result(new artdaq::Fragment(source));
  result->setUserType(dType);
  result->setFragmentID(id);
  V172xFragmentWriter overlay(*result);
  // Only know how to convert V1720 <-> V1724.
  if (source.type() == FragmentType::V1720 &&
      dType == FragmentType::V1724) {
    std::transform(overlay.dataBegin(),
                   overlay.dataEnd(),
                   overlay.dataBegin(),
                   [this](V172xFragment::adc_type x) ->
    V172xFragment::adc_type {
      auto tmp = x << 2;
      tmp |= twoBits_();
      return tmp;
    });
  }
  else if (source.type() == FragmentType::V1724 &&
           dType == FragmentType::V1720) {
    std::transform(overlay.dataBegin(),
                   overlay.dataEnd(),
                   overlay.dataBegin(),
    [](V172xFragment::adc_type x) {
      return x >> 2;
    });
  }
  else {
    throw art::Exception(art::errors::Configuration)
        << "convertFragment: cannot convert from "
        << fragmentTypeToString(FragmentType(source.type()))
        << " to "
        << fragmentTypeToString(dType)
        << ".\n";
  }
  return std::move(result);
}
DEFINE_ARTDAQ_COMMANDABLE_GENERATOR(demo::V172xFileReader)
