#include "art/Framework/Core/EDProducer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Selector.h"
#include "art/Framework/Principal/Handle.h"
#include "art/Framework/Principal/Run.h"
#include "art/Framework/Principal/SubRun.h"

#ifdef CANVAS
#include "canvas/Persistency/Provenance/BranchType.h"
#include "canvas/Persistency/Provenance/EventID.h"
#else
#include "art/Persistency/Provenance/BranchType.h"
#include "art/Persistency/Provenance/EventID.h"
#endif

#include "artdaq-demo/Compression/Encoder.hh"
#include "artdaq-demo/Compression/Properties.hh"
#include "artdaq-demo/Compression/StatsKeeper.hh"
#include "artdaq-demo/Compression/SymTable.hh"
#include "artdaq-core-demo/Overlays/V172xFragment.hh"
#include "artdaq-demo/Products/CompressedV172x.hh"
#include "artdaq-core/Data/Fragment.hh"
#include "artdaq-core/Data/Fragments.hh"
#include "artdaq-core/Utilities/SimpleLookupPolicy.h"

#include "fhiclcpp/ParameterSet.h"
#include "messagefacility/MessageLogger/MessageLogger.h"

#include <iostream>
#include <list>
#include <ostream>
#include <cstdlib>
#include <fstream>
#include <string>
#include <utility>
#include <memory>

#include <sys/types.h>
#include <unistd.h>


/*
  There are several important things that need to be resolved
  regarding the generation and regeneration of the active Huffman
  table.  

  There are two metrics that can be used to tell how well the
  current table is doing: 
  1) how is the compression ratio?  
  2) how different is the most popular value in the event data from the
  Huffman's smallest encoded symbol?

  We do not know the effects of individual channels having different
  most common values and how far off they are from one another.
  If the most common value occurs many more times than the second and third
  most popular values, then it may be worth keeping per-channel tables. There
  will be a one bit difference in the encoding of the most popular and the 
  second, third, and probably fourth most popular values.

  In order to recalcabrate on the fly, the encoder will need to keep
  track of values (1) and (2) above.  As a result of seeing a constant
  drift in performance, the encoder will need to notify the module to 
  begin a Huffman table generation sequence using a configurable number
  of events.  After completion of this, the module will give the table
  to a service, which will alert the system controller that a new subrun
  is necessary for the purpose of starting a new Huffman table.  The
  System Controller will then need to ask the builders for their tables
  and decide what to do.

  We may be able to do the "calculateProbs" function on-the-fly in the 
  encoder, and only after performance degradation do the remaining 
  costly steps of constructing the Huffman tree.

  The compressed products will be labeled like this:
  module_label="huffman", 
  product_type="CompressedV172x", 
  instance_name="V1720"
 */

namespace {

  struct compression_record {
    art::EventID                    eid;
    artdaq::Fragment::fragment_id_t fid;
    std::size_t                     uncompressed_size;
    std::size_t                     compressed_size;

    compression_record(art::EventID e,
                       artdaq::Fragment const& uncomp,
                       demo::DataVec const& comp) :
      eid(e), fid(uncomp.fragmentID()),
      uncompressed_size(uncomp.dataSize()),
      compressed_size(comp.size())
    { }
  };

  std::ostream& operator<< (std::ostream& os,
                            compression_record const& r) {
    os << r.eid.run() << ' ' << r.eid.subRun() << ' ' << r.eid.event() << ' '
       << r.fid << ' '
       << r.uncompressed_size << ' '
       << r.compressed_size;
    return os;
  }
}

namespace demo {

  class Compression : public art::EDProducer {
  public:
    explicit Compression(fhicl::ParameterSet const & p);
    virtual ~Compression() { }

    virtual void produce(art::Event & e);
    virtual void endSubRun(art::SubRun & sr);
    virtual void endRun(art::Run & r);

  private:
    std::string const mod_label_;
    std::string const raw_label_;
    std::string const inst_name_;
    std::string const table_file_;
    std::string const table_file_path_;
    bool use_diffs_;
    bool perf_print_;
    int bits_;
    SymTable table_;
    Encoder encode_;

    bool record_compression_;
    // we use a list to avoid timing glitches from a vector resizing.
    std::list<compression_record> records_;
    StatsKeeper stats_;
  };

  static SymTable callReadTable(std::string const & fname,
                                std::string const & fpath,
                                size_t bits)
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
    readTable(fullPath.c_str(), t, 1UL<<bits);
    return t;
  }

  Compression::Compression(fhicl::ParameterSet const & p):
    mod_label_(p.get<std::string>("module_label")),
    raw_label_(p.get<std::string>("raw_label","daq")),
    inst_name_(p.get<std::string>("instance_name")),
    table_file_(p.get<std::string>("table_file")),
    table_file_path_(p.get<std::string>("table_file_path", "DAQ_INDATA_PATH")),
    use_diffs_(p.get<bool>("use_diffs")),
    perf_print_(p.get<bool>("perf_print",false)),
    bits_(use_diffs_?16:inst_name_=="V1724" ? 14 : inst_name_=="V1720" ? 12 : 0),
    table_(callReadTable(table_file_, table_file_path_, bits_)),
    encode_(table_, use_diffs_),
    record_compression_(p.get<bool>("record_compression", false)),
    records_(),
    stats_()
  {
    produces<CompressedV172x>(inst_name_);

    if(bits_==0)
      throw std::runtime_error("invalid instance name given");
  }

  void Compression::produce(art::Event & e)
  {
    art::Handle<artdaq::Fragments> h_172x;
    e.getByLabel(raw_label_, inst_name_, h_172x);

    // Could do both at once using selectors so we can get zero or one
    // ProductInstanceNameSelector("V172x") sel_172x;
    // ProductInstanceNameSelector("V1724") sel_1724;
    // e.getMany(sel_172x, results_vec_handles);

    std::unique_ptr<CompressedV172x> prod_172x(new CompressedV172x(*h_172x));
    prod_172x->setMetadata(bits_, CompressedV172x::AlgoHuffman,
			   use_diffs_==true?1:0);

    size_t const len = h_172x->size();

    for (size_t i = 0; i < len; ++i) 
      {
	artdaq::Fragment const & frag = (*h_172x)[i];
	V172xFragment b(frag);
	// start of payload is the  header
	// b.checkADCData(12); // Check data integrity.

#if 1
	auto adc_start = b.dataBegin();
	auto adc_end   = b.dataEnd();
#endif

	// I think the original code assumes that the size of 
	// compressed record will always be smaller than the original
	// make sure the destination is big enough

	size_t big_enough = (size_t)(frag.dataSize() * 2.0);

	prod_172x->fragment(i).resize(big_enough);
#if 1
	reg_type bit_count  = encode_(adc_start, adc_end,
				      prod_172x->fragment(i));
#else
	reg_type bit_count=1;
#endif
	// shrink destination to the actual size
	size_t actual_size = std::ceil(bit_count /
				       (8.0*sizeof(DataVec::value_type)));
	prod_172x->fragment(i).resize(actual_size);
	prod_172x->setFragmentBitCount(i, bit_count);

	stats_.put((*h_172x)[i].dataSize(), prod_172x->fragment(i).size());

	if(perf_print_)
          {
	    std::cout << "Compression" << " "
		      << mod_label_ << " "
		      << inst_name_ << " "
		      << raw_label_ << " "
		      << e.event() << " "
		      << i << " "
		      << ((double)(*h_172x)[i].dataSize() / (double)prod_172x->fragment(i).size()) << " "
		      << ((double)prod_172x->fragment(i).size() / (double)(*h_172x)[i].dataSize()) << " "
		      << "\n";
          }


      }
    
    if (record_compression_)
      {
	for (size_t i = 0; i < len; ++i)
	  {
	    records_.emplace_back(e.id(), (*h_172x)[i], prod_172x->fragment(i));
	  }
      }

    e.put(std::move(prod_172x),inst_name_);
  }

  void Compression::endSubRun(art::SubRun &) { }

  void Compression::endRun(art::Run &) 
  {
    if(!record_compression_) return; 

    std::string filename("compression_stats_");
    filename += std::to_string(getpid());
    filename += ".txt";
    std::cerr << "Attempting to open file: " << filename << std::endl;

    std::ofstream ofs(filename);
    if (!ofs) {
      std::cerr << "Failed to open the output file." << std::endl;
    }

    for (auto const& r : records_) {
      ofs << r << "\n";
    }
    ofs.close();
  }

  DEFINE_ART_MODULE(Compression)
}

