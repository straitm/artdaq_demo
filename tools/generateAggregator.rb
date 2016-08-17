
def generateAggregator(totalFRs, totalEBs, bunchSize, fragSizeWords,
                       xmlrpcClientList, fileSizeThreshold, fileDuration,
                       fileEventCount, queueDepth, queueTimeout, onmonEventPrescale,
                       aggHost, aggPort, agType, withGanglia = 0, withMsgFacility = 0,
                       withGraphite = 0)

agConfig = String.new( "\
daq: {
  max_fragment_size_words: %{size_words}
  aggregator: {
    mpi_buffer_count: %{buffer_count}
    first_event_builder_rank: %{total_frs}
    event_builder_count: %{total_ebs}
    expected_events_per_bunch: %{bunch_size}
    print_event_store_stats: true
    event_queue_depth: %{queue_depth}
    event_queue_wait_time: %{queue_timeout}
    onmon_event_prescale: %{onmon_event_prescale}
    xmlrpc_client_list: \"%{xmlrpc_client_list}\"
    file_size_MB: %{file_size}
    file_duration: %{file_duration}
    file_event_count: %{file_event_count}
    %{ag_type_param_name}: true
  }

  metrics: {
    aggFile: {
      metricPluginType: \"file\"
      level: 3
      fileName: \"/tmp/aggregator/agg_%UID%_metrics.log\"
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

  monitoring_transfer: {

    transferPluginType: shmem

    max_fragment_size_words: %{size_words}
    first_event_builder_rank: %{total_frs}
    
    \# Variables below this in the monitoring_transfer table are only relevant
    \# if transferPluginType, above, is set to multicast. You'll need to 
    \# figure out what the local_address to use for your system is

    multicast_address: \"224.0.0.1\"
    multicast_port: 30001   

    local_address: \"10.226.9.16\"  \# mu2edaq01
    \#  local_address: \"10.226.9.19\"  \# mu2edaq05

    receive_buffer_size: 100000000

    subfragment_size: 6000
    subfragments_per_send: 10

  }
}" )

  agConfig.gsub!(/\%\{size_words\}/, String(fragSizeWords))
  agConfig.gsub!(/\%\{buffer_count\}/, String(totalEBs*4))
  agConfig.gsub!(/\%\{total_ebs\}/, String(totalEBs))
  agConfig.gsub!(/\%\{bunch_size\}/, String(bunchSize))  
  agConfig.gsub!(/\%\{queue_depth\}/, String(queueDepth))  
  agConfig.gsub!(/\%\{queue_timeout\}/, String(queueTimeout))  
  agConfig.gsub!(/\%\{onmon_event_prescale\}/, String(onmonEventPrescale))
  agConfig.gsub!(/\%\{xmlrpc_client_list\}/, String(xmlrpcClientList))
  agConfig.gsub!(/\%\{file_size\}/, String(fileSizeThreshold))
  agConfig.gsub!(/\%\{file_duration\}/, String(fileDuration))
  agConfig.gsub!(/\%\{file_event_count\}/, String(fileEventCount))
  if agType == "online_monitor"
    agConfig.gsub!(/\%\{ag_type_param_name\}/, "is_online_monitor")
  else
    agConfig.gsub!(/\%\{ag_type_param_name\}/, "is_data_logger")
  end

  if Integer(withGanglia) > 0
    brConfig.gsub!(/\%\{ganglia_metric\}/, "")
    agConfig.gsub!(/\%\{ganglia_level\}/, Integer(withGanglia))
  else
    agConfig.gsub!(/\%\{ganglia_metric\}/, "#")
  end
  if Integer(withMsgFacility) > 0
    agConfig.gsub!(/\%\{mf_metric\}/, "")
    agConfig.gsub!(/\%\{mf_level\}/, Integer(withMsgFacility))
  else
    agConfig.gsub!(/\%\{mf_metric\}/, "#")
  end
  if Integer(withGraphite) > 0
    agConfig.gsub!(/\%\{graphite_metric\}/, "")
    agConfig.gsub!(/\%\{graphite_level\}/, Integer(withGraphite))
  else
    agConfig.gsub!(/\%\{graphite_metric\}/, "#")
  end

  return agConfig
end
