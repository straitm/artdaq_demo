
def generateAggregator(bunchSize, fragSizeWords, sources_fhicl,
                       xmlrpcClientList,
					   subrunSizeThreshold, subrunDuration, subrunEventCount, 
					   queueDepth, queueTimeout, onmonEventPrescale,
                       agType, logger_rank, dispatcher_rank, dataDir,
					   withGanglia = 0, withMsgFacility = 0, withGraphite = 0)

agConfig = String.new( "\
daq: {
  aggregator: {
    expected_events_per_bunch: %{bunch_size}
    print_event_store_stats: true
    event_queue_depth: %{queue_depth}
    event_queue_wait_time: %{queue_timeout}
    onmon_event_prescale: %{onmon_event_prescale}
    xmlrpc_client_list: \"%{xmlrpc_client_list}\"
    subrun_size_MB: %{subrun_size}
    subrun_duration: %{subrun_duration}
    subrun_event_count: %{subrun_event_count}
    %{ag_type_param_name}: true

	sources: {
		%{sources_fhicl}
	}
  }

  metrics: {
    aggFile: {
      metricPluginType: \"file\"
      level: 3
      fileName: \"%{datadir}/aggregator/agg_%UID%_metrics.log\"
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

  transfer_to_dispatcher: {
    transferPluginType: Shmem
	source_rank: %{logger_rank}
	destination_rank: %{dispatcher_rank}
    max_fragment_size_words: %{size_words}
  }

}" )

agConfig.gsub!(/\%\{sources_fhicl\}/, sources_fhicl)
  agConfig.gsub!(/\%\{size_words\}/, String(fragSizeWords))
  agConfig.gsub!(/\%\{bunch_size\}/, String(bunchSize))  
  agConfig.gsub!(/\%\{queue_depth\}/, String(queueDepth))  
  agConfig.gsub!(/\%\{queue_timeout\}/, String(queueTimeout))  
  agConfig.gsub!(/\%\{onmon_event_prescale\}/, String(onmonEventPrescale))
  agConfig.gsub!(/\%\{xmlrpc_client_list\}/, String(xmlrpcClientList))
  agConfig.gsub!(/\%\{subrun_size\}/, String(subrunSizeThreshold))
  agConfig.gsub!(/\%\{subrun_duration\}/, String(subrunDuration))
  agConfig.gsub!(/\%\{subrun_event_count\}/, String(subrunEventCount))
  if agType == "online_monitor"
    #agConfig.gsub!(/\%\{ag_type_param_name\}/, "is_online_monitor")
    agConfig.gsub!(/\%\{ag_type_param_name\}/, "is_dispatcher")
  else
    agConfig.gsub!(/\%\{ag_type_param_name\}/, "is_data_logger")
  end
  agConfig.gsub!(/\%\{logger_rank\}/, String(logger_rank))
  agConfig.gsub!(/\%\{dispatcher_rank\}/, String(dispatcher_rank))

  
  agConfig.gsub!(/\%\{datadir\}/, dataDir)
  if Integer(withGanglia) > 0
    agConfig.gsub!(/\%\{ganglia_metric\}/, "")
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
