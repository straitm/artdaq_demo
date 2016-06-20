#include "art/Framework/Core/EDProducer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Handle.h"
#include "art/Framework/Principal/Run.h"
#include "art/Framework/Principal/SubRun.h"

#ifdef CANVAS
#include "canvas/Persistency/Provenance/BranchType.h"
#else
#include "art/Persistency/Provenance/BranchType.h"
#endif

#include "artdaq-demo/Compression/Decoder.hh"
#include "artdaq-demo/Compression/Properties.hh"
#include "artdaq-demo/Compression/SymTable.hh"
#include "artdaq-demo/Products/CompressedV172x.hh"

#include "artdaq-core/Data/Fragment.hh"
#include "artdaq-core/Data/Fragments.hh"
#include "artdaq-core/Utilities/SimpleLookupPolicy.h"

#include "fhiclcpp/ParameterSet.h"
#include "messagefacility/MessageLogger/MessageLogger.h"

#include <cmath>
#include <iostream>
#include <string>
#include <memory>

namespace demo {

  class Decompression : public art::EDProducer
  {
  public:
    explicit Decompression(fhicl::ParameterSet const & p);
    virtual ~Decompression() { }

    virtual void produce(art::Event & e);
    virtual void endSubRun(art::SubRun & sr);
    virtual void endRun(art::Run & r);

  private:
    std::string compressed_label_;
    std::string inst_name_;
    std::string table_file_;
    std::string table_file_path_;
    SymTable table_;
    Decoder decode_;
  };

  static SymTable readAndSortTable(std::string const & fname,
                                   std::string const & fpath)
  {
    // 08-Apr-2013, KAB - added the ability to find the specified
    // table file in a list of paths specified explicitly or in an
    // environmental variable
    artdaq::SimpleLookupPolicy::ArgType argType =
      artdaq::SimpleLookupPolicy::ENV_VAR;
    if (getenv(fpath.c_str()) == nullptr) {
      if (fpath.find("/") == std::string::npos &&
          fpath.find(".") == std::string::npos) {
        setenv(fpath.c_str(), ".", 0);
      }
      else {
        argType = artdaq::SimpleLookupPolicy::PATH_STRING;
      }
    }
    artdaq::SimpleLookupPolicy lookup_policy(fpath, argType);
    std::string fullPath = fname;
    try {fullPath = lookup_policy(fname);}
    catch (...) {}

    SymTable t;
    readTable(fullPath.c_str(), t, Properties::count_max());
    return t;
  }

#define ORIG_WAY 0

  Decompression::Decompression(fhicl::ParameterSet const & p)
    : compressed_label_(p.get<std::string>("compressed_label")),
      inst_name_(p.get<std::string>("instance_name")),
      table_file_(p.get<std::string>("table_file")),
      table_file_path_(p.get<std::string>("table_file_path", "DAQ_INDATA_PATH")),
      table_(readAndSortTable(table_file_, table_file_path_)),
      decode_(table_)
  {
    produces<artdaq::Fragments>(inst_name_);
  }

  void Decompression::produce(art::Event & e)
  {
    art::Handle<CompressedV172x> handle;
    e.getByLabel(compressed_label_, inst_name_, handle);
    size_t len = handle->size();
    bool use_diffs = handle->getBias() > 0;

#if ORIG_WAY
    std::unique_ptr<artdaq::Fragments> prod(new artdaq::Fragments(len));
#else
    std::unique_ptr<artdaq::Fragments> prod(new artdaq::Fragments);
#endif

    for (size_t i = 0; i < len; ++i)
      {
#if ORIG_WAY
	artdaq::Fragment & newfrag = (*prod)[i];
	newfrag = (handle->headerOnlyFrag(i)); // Load in the header
	V172xFragment b_orig(newfrag);
	size_t cnt = ceil(b_orig.total_adc_values() * 
			  sizeof(adc_type))/sizeof(reg_type);
	newfrag.resize(cnt + V172xFragment::header_size_words()/2);
	V172xFragment b(newfrag);
	adc_type* new_begin = (adc_type*)(&*newfrag.dataBegin())+
			       (V172xFragment::header_size_words()*2);
	  

	std::cerr << "event=" << e.event()
		  << "dist=" 
		  << std::distance((adc_type const*)(new_begin),b.dataEnd())
		  << " bits=" << handle->fragmentBitCount(i)
		  << "\n";

	auto size_check __attribute__((unused))
	  (decode_(handle->fragmentBitCount(i),
		   handle->fragment(i).begin(),
		   new_begin,
		   b.dataEnd()));
	assert(size_check == b.total_adc_values());

#else
        // Load in the old header                                               
	artdaq::Fragment orig_frag = (handle->headerOnlyFrag(i));
        // size_t curr_payload_sz = orig_frag.dataSize();                       
        V172xFragment b_orig(orig_frag);
        size_t cnt = ceil(b_orig.total_adc_values() *
			  sizeof(adc_type))/sizeof(reg_type);
        orig_frag.resize(cnt + V172xFragment::header_size_words()/2);
        V172xFragment b(orig_frag);

	decode_(handle->fragmentBitCount(i),
		handle->fragment(i).begin(),
		(adc_type*)(&*orig_frag.dataBegin())+
		(V172xFragment::header_size_words()*2),
		b.dataEnd(),
		use_diffs);
	
	prod->push_back(orig_frag);
#endif
      }
    e.put(std::move(prod), inst_name_);
  }

  void Decompression::endSubRun(art::SubRun &) { }
  void Decompression::endRun(art::Run &) { }

  DEFINE_ART_MODULE(Decompression)
}

