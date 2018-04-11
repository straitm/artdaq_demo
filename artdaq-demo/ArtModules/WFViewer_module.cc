#include "art/Framework/Core/EDAnalyzer.h"
#include "art/Framework/Principal/Handle.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Run.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "canvas/Utilities/InputTag.h"

#include "artdaq-core/Data/Fragment.hh"
#include "artdaq-core/Data/ContainerFragment.hh"

#include "cetlib/exception.h"

#include <TFile.h>
#include <TRootCanvas.h>
#include <TCanvas.h>
#include <TGraph.h>
#include <TAxis.h>
#include <TH1D.h>
#include <TStyle.h>

#include <numeric>
#include <vector>
#include <functional>
#include <algorithm>
#include <iostream>
#include <sstream>
#include <initializer_list>
#include <limits>

using std::cout;
using std::cerr;
using std::endl;

namespace demo
{
	/**
	 * \brief An example art analysis module which plots events both as histograms and event snapshots (plot of ADC value vs ADC number)
	 */
	class WFViewer : public art::EDAnalyzer
	{
	public:
		/**
		 * \brief WFViewer Constructor
		 * \param p ParameterSet used to configure WFViewer
		 * 
		 * \verbatim
		 * WFViewer accepts the following Parameters:
		 * "prescale" (REQUIRED): WFViewer will only redraw historgrams once per this many events
		 * "digital_sum_only" (Default: false): Only create the histogram, not the event snapshot
		 * "num_x_plots": (Default: size_t::MAX_VALUE): Maximum number of columns of plots
		 * "num_y_plots": (Default: size_t::MAX_VALUE): Maximum number of rows of plots
		 * "raw_data_label": (Default: "daq"): Label under which artdaq data is stored
		 * "fragment_ids": (REQUIRED): List of ids to process. Fragment IDs are assigned by BoardReaders.
		 * "fileName": (Default: artdaqdemo_onmon.root): File name for output, if
		 * "write_to_file": (Default: false): Whether to write output histograms to "fileName"
		 * \endverbatim
		 */
		explicit WFViewer(fhicl::ParameterSet const& p);

		/**
		 * \brief WFViewer default Destructor
		 */
		virtual ~WFViewer() = default;

		/**
		* \brief Analyze an event. Called by art for each event in run (based on command line options)
		* \param e The art::Event object to process, and display if it passes the prescale
		*/
		void analyze(art::Event const& e) override;

		/**
		 * \brief Art calls this function at the beginning of the run. Used for set-up of ROOT histogram objects
		 * and to open the output file if one is specified.
		 */
		void beginRun(art::Run const&) override;

	private:

		std::unique_ptr<TCanvas> canvas_[2];
		std::vector<Double_t> x_;
		int prescale_;
		bool digital_sum_only_;
		art::RunNumber_t current_run_;

		std::size_t num_x_plots_;
		std::size_t num_y_plots_;

		std::string raw_data_label_;
		std::vector<artdaq::Fragment::fragment_id_t> fragment_ids_;

		std::vector<std::unique_ptr<TGraph>> graphs_;
		std::vector<std::unique_ptr<TH1D>> histograms_;

		std::map<artdaq::Fragment::fragment_id_t, std::size_t> id_to_index_;
		std::string outputFileName_;
		TFile* fFile_;
		bool writeOutput_;
	};
}

demo::WFViewer::WFViewer(fhicl::ParameterSet const& ps):
													   art::EDAnalyzer(ps)
													   , prescale_(ps.get<int>("prescale"))
													   , digital_sum_only_(ps.get<bool>("digital_sum_only", false))
													   , current_run_(0)
													   , num_x_plots_(ps.get<std::size_t>("num_x_plots", std::numeric_limits<std::size_t>::max()))
													   , num_y_plots_(ps.get<std::size_t>("num_y_plots", std::numeric_limits<std::size_t>::max()))
													   , raw_data_label_(ps.get<std::string>("raw_data_label", "daq"))
													   , fragment_ids_(ps.get<std::vector<artdaq::Fragment::fragment_id_t>>("fragment_ids"))
													   , graphs_(fragment_ids_.size())
													   , histograms_(fragment_ids_.size())
													   , outputFileName_(ps.get<std::string>("fileName", "artdaqdemo_onmon.root"))
													   , writeOutput_(ps.get<bool>("write_to_file", false))
{
	if (num_x_plots_ == std::numeric_limits<std::size_t>::max() ||
		num_y_plots_ == std::numeric_limits<std::size_t>::max())
	{
		switch (fragment_ids_.size())
		{
		case 1: num_x_plots_ = num_y_plots_ = 1;
			break;
		case 2: num_x_plots_ = 2;
			num_y_plots_ = 1;
			break;
		case 3:
		case 4: num_x_plots_ = 2;
			num_y_plots_ = 2;
			break;
		case 5:
		case 6: num_x_plots_ = 3;
			num_y_plots_ = 2;
			break;
		case 7:
		case 8: num_x_plots_ = 4;
			num_y_plots_ = 2;
			break;
		default:
			num_x_plots_ = num_y_plots_ = static_cast<std::size_t>(ceil(sqrt(fragment_ids_.size())));
		}
	}

	// id_to_index_ will translate between a fragment's ID and where in
	// the vector of graphs and histograms it's located

	for (std::size_t i_f = 0; i_f < fragment_ids_.size(); ++i_f)
	{
		id_to_index_[fragment_ids_[i_f]] = i_f;
	}

	gStyle->SetOptStat("irm");
	gStyle->SetMarkerStyle(22);
	gStyle->SetMarkerColor(4);
}

void demo::WFViewer::analyze(__attribute__((unused)) art::Event const& e)
{
}

void demo::WFViewer::beginRun(art::Run const& e)
{
	if (e.run() == current_run_) return;
	current_run_ = e.run();

	if (writeOutput_)
	{
		fFile_ = new TFile(outputFileName_.c_str(), "RECREATE");
		fFile_->cd();
	}

	for (int i = 0; i < 2; i++) canvas_[i] = 0;
	for (auto& x: graphs_) x = 0;
	for (auto& x: histograms_) x = 0;

	for (int i = 0; (i < 2 && !digital_sum_only_) || i < 1; i++)
	{
		canvas_[i] = std::unique_ptr<TCanvas>(new TCanvas(Form("wf%d", i)));
		canvas_[i]->Divide(num_x_plots_, num_y_plots_);
		canvas_[i]->Update();
		((TRootCanvas*)canvas_[i]->GetCanvasImp())->DontCallClose();
	}

	canvas_[0]->SetTitle("ADC Value Distribution");

	if (! digital_sum_only_)
	{
		canvas_[1]->SetTitle("ADC Values, Event Snapshot");
	}

	if (writeOutput_)
	{
		canvas_[0]->Write();
		canvas_[1]->Write();
	}
}


DEFINE_ART_MODULE(demo::WFViewer)
