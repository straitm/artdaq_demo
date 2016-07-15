#include "artdaq-demo/ArtModules/converter_algs.hh"
#ifdef CANVAS
#include "canvas/Utilities/Exception.h"
#else
#include "art/Utilities/Exception.h"
#endif

double darkart::scaling_factor(artdaq::Fragment::type_t board_type __attribute__((unused)))
{
  // FIXME
  return 1.0;
}

void darkart::convert_channel(demo::V172xFragment::adc_type const * begin,
                              demo::V172xFragment::adc_type const * end,
                              darkart::Channel& output,
                              artdaq::Fragment::type_t board_type)
{
  double const scale = scaling_factor(board_type);
  for ( ; begin != end; ++begin) output.waveform.push_back(*begin * scale);
}

void darkart::convert_fragment(artdaq::Fragment const& frag,
                               darkart::Channels& output,
                               artdaq::Fragment::type_t board_type)
{
  // Use the overlay to allow us to read channels.
  demo::V172xFragment overlay(frag);

  // Loop over all channels in the fragment, and make a Channel object
  // for each one.
  for (size_t i = 0, nchan = overlay.enabled_channels(); i != nchan; ++i)
    {
      output.emplace_back(overlay.board_id(), i,
                          overlay.adc_values_for_channel());
      convert_channel(overlay.chDataBegin(i),
                      overlay.chDataEnd(i),
                      output.back(),
                      board_type);
    }
}

void darkart::convert_fragments(artdaq::Fragments const& input,
                                darkart::Channels& output,
                                artdaq::Fragment::type_t board_type)
{
  for (auto const& frag : input)
    convert_fragment(frag, output, board_type);
}
