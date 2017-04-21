# This function will generate the FHiCL code used to control the
# RoutingMasterMain application by configuring its
# artdaq::RoutingMasterCore object
  
def generateRoutingMasterMain( routingCode, evb_ranks, br_ranks, buffer_count = 10, withGanglia = 0, withMsgFacility = 0, withGraphite = 0)

  rmConfig = String.new( "\
  daq: {
  policy: {
  	  policy: \"NoOp\"
	  event_builder_ranks: %{evb_ranks_list}
	  event_builder_buffer_count: %{buffer_count}
  }

  boardreader_ranks: %{br_ranks_list}

  %{routing_code}

  metrics: {
	rmFile: {
	  metricPluginType: \"file\"
	  level: 3
	  fileName: \"%{datadir}/RoutingMaster/rm_%UID%_metrics.log\"
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
  
  rmConfig.gsub!(/\%\{routing_code\}/, routingCode )
  rmConfig.gsub!(/\%\{buffer_count\}/, String(buffer_count))

  evb_ranks_list = String.new("[")
  evb_ranks.each do |rank|
	evb_ranks_list << String(rank) << ","
  end
  evb_ranks_list.gsub!(/,$/,"]")
  rmConfig.gsub!(/\%\{evb_ranks_list\}/, evb_ranks_list)	

  br_ranks_list = String.new("[")
  br_ranks.each do |rank|
	br_ranks_list << String(rank) << ","
  end
  br_ranks_list.gsub!(/,$/,"]")
  rmConfig.gsub!(/\%\{br_ranks_list\}/, br_ranks_list)

  if Integer(withGanglia) > 0
	rmConfig.gsub!(/\%\{ganglia_metric\}/, "")
	rmConfig.gsub!(/\%\{ganglia_level\}/, String(withGanglia))
  else
	rmConfig.gsub!(/\%\{ganglia_metric\}/, "#")
  end
  if Integer(withMsgFacility) > 0
	rmConfig.gsub!(/\%\{mf_metric\}/, "")
	rmConfig.gsub!(/\%\{mf_level\}/, String(withMsgFacility))
  else
	rmConfig.gsub!(/\%\{mf_metric\}/, "#")
  end
  if Integer(withGraphite) > 0
	rmConfig.gsub!(/\%\{graphite_metric\}/, "")
	rmConfig.gsub!(/\%\{graphite_level\}/, String(withGraphite))
  else
	rmConfig.gsub!(/\%\{graphite_metric\}/, "#")
  end
 
  return rmConfig
end
