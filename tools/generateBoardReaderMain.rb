# This function will generate the FHiCL code used to control the
# BoardReaderMain application by configuring its
# artdaq::BoardReaderCore object
  
def generateBoardReaderMain(totalEBs, totalFRs, fragSizeWords, generatorCode, brHost, brPort, withGanglia = 0, withMsgFacility = 0, withGraphite = 0)

  brConfig = String.new( "\
  daq: {
  max_fragment_size_words: %{size_words}
  fragment_receiver: {
    mpi_buffer_count: %{buffer_count}
    mpi_sync_interval: 50
    first_event_builder_rank: %{total_frs}
    event_builder_count: %{total_ebs}

    %{generator_code}
    }

  metrics: {
    brFile: {
      metricPluginType: \"file\"
      level: 3
      fileName: \"/tmp/boardreader/br_%UID%_metrics.log\"
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
}"
)
  
  brConfig.gsub!(/\%\{total_ebs\}/, String(totalEBs))
  brConfig.gsub!(/\%\{total_frs\}/, String(totalFRs))
  brConfig.gsub!(/\%\{buffer_count\}/, String(totalEBs*8))
  brConfig.gsub!(/\%\{size_words\}/, String(fragSizeWords))
  brConfig.gsub!(/\%\{generator_code\}/, String(generatorCode))

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
 
  return brConfig
end
