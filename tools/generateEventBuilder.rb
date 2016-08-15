
def generateEventBuilder( fragSizeWords, totalFRs, totalAGs, totalFragments, verbose, ebHost, 
                          ebPort, withGanglia = 0, withMsgFacility = 0, withGraphite = 0)

ebConfig = String.new( "\
daq: {
  max_fragment_size_words: %{size_words}
  event_builder: {
    mpi_buffer_count: %{buffer_count}
    first_fragment_receiver_rank: 0
    fragment_receiver_count: %{total_frs}
    expected_fragments_per_event: %{total_fragments}
    use_art: true
    print_event_store_stats: true
    verbose: %{verbose}
  }
  metrics: {
    evbFile: {
      metricPluginType: \"file\"
      level: 3
      fileName: \"/tmp/eventbuilder/evb_%UID%_metrics.log\"
      uniquify: true
    }
    %{ganglia_metric} ganglia: {
    %{ganglia_metric}   metricPluginType: \"ganglia\"
    %{ganglia_metric}   level: %{ganglia_level}
    %{ganglia_metric}   reporting_interval: 15.0
    %{ganglia_metric} 
    %{ganglia_metric}   configFile: \"/etc/ganglia/gmond.conf\"
    %{ganglia_metric}   group: \"ARTDAQ\"
    %{ganglia_metric} }
    %{mf_metric} msgfac: {
    %{mf_metric}    level: %{mf_level}
    %{mf_metric}    metricPluginType: \"msgFacility\"
    %{mf_metric}    output_message_application_name: \"ARTDAQ Metric\"
    %{mf_metric}    output_message_severity: 0 
    %{mf_metric} }
    %{graphite_metric} graphite: {
    %{graphite_metric}   level: %{graphite_level}
    %{graphite_metric}   metricPluginType: \"graphite\"
    %{graphite_metric}   host: \"localhost\"
    %{graphite_metric}   port: 20030
    %{graphite_metric}   namespace: \"artdaq.\"
    %{graphite_metric} }
  }
} "
)

  ebConfig.gsub!(/\%\{size_words\}/, String(fragSizeWords))
  ebConfig.gsub!(/\%\{buffer_count\}/, String(totalFRs*8))
  ebConfig.gsub!(/\%\{total_frs\}/, String(totalFRs))
  ebConfig.gsub!(/\%\{total_fragments\}/, String(totalFragments))
  ebConfig.gsub!(/\%\{verbose\}/, String(verbose))
  test = ebPort * 2

  if Integer(withGanglia) > 0
    brConfig.gsub!(/\%\{ganglia_metric\}/, "")
    brConfig.gsub!(/\%\{ganglia_level\}/, Integer(withGanglia))
  else
    brConfig.gsub!(/\%\{ganglia_metric\}/, "#")
  end
  if Integer(withMsgFacility) > 0
    brConfig.gsub!(/\%\{mf_metric\}/, "")
    brConfig.gsub!(/\%\{mf_level\}/, Integer(withMsgFacility))
  else
    brConfig.gsub!(/\%\{mf_metric\}/, "#")
  end
  if Integer(withGraphite) > 0
    brConfig.gsub!(/\%\{graphite_metric\}/, "")
    brConfig.gsub!(/\%\{graphite_level\}/, Integer(withGraphite))
  else
    brConfig.gsub!(/\%\{graphite_metric\}/, "#")
  end

  return ebConfig

end

